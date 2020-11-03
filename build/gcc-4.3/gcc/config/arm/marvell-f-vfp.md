;; Marvell 2850 VFP pipeline description
;; Copyright (C) 2007 Free Software Foundation, Inc.
;; Written by CodeSourcery, Inc.

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

(define_automaton "marvell_f_vfp")

;; This is a single-issue VFPv2 implementation with the following execution
;; units:
;;
;; 1. Addition/subtraction unit; takes three cycles, pipelined.
;; 2. Multiplication unit; takes four cycles, pipelined.
;; 3. Add buffer, used for multiply-accumulate (see below).
;; 4. Divide/square root unit, not pipelined.
;;    For single-precision: takes sixteen cycles, can accept another insn
;;			    after fifteen cycles.
;;    For double-precision: takes thirty-one cycles, can accept another insn
;;			    after thirty cycles.
;; 5. Single-cycle unit, pipelined.
;;    This does absolute value/copy/negate/compare in one cycle and
;;    conversion in two cycles.
;;
;; When all three operands of a multiply-accumulate instruction are ready,
;; one is issued to the add buffer (which can hold six operands in a FIFO)
;; and the two to be multiplied are issued to the multiply unit.  After
;; four cycles in the multiply unit, one cycle is taken to issue the
;; operand from the add buffer plus the multiplication result to the
;; addition/subtraction unit.  That issue takes priority over any add/sub
;; instruction waiting at the normal issue stage, but may be performed in
;; parallel with the issue of a non-add/sub instruction.  The total time
;; for a multiply-accumulate instruction to pass through the execution
;; units is hence eight cycles.
;;
;; We do not need to explicitly model the add buffer because it can
;; always issue the instruction at the head of its FIFO (due to the above
;; priority rule) and there are more spaces in the add buffer (six) than
;; there are stages (four) in the multiplication unit.
;;
;; Two instructions may be retired at once from the head of an 8-entry
;; reorder buffer.  Data from these first two instructions only may be
;; forwarded to the inputs of the issue unit.  We assume that the
;; pressure on the reorder buffer will be sufficiently low that every
;; instruction entering it will be eligible for data forwarding.  Since
;; data is forwarded to the issue unit and not the execution units (so
;; for example single-cycle instructions cannot be issued back-to-back),
;; the latencies given below are the cycle counts above plus one.

(define_cpu_unit "mf_vfp_issue" "marvell_f_vfp")
(define_cpu_unit "mf_vfp_add" "marvell_f_vfp")
(define_cpu_unit "mf_vfp_mul" "marvell_f_vfp")
(define_cpu_unit "mf_vfp_div" "marvell_f_vfp")
(define_cpu_unit "mf_vfp_single_cycle" "marvell_f_vfp")

;; An attribute to indicate whether our reservations are applicable.

(define_attr "marvell_f_vfp" "yes,no"
  (const (if_then_else (and (eq_attr "tune" "marvell_f")
                            (eq_attr "fpu" "vfp"))
                       (const_string "yes") (const_string "no"))))

;; Reservations of functional units.  The nothing*2 reservations at the
;; start of many of the reservation strings correspond to the decode
;; stages.  We need to have these reservations so that we can correctly
;; reserve parts of the core's A1 pipeline for loads and stores.  For
;; that case (since loads skip E1) the pipelines line up thus:
;;	A1 pipe:	Issue	E2	OF	WR	WB	 ...
;;	VFP pipe:	Fetch	Decode1	Decode2	Issue	Execute1 ...
;; For a load, we need to make a reservation of E2, and thus we must
;; use Decode1 as the starting point for all VFP reservations here.
;;
;; For reservations of pipelined VFP execution units we only reserve
;; the execution unit for the first execution cycle, omitting any trailing
;; "nothing" reservations.

(define_insn_reservation "marvell_f_vfp_add" 4
  (and (eq_attr "marvell_f_vfp" "yes")
       (eq_attr "type" "fadds,faddd"))
  "nothing*2,mf_vfp_issue,mf_vfp_add")

(define_insn_reservation "marvell_f_vfp_mul" 5
  (and (eq_attr "marvell_f_vfp" "yes")
       (eq_attr "type" "fmuls,fmuld"))
  "nothing*2,mf_vfp_issue,mf_vfp_mul")

(define_insn_reservation "marvell_f_vfp_divs" 17
  (and (eq_attr "marvell_f_vfp" "yes")
       (eq_attr "type" "fdivs"))
  "nothing*2,mf_vfp_issue,mf_vfp_div*15")

(define_insn_reservation "marvell_f_vfp_divd" 32
  (and (eq_attr "marvell_f_vfp" "yes")
       (eq_attr "type" "fdivd"))
  "nothing*2,mf_vfp_issue,mf_vfp_div*30")

;; The DFA lookahead is small enough that the "add" reservation here
;; will always take priority over any addition/subtraction instruction
;; issued five cycles after the multiply-accumulate instruction, as
;; required.
(define_insn_reservation "marvell_f_vfp_mac" 9
  (and (eq_attr "marvell_f_vfp" "yes")
       (eq_attr "type" "fmacs,fmacd"))
  "nothing*2,mf_vfp_issue,mf_vfp_mul,nothing*4,mf_vfp_add")

(define_insn_reservation "marvell_f_vfp_single" 2
  (and (eq_attr "marvell_f_vfp" "yes")
       (eq_attr "type" "fcpys,ffariths,ffarithd,fcmps,fcmpd"))
  "nothing*2,mf_vfp_issue,mf_vfp_single_cycle")

(define_insn_reservation "marvell_f_vfp_convert" 3
  (and (eq_attr "marvell_f_vfp" "yes")
       (eq_attr "type" "f_cvt"))
  "nothing*2,mf_vfp_issue,mf_vfp_single_cycle")

(define_insn_reservation "marvell_f_vfp_load" 2
  (and (eq_attr "marvell_f_vfp" "yes")
       (eq_attr "type" "f_loads,f_loadd"))
  "a1_e2+sram,a1_of,a1_wr+mf_vfp_issue,a1_wb+mf_vfp_single_cycle")

(define_insn_reservation "marvell_f_vfp_from_core" 2
  (and (eq_attr "marvell_f_vfp" "yes")
       (eq_attr "type" "r_2_f"))
  "a1_e2,a1_of,a1_wr+mf_vfp_issue,a1_wb+mf_vfp_single_cycle")

;; The interaction between the core and VFP pipelines during VFP
;; store operations and core <-> VFP moves is not clear, so we guess.
(define_insn_reservation "marvell_f_vfp_store" 3
  (and (eq_attr "marvell_f_vfp" "yes")
       (eq_attr "type" "f_stores,f_stored"))
  "a1_e2,a1_of,mf_vfp_issue,a1_wr+sram+mf_vfp_single_cycle")

(define_insn_reservation "marvell_f_vfp_to_core" 4
  (and (eq_attr "marvell_f_vfp" "yes")
       (eq_attr "type" "f_2_r"))
  "a1_e2,a1_of,a1_wr+mf_vfp_issue,a1_wb+mf_vfp_single_cycle")

