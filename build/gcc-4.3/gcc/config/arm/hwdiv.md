;; ARM instruction patterns for hardware division 
;; Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
;; Written by CodeSourcery, LLC.
;;
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

(define_insn "divsi3"
  [(set (match_operand:SI	  0 "s_register_operand" "=r")
	(div:SI (match_operand:SI 1 "s_register_operand"  "r")
		(match_operand:SI 2 "s_register_operand"  "r")))]
  "arm_arch_hwdiv"
  "sdiv%?\t%0, %1, %2"
  [(set_attr "predicable" "yes")
   (set_attr "insn" "sdiv")]
)

(define_insn "udivsi3"
  [(set (match_operand:SI	   0 "s_register_operand" "=r")
	(udiv:SI (match_operand:SI 1 "s_register_operand"  "r")
		 (match_operand:SI 2 "s_register_operand"  "r")))]
  "arm_arch_hwdiv"
  "udiv%?\t%0, %1, %2"
  [(set_attr "predicable" "yes")
   (set_attr "insn" "udiv")]
)

