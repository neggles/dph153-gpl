/* Local static variable elimination pass.
   Copyright (C) 2007 Free Software Foundation, Inc.
   Contributed by Nathan Froyd <froydnj@codesourcery.com>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

GCC is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/* Converting static function-local variables to automatic variables.

   The motivating example is a function like:

   void
   foo (unsigned n)
   {
     static int var;
     unsigned i;

     for (i = 0; i != n; i++)
       {
         var = ...

         do other things with var...
       }
   }

   Because VAR is static, doing things like code motion to loads and
   stores of VAR is difficult.  Furthermore, accesses to VAR are
   inefficient.  This pass aims to recognize the cases where it is not
   necessary for VAR to be static and modify the code so that later
   passes will do the appropriate optimizations.

   The criteria for a static function-local variable V in a function F
   being converted to an automatic variable are:

   1. F does not call setjmp; and
   2. V's address is never taken; and
   3. V is not declared volatile; and
   4. V is not used in any nested function; and
   5. Every use of V is defined along all paths leading to the use.

   NOTE: For ease of implementation, we currently treat a function call
   as killing all previous definitions of static variables, since we
   could have:

   static void
   foo (...)
   {
     static int x;

     x = ...;       (1)

    f (...);        (2)

     ... = x;       (3)
   }

   The use at (3) needs to pick up a possible definition made by the
   call at (2).  If the call at (2) does not call back into 'foo',
   then the call is not a killing call.  We currently treat it as
   though it is.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"

#include "rtl.h"
#include "tm_p.h"
#include "hard-reg-set.h"
#include "obstack.h"
#include "basic-block.h"

#include "tree.h"
#include "tree-gimple.h"
#include "hashtab.h"
#include "diagnostic.h"
#include "tree-flow.h"
#include "tree-dump.h"
#include "flags.h"
#include "timevar.h"
#include "tree-pass.h"

struct rls_decl_info
{
  /* The variable declaration.  */
  tree orig_var;

  /* Its index in rls_block_local_data.  */
  int index;

  /* Whether we can optimize this variable.  */
  bool optimizable_p;

  /* The new variable declaration, if we can optimize away the staticness
     of 'orig_var'.  */
  tree new_var;
};

/* Filled with 'struct rls_decl_info'; keyed off ORIG_VAR.  */
static htab_t static_variables;

struct rls_stmt_info
{
  /* The variable declaration.  */
  tree var;

  /* The statement in which we found a def or a use of the variable.  */
  tree stmt;

  /* Whether STMT represents a use of VAR.  */
  bool use_p;

  /* A bitmap whose entries denote what variables have been defined
     when execution arrives at STMT.  This field is only used when
     USE_P is true.  */
  sbitmap defined;
};

/* Filled with 'struct rls_stmt_info'; keyed off STMT.  */
static htab_t defuse_statements;

static struct
{
  /* The number of static variables we found.  */
  size_t n_statics;

  /* The number of optimizable variables we found.  */
  size_t n_optimizable;
} stats;

struct rls_block_dataflow_data {
  /* A bitmap whose entries denote what variables have been defined on
     entry to this block.  */
  sbitmap defined_in;

  /* A bitmap whose entries denote what variables have been defined on
     exit from this block.  */
  sbitmap defined_out;
};

/* Parameters for the 'static_variables' hash table.  */

static hashval_t
rls_hash_decl_info (const void *x)
{
  return htab_hash_pointer
    ((const void *) ((const struct rls_decl_info *) x)->orig_var);
}

static int
rls_eq_decl_info (const void *x, const void *y)
{
  const struct rls_decl_info *a = x;
  const struct rls_decl_info *b = y;

  return a->orig_var == b->orig_var;
}

static void
rls_free_decl_info (void *info)
{
  free (info);
}

/* Parameters for the 'defuse_statements' hash table.  */

static hashval_t
rls_hash_use_info (const void *x)
{
  return htab_hash_pointer
    ((const void *) ((const struct rls_stmt_info *) x)->stmt);
}

static int
rls_eq_use_info (const void *x, const void *y)
{
  const struct rls_stmt_info *a = x;
  const struct rls_stmt_info *b = y;

  return a->stmt == b->stmt;
}

static void
rls_free_use_info (void *info)
{
  struct rls_stmt_info *stmt_info = info;

  if (stmt_info->defined)
    sbitmap_free (stmt_info->defined);

  free (stmt_info);
}

/* Initialize data structures and statistics.  */

