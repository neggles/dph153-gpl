;; Marvell 2850 pipeline description
;; Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
;; Written by Marvell and CodeSourcery, Inc.

;; This file is part of GCC.

;; GCC is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published
;; by the Free Software Foundation; either version 3, or (at your
;; option) any later version.

;; GCC is distributed in the hope that it will be useful, but WITHOUT
;; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
;; or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
;; License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING3.  If not see
;; <http://www.gnu.org/licenses/>.

;; This automaton provides a pipeline description for the Marvell
;; 2850 core.
;;
;; The model given here assumes that the condition for all conditional
;; instructions is "true", i.e., that all of the instructions are
;; actually executed.

(define_automaton "marvell_f")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Pipelines
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; This is a dual-issue processor with three pipelines:
;;
;; 1. Arithmetic and load/store pipeline A1.
;;    Issue | E1 | E2 | OF | WR | WB  for load-store instructions
;;    Issue | E1 | E2 | WB  for arithmetic instructions
;;
;; 2. Arithmetic pipeline A2.
;;    Issue | E1 | E2 | WB
;;
;; 3. Multiply and multiply-accumulate pipeline.
;;    Issue | MAC1 | MAC2 | MAC3 | WB
;;
;; There are various bypasses modelled to a greater or lesser extent.
;;
;; Latencies in this file correspond to the number of cycles after
;; the issue stage that it takes for the result of the instruction to
;; be computed, or for its side-effects to occur.

(define_cpu_unit "a1_e1,a1_e2,a1_of,a1_wr,a1_wb" "marvell_f") ; ALU 1
(define_cpu_unit "a2_e1,a2_e2,a2_wb" "marvell_f")             ; ALU 2
(define_cpu_unit "m_1,m_2,m_3,m_wb" "marvell_f")              ; MAC

;; We define an SRAM cpu unit to enable us to describe conflicts
;; between loads at the E2 stage and stores at the WR stage.

(define_cpu_unit "sram" "marvell_f")

;; Handling of dual-issue constraints.
;;
;; Certain pairs of instructions can be issued in parallel, and certain
;; pairs cannot.  We divide a subset of the instructions into groups as
;; follows.
;;
;; - data processing 1 (mov, mvn);
;; - data processing 2 (adc, add, and, bic, cmn, cmp, eor, orr, rsb,
;;                      rsc, sbc, sub, teq, tst);
;; - load single (ldr, ldrb, ldrbt, ldrt, ldrh, ldrsb, ldrsh);
;; - store single (str, strb, strbt, strt, strh);
;; - swap (swp, swpb);
;; - pld;
;; - count leading zeros and DSP add/sub (clz, qadd, qdadd, qsub, qdsub);
;; - multiply 2 (mul, muls, smull, umull, smulxy, smulls, umulls);
;; - multiply 3 (mla, mlas, smlal, umlal, smlaxy, smlalxy, smlawx,
;;               smlawy, smlals, umlals);
;; - branches (b, bl, blx, bx).
;;
;; Ignoring conditional execution, it is a good approximation to the core
;; to model that two instructions may only be issued in parallel if the
;; following conditions are met.
;; I.   The instructions both fall into one of the above groups and their
;;      corresponding groups have a entry in the matrix below that is not X.
;; II.  The second instruction does not read any register updated by the
;;      first instruction (already enforced by the GCC scheduler).
;; III. The second instruction does not need the carry flag updated by the
;;      first instruction.  Currently we do not model this.
;;
;; First	Second instruction group
;; insn
;;		DP1  DP2  L    S    SWP  PLD  CLZ  M2   M3   B
;;
;;	DP1	ok   ok   ok   ok   ok   ok   ok   ok   ok   ok
;;	DP2(1)  ok   ok   ok   ok   ok   ok   ok   ok   ok   ok
;;	DP2(2)  ok   (2)  ok   (4)  ok   ok   ok   ok   X    ok
;;	L   }
;;	SWP }   ok   ok   X    X    X    X    ok   ok   ok   ok
;;	PLD }
;;      S(3)	ok   ok   X    X    X    X    ok   ok   ok   ok
;;      S(4)	ok   (2)  X    X    X    X    ok   ok   X    ok
;;	CLZ     ok   ok   ok   ok   ok   ok   ok   ok   ok   ok
;;	M2	ok   ok   ok   ok   ok   ok   ok   X    X    ok
;;	M3	ok   (2)  ok   (4)  ok   ok   ok   X    X    ok
;;	B	ok   ok   ok   ok   ok   ok   ok   ok   ok   ok
;;
;; (1) without register shift
;; (2) with register shift
;; (3) with immediate offset
;; (4) with register offset
;;
;; We define a fake cpu unit "reg_shift_lock" to enforce constraints
;; between instructions in groups DP2(2) and M3.  All other
;; constraints are enforced automatically by virtue of the limited
;; number of pipelines available for the various operations, with
;; the exception of constraints involving S(4) that we do not model.

