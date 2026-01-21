/* BEGIN_LEGAL 

Copyright (c) 2025 Intel Corporation

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  
END_LEGAL */

#ifndef XED_REG_ENUM_H
# define XED_REG_ENUM_H

typedef enum {
    XED_REG_INVALID = 0,
    XED_REG_AL = 1,
    XED_REG_CL = 2,
    XED_REG_DL = 3,
    XED_REG_BL = 4,
    XED_REG_AH = 5,
    XED_REG_CH = 6,
    XED_REG_DH = 7,
    XED_REG_BH = 8,
    XED_REG_AX = 9,
    XED_REG_CX = 10,
    XED_REG_DX = 11,
    XED_REG_BX = 12,
    XED_REG_SP = 13,
    XED_REG_BP = 14,
    XED_REG_SI = 15,
    XED_REG_DI = 16,
    XED_REG_EAX = 17,
    XED_REG_ECX = 18,
    XED_REG_EDX = 19,
    XED_REG_EBX = 20,
    XED_REG_ESP = 21,
    XED_REG_EBP = 22,
    XED_REG_ESI = 23,
    XED_REG_EDI = 24,
    XED_REG_RAX = 25,
    XED_REG_RCX = 26,
    XED_REG_RDX = 27,
    XED_REG_RBX = 28,
    XED_REG_RSP = 29,
    XED_REG_RBP = 30,
    XED_REG_RSI = 31,
    XED_REG_RDI = 32,
    XED_REG_R8 = 33,
    XED_REG_R9 = 34,
    XED_REG_R10 = 35,
    XED_REG_R11 = 36,
    XED_REG_R12 = 37,
    XED_REG_R13 = 38,
    XED_REG_R14 = 39,
    XED_REG_R15 = 40,
    
    // AVX-512 registers
    XED_REG_ZMM0 = 100,
    XED_REG_ZMM1 = 101,
    XED_REG_ZMM2 = 102,
    XED_REG_ZMM3 = 103,
    XED_REG_ZMM4 = 104,
    XED_REG_ZMM5 = 105,
    XED_REG_ZMM6 = 106,
    XED_REG_ZMM7 = 107,
    XED_REG_ZMM8 = 108,
    XED_REG_ZMM9 = 109,
    XED_REG_ZMM10 = 110,
    XED_REG_ZMM11 = 111,
    XED_REG_ZMM12 = 112,
    XED_REG_ZMM13 = 113,
    XED_REG_ZMM14 = 114,
    XED_REG_ZMM15 = 115,
    XED_REG_ZMM16 = 116,
    XED_REG_ZMM17 = 117,
    XED_REG_ZMM18 = 118,
    XED_REG_ZMM19 = 119,
    XED_REG_ZMM20 = 120,
    XED_REG_ZMM21 = 121,
    XED_REG_ZMM22 = 122,
    XED_REG_ZMM23 = 123,
    XED_REG_ZMM24 = 124,
    XED_REG_ZMM25 = 125,
    XED_REG_ZMM26 = 126,
    XED_REG_ZMM27 = 127,
    XED_REG_ZMM28 = 128,
    XED_REG_ZMM29 = 129,
    XED_REG_ZMM30 = 130,
    XED_REG_ZMM31 = 131,
    
    XED_REG_LAST
} xed_reg_enum_t;

#endif // XED_REG_ENUM_H