static void
rls_init (void)
{
  basic_block bb;

  /* We expect relatively few static variables, hence the small
     initial size for the hash table.  */
  static_variables = htab_create (8, rls_hash_decl_info,
                                  rls_eq_decl_info, rls_free_decl_info);

  /* We expect quite a few statements.  */
  defuse_statements = htab_create (128, rls_hash_use_info,
                                   rls_eq_use_info, rls_free_use_info);

  FOR_ALL_BB (bb)
    {
      struct rls_block_dataflow_data *data;

      data = XNEW (struct rls_block_dataflow_data);
      memset (data, 0, sizeof (*data));
      bb->aux = data;
    }

  stats.n_statics = 0;
  stats.n_optimizable = 0;
}

/* Free data structures.  */

static void
rls_done (void)
{
  basic_block bb;

  htab_delete (static_variables);
  htab_delete (defuse_statements);

  FOR_ALL_BB (bb)
    {
      struct rls_block_dataflow_data *data = bb->aux;

      gcc_assert (data);

      if (data->defined_in)
	sbitmap_free (data->defined_in);
      if (data->defined_out)
	sbitmap_free (data->defined_out);
      free (data);
      bb->aux = NULL;
    }
}


/* Doing the initial work to find static variables.  */

/* Examine the defining statement for VAR and determine whether it is a
   static variable we could potentially optimize.  If so, stick in it
   in the 'static_variables' hashtable.

   STMT is the statement in which a definition or use of VAR occurs.
   USE_P indicates whether VAR is used or defined in STMT.  Enter STMT
   into 'defuse_statements' as well for use during dataflow
   analysis.  */

static void
maybe_discover_new_declaration (tree var, tree stmt, bool use_p)
{
  tree def_stmt = SSA_NAME_VAR (var);

  if (TREE_CODE (def_stmt) == VAR_DECL
      && DECL_CONTEXT (def_stmt) != NULL_TREE
      && TREE_CODE (DECL_CONTEXT (def_stmt)) == FUNCTION_DECL
      && TREE_STATIC (def_stmt)
      && !TREE_ADDRESSABLE (def_stmt)
      && !TREE_THIS_VOLATILE (def_stmt))
    {
      struct rls_decl_info dummy;
      void **slot;

      dummy.orig_var = def_stmt;
      slot = htab_find_slot (static_variables, &dummy, INSERT);

      if (*slot == NULL)
        {
          /* Found a use or a def of a new declaration.  */
          struct rls_decl_info *info = XNEW (struct rls_decl_info);

          info->orig_var = def_stmt;
          info->index = stats.n_statics++;
          /* Optimistically assume that we can optimize.  */
          info->optimizable_p = true;
          info->new_var = NULL_TREE;
          *slot = (void *) info;
        }

      /* Enter the statement into DEFUSE_STATEMENTS.  */
      {
        struct rls_stmt_info dummy;
        struct rls_stmt_info *info;

        dummy.stmt = stmt;
        slot = htab_find_slot (defuse_statements, &dummy, INSERT);

        /* We should never insert the same statement into the
           hashtable twice.  */
        gcc_assert (*slot == NULL);

        info = XNEW (struct rls_stmt_info);
        info->var = def_stmt;
        info->stmt = stmt;
        if (dump_file)
          {
            fprintf (dump_file, "entering as %s ", use_p ? "use" : "def");
            print_generic_stmt (dump_file, stmt, TDF_DETAILS);
          }
        info->use_p = use_p;
        /* We don't know how big to make the bitmap yet.  */
        info->defined = NULL;
        *slot = (void *) info;
      }
    }
}

/* Grovel through all the statements in the program, looking for
   SSA_NAMEs whose SSA_NAME_VAR is a VAR_DECL.  We look at both use and
   def SSA_NAMEs.  */

static void
find_static_nonvolatile_declarations (void)
{
  basic_block bb;

  FOR_EACH_BB (bb)
    {
      block_stmt_iterator i;

      for (i = bsi_start (bb); !bsi_end_p (i); bsi_next (&i))
        {
          tree var;
          ssa_op_iter iter;
          tree stmt = bsi_stmt (i);

	  /* If there's a call expression in STMT, then previous passes
	     will have determined if the call transitively defines some
	     static variable.  However, we need more precise
	     information--we need to know whether static variables are
	     live out after the call.

	     Since we'll never see something like:

	       staticvar = foo (bar, baz);

	     in GIMPLE (the result of the call will be assigned to a
	     normal, non-static local variable which is then assigned to
	     STATICVAR in a subsequent statement), don't bother finding
	     new declarations if we see a CALL_EXPR.  */
	  if (get_call_expr_in (stmt) == NULL_TREE)
	    FOR_EACH_SSA_TREE_OPERAND (var, stmt, iter, SSA_OP_VDEF)
	      {
		maybe_discover_new_declaration (var, stmt, false);
	      }

          FOR_EACH_SSA_TREE_OPERAND (var, stmt, iter, SSA_OP_VUSE)
            {
              maybe_discover_new_declaration (var, stmt, true);
            }
        }
    }
}