(define_cpu_unit "reg_shift_lock" "marvell_f")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ALU instructions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; 1. Certain logic operations can be retired after the E1 stage if
;; the pipeline is not already retiring another instruction.  In this
;; model we assume this behaviour always holds for mov, mvn, and, orr, eor
;; instructions.  If a register shift is involved and the instruction is
;; not mov or mvn, then a dual-issue constraint must be enforced.

;; The first two cases are separate so they can be identified for
;; bypasses below.

(define_insn_reservation "marvell_f_alu_early_retire" 1
  (and (eq_attr "tune" "marvell_f")
       (and (eq_attr "type" "alu")
            (eq_attr "insn" "mov,mvn,and,orr,eor")))
  "(a1_e1,a1_wb)|(a2_e1,a2_wb)")

(define_insn_reservation "marvell_f_alu_early_retire_shift" 1
  (and (eq_attr "tune" "marvell_f")
       (and (eq_attr "type" "alu_shift_reg")
            (eq_attr "insn" "mov,mvn,and,orr,eor")))
  "(a1_e1,a1_wb)|(a2_e1,a2_wb)")

(define_insn_reservation "marvell_f_alu_early_retire_reg_shift1" 1
  (and (eq_attr "tune" "marvell_f")
       (and (eq_attr "type" "alu_shift_reg")
            (eq_attr "insn" "mov,mvn")))
  "(a1_e1,a1_wb)|(a2_e1,a2_wb)")

(define_insn_reservation "marvell_f_alu_early_retire_reg_shift2" 1
  (and (eq_attr "tune" "marvell_f")
       (and (eq_attr "type" "alu_shift_reg")
            (eq_attr "insn" "and,orr,eor")))
  "(reg_shift_lock+a1_e1,a1_wb)|(reg_shift_lock+a2_e1,a2_wb)")

;; 2. ALU operations with no shifted operand.  These bypass the E1 stage if
;; the E2 stage of the corresponding pipeline is clear; here, we always
;; model this scenario [*].  We give the operation a latency of 1 yet reserve
;; both E1 and E2 for it (thus preventing the GCC scheduler, in the case
;; where both E1 and E2 of one pipeline are clear, from issuing one
;; instruction to each).
;;
;; [*] The non-bypass case is a latency of two, reserving E1 on the first
;;     cycle and E2 on the next.  Due to the way the scheduler works we
;;     have to choose between taking this as the default and taking the
;;     above case (with latency one) as the default; we choose the latter.

(define_insn_reservation "marvell_f_alu_op_bypass_e1" 1
  (and (eq_attr "tune" "marvell_f")
       (and (eq_attr "type" "alu")
            (not (eq_attr "insn" "mov,mvn,and,orr,eor"))))
  "(a1_e1+a1_e2,a1_wb)|(a2_e1+a2_e2,a2_wb)")

;; 3. ALU operations with a shift-by-constant operand.

(define_insn_reservation "marvell_f_alu_shift_op" 2
  (and (eq_attr "tune" "marvell_f")
       (and (eq_attr "type" "alu_shift")
            (not (eq_attr "insn" "mov,mvn,and,orr,eor"))))
  "(a1_e1,a1_e2,a1_wb)|(a2_e1,a2_e2,a2_wb)")

