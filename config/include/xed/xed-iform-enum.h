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

#ifndef XED_IFORM_ENUM_H
# define XED_IFORM_ENUM_H

typedef enum {
    XED_IFORM_INVALID = 0,
    XED_IFORM_NOP = 1,
    XED_IFORM_RET = 2,
    XED_IFORM_ADD_REG_REG = 3,
    XED_IFORM_ADD_REG_IMM = 4,
    XED_IFORM_ADD_MEM_REG = 5,
    XED_IFORM_SUB_REG_REG = 6,
    XED_IFORM_SUB_REG_IMM = 7,
    XED_IFORM_SUB_MEM_REG = 8,
    XED_IFORM_MUL_REG_REG = 9,
    XED_IFORM_DIV_REG_REG = 10,
    
    // AVX-512 instruction forms
    XED_IFORM_VPADDD_ZMM_ZMM_ZMM = 100,
    XED_IFORM_VPSUBD_ZMM_ZMM_ZMM = 101,
    XED_IFORM_VPMULUD_ZMM_ZMM_ZMM = 102,
    XED_IFORM_VPERMD_ZMM_ZMM_ZMM = 103,
    XED_IFORM_VPBROADCASTD_ZMM_REG = 104,
    XED_IFORM_VGATHERDPS_ZMM_MEM = 105,
    XED_IFORM_VSCATTERDPS_MEM_ZMM = 106,
    XED_IFORM_VCMPPS_ZMM_ZMM_IMM = 107,
    XED_IFORM_VMAXPS_ZMM_ZMM_ZMM = 108,
    XED_IFORM_VMINPS_ZMM_ZMM_ZMM = 109,
    
    XED_IFORM_LAST
} xed_iform_enum_t;

#endif // XED_IFORM_ENUM_H