/* Determining if we have anything to optimize.  */

/* Examine *SLOT (which is a 'struct rls_decl_info *') to see whether
   the associated variable is optimizable.  If it is, create a new,
   non-static declaration for the variable; this new variable will be
   used during a subsequent rewrite of the function.  */

#define NEW_VAR_PREFIX ".unstatic"

static int
maybe_create_new_variable (void **slot, void *data ATTRIBUTE_UNUSED)
{
  struct rls_decl_info *info = *slot;
  tree id_node = DECL_NAME (info->orig_var);
  size_t id_len = IDENTIFIER_LENGTH (id_node);
  size_t name_len = id_len + strlen (NEW_VAR_PREFIX) + 1;
  char *name;

  /* Don't create a new variable multiple times.  */
  gcc_assert (!info->new_var);

  /* Tie the new name to the old one to aid debugging dumps.  */
  name = alloca (name_len);
  strcpy (name, IDENTIFIER_POINTER (id_node));
  strcpy (name + id_len, NEW_VAR_PREFIX);
  info->new_var = create_tmp_var (TREE_TYPE (info->orig_var), name);

  if (dump_file)
    {
      fprintf (dump_file, "new variable ");
      print_generic_stmt (dump_file, info->new_var, TDF_DETAILS);
    }

  /* Inform SSA about this new variable.  */
  mark_sym_for_renaming (info->new_var);
  create_var_ann (info->new_var);
  add_referenced_var (info->new_var);

  /* Always continue scanning.  */
  return 1;
}

#undef NEW_VAR_PREFIX

/* Traverse the 'defuse_statements' hash table.  For every use,
   determine if the associated variable is defined along all paths
   leading to said use.  Remove the associated variable from
   'static_variables' if it is not.  */

static int
check_definedness (void **slot, void *data ATTRIBUTE_UNUSED)
{
  struct rls_stmt_info *info = *slot;
  struct rls_decl_info dummy;

  /* We don't need to look at definitions.  Continue scanning.  */
  if (!info->use_p)
    return 1;

  dummy.orig_var = info->var;
  slot = htab_find_slot (static_variables, &dummy, INSERT);

  /* Might not be there because we deleted it already.  */
  if (*slot)
    {
      struct rls_decl_info *decl = *slot;

      if (!TEST_BIT (info->defined, decl->index))
        {
          if (dump_file)
            {
              fprintf (dump_file, "not optimizing ");
              print_generic_stmt (dump_file, decl->orig_var, TDF_DETAILS);
              fprintf (dump_file, "due to uncovered use in ");
              print_generic_stmt (dump_file, info->stmt, TDF_DETAILS);
              fprintf (dump_file, "\n");
            }

          htab_clear_slot (static_variables, slot);
          stats.n_optimizable--;
        }
    }

  /* Continue scan.  */
  return 1;
}

/* Check all statements in 'defuse_statements' to see if all the
   statements that use a static variable have that variable defined
   along all paths leading to the statement.  Once that's done, go
   through and create new, non-static variables for any static variables
   that can be optimized.  */

static size_t
determine_optimizable_statics (void)
{
  htab_traverse (defuse_statements, check_definedness, NULL);

  htab_traverse (static_variables, maybe_create_new_variable, NULL);

  return stats.n_optimizable;
}

/* Look at STMT to see if we have uses or defs of a static variable.
   STMT is passed in DATA.  Definitions of a static variable are found
   by the presence of a V_MUST_DEF, while uses are found by the presence
   of a VUSE.  */