;; 4. ALU operations with a shift-by-register operand.  Since the
;; instruction is never mov or mvn, a dual-issue constraint must
;; be enforced.

(define_insn_reservation "marvell_f_alu_shift_reg_op" 2
  (and (eq_attr "tune" "marvell_f")
       (and (eq_attr "type" "alu_shift_reg")
            (not (eq_attr "insn" "mov,mvn,and,orr,eor"))))
  "(reg_shift_lock+a1_e1,a1_e2,a1_wb)|(reg_shift_lock+a2_e1,a2_e2,a2_wb)")

;; Given an ALU operation with shift (I1) followed by another ALU
;; operation (I2), with I2 depending on the destination register Rd of I1
;; and with I2 not using that value as the amount or the starting value for
;; a shift, then I1 and I2 may be issued to the same pipeline on
;; consecutive cycles.  In terms of this model that corresponds to I1
;; having a latency of one cycle.  There are three cases for various
;; I1 and I2 as follows.

;; (a) I1 has a constant or register shift and I2 doesn't have a shift at all.
(define_bypass 1 "marvell_f_alu_shift_op,\
	          marvell_f_alu_shift_reg_op"
	       "marvell_f_alu_op_bypass_e1,marvell_f_alu_early_retire")

;; (b) I1 has a constant or register shift and I2 has a constant shift.
;; Rd must not provide the starting value for the shift.
(define_bypass 1 "marvell_f_alu_shift_op,\
	          marvell_f_alu_shift_reg_op"
	       "marvell_f_alu_shift_op,marvell_f_alu_early_retire_shift"
	       "arm_no_early_alu_shift_value_dep")

;; (c) I1 has a constant or register shift and I2 has a register shift.
;; Rd must not provide the amount by which to shift.
(define_bypass 1 "marvell_f_alu_shift_op,\
	          marvell_f_alu_shift_reg_op"
	       "marvell_f_alu_shift_reg_op,\
	        marvell_f_alu_early_retire_reg_shift1,\
	        marvell_f_alu_early_retire_reg_shift2"
	       "arm_no_early_alu_shift_dep")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Multiplication instructions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Multiplication instructions in group "Multiply 2".

(define_insn_reservation "marvell_f_multiply_2" 3
  (and (eq_attr "tune" "marvell_f")
       (eq_attr "insn" "mul,muls,smull,umull,smulxy,smulls,umulls"))
  "m_1,m_2,m_3,m_wb")

;; Multiplication instructions in group "Multiply 3".  There is a
;; dual-issue constraint with non-multiplication ALU instructions
;; to be respected here.

(define_insn_reservation "marvell_f_multiply_3" 3
  (and (eq_attr "tune" "marvell_f")
       (eq_attr "insn" "mla,mlas,smlal,umlal,smlaxy,smlalxy,smlawx,\
                        smlawy,smlals,umlals"))
  "reg_shift_lock+m_1,m_2,m_3,m_wb")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Branch instructions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Conditional backward b instructions can have a zero-cycle penalty, and
;; other conditional b and bl instructions have a one-cycle penalty if
;; predicted correctly.  Currently we model the zero-cycle case for all
;; branches.

(define_insn_reservation "marvell_f_branches" 0
 (and (eq_attr "tune" "marvell_f")
      (eq_attr "type" "branch"))
 "nothing")

;; Call latencies are not predictable; a semi-arbitrary very large
;; number is used as "positive infinity" for such latencies.

(define_insn_reservation "marvell_f_call" 32 
 (and (eq_attr "tune" "marvell_f")
      (eq_attr "type" "call"))
 "nothing")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Load/store instructions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; The models for load/store instructions do not accurately describe
;; the difference between operations with a base register writeback.
;; These models assume that all memory references hit in dcache.

;; 1. Load/store for single registers.

;; The worst case for a load is when the load result is needed in E1
;; (for example for a register shift), giving a latency of four.  Loads
;; skip E1 and access memory at the E2 stage.