static int
unstaticize_variable (void **slot, void *data)
{
  struct rls_decl_info *info = *slot;
  tree stmt = (tree) data;
  tree vdef;
  tree vuse;

  /* We should have removed unoptimizable variables during an earlier
     traversal.  */
  gcc_assert (info->optimizable_p);

  /* Check for virtual definitions first.  */
  vdef = SINGLE_SSA_TREE_OPERAND (stmt, SSA_OP_VDEF);

  if (vdef != NULL
      && ZERO_SSA_OPERANDS (stmt, SSA_OP_DEF)
      && TREE_CODE (stmt) == GIMPLE_MODIFY_STMT
      && TREE_CODE (GIMPLE_STMT_OPERAND (stmt, 0)) == VAR_DECL
      && GIMPLE_STMT_OPERAND (stmt, 0) == info->orig_var)
    {
      /* Make the statement define the new name.  The new name has
         already been marked for renaming, so no need to do that
         here.  */
      GIMPLE_STMT_OPERAND (stmt, 0) = info->new_var;

      update_stmt (stmt);

      /* None of the other optimizable static variables can occur
         in this statement.  Stop the scan.  */
      return 0;
    }

  /* Check for virtual uses.  */
  vuse = SINGLE_SSA_TREE_OPERAND (stmt, SSA_OP_VUSE);

  if (vuse != NULL
      && TREE_CODE (stmt) == GIMPLE_MODIFY_STMT
      && TREE_CODE (GIMPLE_STMT_OPERAND (stmt, 1)) == VAR_DECL
      && GIMPLE_STMT_OPERAND (stmt, 1) == info->orig_var)
    {
      /* Make the statement use the new name.  */
      GIMPLE_STMT_OPERAND (stmt, 1) = info->new_var;

      update_stmt (stmt);

      /* None of the other optimizable static variables can occur
         in this statement.  Stop the scan.  */
      return 0;
    }

  /* Continue scanning.  */
  return 1;
}

/* Determine if we have any static variables we can optimize.  If so,
   replace any defs or uses of those variables in their defining/using
   statements.  */

static void
maybe_remove_static_from_declarations (void)
{
  size_t n_optimizable = determine_optimizable_statics ();
  basic_block bb;

  if (n_optimizable)
    /* Replace any optimizable variables with new, non-static variables.  */
    FOR_EACH_BB (bb)
      {
        block_stmt_iterator bsi;

        for (bsi = bsi_start (bb); !bsi_end_p (bsi); bsi_next (&bsi))
          {
            tree stmt = bsi_stmt (bsi);

            htab_traverse (static_variables, unstaticize_variable, stmt);
          }
      }
}

/* Callback for htab_traverse to initialize the bitmap for *SLOT, which
   is a 'struct rls_stmt_info'.  */

static int
initialize_statement_dataflow (void **slot, void *data ATTRIBUTE_UNUSED)
{
  struct rls_stmt_info *info = *slot;

  gcc_assert (!info->defined);

  if (info->use_p)
    {
      info->defined = sbitmap_alloc (stats.n_statics);
      /* Assume defined along all paths until otherwise informed.  */
      sbitmap_ones (info->defined);
    }

  /* Continue traversal.  */
  return 1;
}

/* We have N_STATICS static variables to consider.  Go through all the
   blocks and all the use statements to initialize their bitmaps.  */

static void
initialize_block_and_statement_dataflow (size_t n_statics)
{
  basic_block bb;

  FOR_ALL_BB (bb)
    {
      struct rls_block_dataflow_data *data = bb->aux;

      gcc_assert (data);

      data->defined_in = sbitmap_alloc (n_statics);
      sbitmap_zero (data->defined_in);
      data->defined_out = sbitmap_alloc (n_statics);
      sbitmap_zero (data->defined_out);
    }

  htab_traverse (defuse_statements, initialize_statement_dataflow, NULL);
}

/* Apply the individual effects of the stmts in BB to update the
   dataflow analysis information for BB.  */

static void
compute_definedness_for_block (basic_block bb)
{
  bool changed_p = false;
  struct rls_block_dataflow_data *data = bb->aux;
  block_stmt_iterator bsi;

  sbitmap_copy (data->defined_out, data->defined_in);

  for (bsi = bsi_start (bb); !bsi_end_p (bsi); bsi_next (&bsi))
    {
      tree stmt = bsi_stmt (bsi);
      struct rls_stmt_info dummy;
      void **slot;

      /* First see if this statement uses or defines a static variable.  */
      dummy.stmt = stmt;
      slot = htab_find_slot (defuse_statements, &dummy, INSERT);

      /* Check for uses.  */
      if (*slot != NULL)
	{
	  struct rls_stmt_info *info = *slot;

	  gcc_assert (get_call_expr_in (stmt) == NULL_TREE);

	  if (info->use_p)
	    {
	      gcc_assert (info->defined);

	      /* Found a statement that uses a function-local static
		 variable.  Copy the current state of definedness.  */
	      sbitmap_copy (info->defined, data->defined_out);
	    }
	  else
	    {
	      struct rls_decl_info dummy;
	      struct rls_decl_info *decl;

	      gcc_assert (!info->defined);

	      /* Found a statement that defines a function-local static
		 variable.  Look up the associated variable's information
		 and mark it as defined in the block.  */
	      dummy.orig_var = info->var;
	      slot = htab_find_slot (static_variables, &dummy, INSERT);

	      gcc_assert (*slot);

	      decl = (struct rls_decl_info *) *slot;

	      SET_BIT (data->defined_out, decl->index);
	      changed_p |= true;
	    }
	}
      else if (get_call_expr_in (stmt) != NULL_TREE)
	/* If there's a call expression in STMT, then previous passes
	   will have determined if the call transitively defines some
	   static variable.  However, we need more precise
	   information--we need to know whether static variables are
	   live out after the call.  In the absence of such information,
	   simply declare that all static variables are clobbered by the
	   call.  A better analysis would be interprocedural and compute
	   the liveness information we require, but for now, we're being
	   pessimistic.  */
	sbitmap_zero (data->defined_out);
    }
}

/* Solve the dataflow equations:

   DEFINED_IN(b) = intersect DEFINED_OUT(p) for p in preds(b)
   DEFINED_OUT(b) = VARIABLES_DEFINED (b, DEFINED_IN (b))

   via a simple iterative solver.  VARIABLES_DEFINED is computed by
   'compute_definedness_for_block'.  */

static void
compute_definedness (void)
{
  basic_block bb;
  bool changed_p;
  sbitmap tmp_bitmap = sbitmap_alloc (stats.n_statics);

  /* Compute initial sets.  */
  FOR_EACH_BB (bb)
    {
      compute_definedness_for_block (bb);
    }

  /* Iterate.  */
  do {
    changed_p = false;

    FOR_EACH_BB (bb)
      {
        edge e;
        edge_iterator ei;
        struct rls_block_dataflow_data *data = bb->aux;
        bool bitmap_changed_p = false;

        sbitmap_ones (tmp_bitmap);

        gcc_assert (data);

        /* We require information about whether a variable was defined
           over all paths leading to a particular use.  Therefore, we
           intersect the DEFINED sets of all predecessors.  */
        FOR_EACH_EDGE (e, ei, bb->preds)
          {
            struct rls_block_dataflow_data *pred_data = e->src->aux;

            gcc_assert (pred_data);

            sbitmap_a_and_b (tmp_bitmap, tmp_bitmap, pred_data->defined_out);
          }

        bitmap_changed_p = !sbitmap_equal (tmp_bitmap, data->defined_in);

        if (bitmap_changed_p)
          {
            sbitmap_copy (data->defined_in, tmp_bitmap);
            compute_definedness_for_block (bb);
          }

        changed_p |= bitmap_changed_p;
      }
  } while (changed_p);

  sbitmap_free (tmp_bitmap);
}

static unsigned int
execute_rls (void)
{
  rls_init ();

  find_static_nonvolatile_declarations ();

  /* Can we optimize anything?  */
  if (stats.n_statics != 0)
    {
      stats.n_optimizable = stats.n_statics;

      if (dump_file)
        fprintf (dump_file, "found %d static variables to consider\n",
                 stats.n_statics);

      initialize_block_and_statement_dataflow (stats.n_statics);

      compute_definedness ();

      maybe_remove_static_from_declarations ();

      if (dump_file)
        fprintf (dump_file, "removed %d static variables\n",
                 stats.n_optimizable);
    }

  rls_done ();

  return 0;
}

static bool
gate_rls (void)
{
  return (flag_remove_local_statics != 0
          && !current_function_calls_setjmp
          && !cgraph_node (current_function_decl)->ever_was_nested);
}

struct tree_opt_pass pass_remove_local_statics =
{
  "remlocstatic",               /* name */
  gate_rls,                     /* gate */
  execute_rls,                  /* execute */
  NULL,                         /* sub */
  NULL,                         /* next */
  0,                            /* static_pass_number */
  TV_TREE_RLS,                  /* tv_id */
  PROP_cfg | PROP_ssa,          /* properties_required */
  0,                            /* properties_provided */
  0,                            /* properties_destroyed */
  0,                            /* todo_flags_start */
  TODO_dump_func | TODO_verify_ssa | TODO_verify_stmts
  | TODO_update_ssa,                /* todo_flags_finish */
  0                                 /* letter */
};