(define_insn_reservation "marvell_f_load1" 4
 (and (eq_attr "tune" "marvell_f")
      (eq_attr "type" "load1,load_byte"))
 "a1_e2+sram,a1_of,a1_wr,a1_wb")

;; The result for a load may be bypassed (to be available at the same
;; time as the load arrives in the WR stage, so effectively at the OF
;; stage) to the Rn operand at E2 with a latency of two.  The result may
;; be bypassed to a non-Rn operand at E2 with a latency of three.  For
;; instructions without shifts, detection of an Rn bypass situation is
;; difficult (because some of the instruction patterns switch their
;; operands), and so we do not model that here.  For instructions with
;; shifts, the operand used at E2 will always be Rn, and so we can
;; model the latency-two bypass for these.

(define_bypass 2 "marvell_f_load1"
               "marvell_f_alu_shift_op"
	       "arm_no_early_alu_shift_value_dep")

(define_bypass 2 "marvell_f_load1"
               "marvell_f_alu_shift_reg_op"
	       "arm_no_early_alu_shift_dep")

;; Stores write at the WR stage and loads read at the E2 stage, giving
;; a store latency of three.

(define_insn_reservation "marvell_f_store1" 3
 (and (eq_attr "tune" "marvell_f")
      (eq_attr "type" "store1"))
 "a1_e2,a1_of,a1_wr+sram,a1_wb")

;; 2. Load/store for two consecutive registers.  These may be dealt
;; with in the same number of cycles as single loads and stores.

(define_insn_reservation "marvell_f_load2" 4
 (and (eq_attr "tune" "marvell_f")
      (eq_attr "type" "load2"))
 "a1_e2+sram,a1_of,a1_wr,a1_wb")

(define_insn_reservation "marvell_f_store2" 3
 (and (eq_attr "tune" "marvell_f")
      (eq_attr "type" "store2"))
 "a1_e2,a1_of,a1_wr+sram,a1_wb")

;; The first word of a doubleword load is eligible for the latency-two
;; bypass described above for single loads, but this is not modelled here.
;; We do however assume that either word may also be bypassed with
;; latency three for ALU operations with shifts (where the shift value and
;; amount do not depend on the loaded value) and latency four for ALU
;; operations without shifts.  The latency four case is of course the default.

(define_bypass 3 "marvell_f_load2"
               "marvell_f_alu_shift_op"
	       "arm_no_early_alu_shift_value_dep")

(define_bypass 3 "marvell_f_load2"
               "marvell_f_alu_shift_reg_op"
	       "arm_no_early_alu_shift_dep")

;; 3. Load/store for more than two registers.

;; These instructions stall for an extra cycle in the decode stage;
;; individual load/store instructions for each register are then issued.
;; The load/store multiple instruction itself is removed from the decode
;; stage at the same time as the final load/store instruction is issued.
;; To complicate matters, pairs of loads/stores referencing two
;; consecutive registers will be issued together as doubleword operations.
;; We model a 3-word load as an LDR plus an LDRD, and a 4-word load
;; as two LDRDs; thus, these are allocated the same latencies (the
;; latency for two consecutive loads plus one for the setup stall).
;; The extra stall is modelled by reserving E1.

(define_insn_reservation "marvell_f_load3_4" 6
 (and (eq_attr "tune" "marvell_f")
      (eq_attr "type" "load3,load4"))
 "a1_e1,a1_e1+a1_e2+sram,a1_e2+sram+a1_of,a1_of+a1_wr,a1_wr+a1_wb,a1_wb")

;; Bypasses are possible for ldm as for single loads, but we do not
;; model them here since the order of the constituent loads is
;; difficult to predict.

(define_insn_reservation "marvell_f_store3_4" 5
 (and (eq_attr "tune" "marvell_f")
      (eq_attr "type" "store3,store4"))
 "a1_e1,a1_e1+a1_e2,a1_e2+a1_of,a1_of+a1_wr+sram,a1_wr+sram+a1_wb,a1_wb")

