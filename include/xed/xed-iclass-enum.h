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

#ifndef XED_ICLASS_ENUM_H
# define XED_ICLASS_ENUM_H

typedef enum {
    XED_ICLASS_INVALID = 0,
    
    // Basic x86 instructions
    XED_ICLASS_NOP = 1,
    XED_ICLASS_RET = 2,
    XED_ICLASS_ADD = 3,
    XED_ICLASS_SUB = 4,
    XED_ICLASS_MUL = 5,
    XED_ICLASS_DIV = 6,
    
    // AVX-512 / KNC vector instructions
    XED_ICLASS_VPADDD = 100,
    XED_ICLASS_VPSUBD = 101,
    XED_ICLASS_VPMULUD = 102,
    XED_ICLASS_VPERMD = 103,
    XED_ICLASS_VPBROADCASTD = 104,
    XED_ICLASS_VGATHERDPS = 105,
    XED_ICLASS_VSCATTERDPS = 106,
    XED_ICLASS_VCMPPS = 107,
    XED_ICLASS_VMAXPS = 108,
    XED_ICLASS_VMINPS = 109,
    
    // KNC-specific instructions
    XED_ICLASS_KNC_VPADDD = 200,
    XED_ICLASS_KNC_VPSUBD = 201,
    XED_ICLASS_KNC_VPMULUD = 202,
    XED_ICLASS_KNC_VPERMD = 203,
    XED_ICLASS_KNC_VPBROADCASTD = 204,
    XED_ICLASS_KNC_VGATHERDPS = 205,
    XED_ICLASS_KNC_VSCATTERDPS = 206,
    XED_ICLASS_KNC_VCMPPS = 207,
    XED_ICLASS_KNC_VMAXPS = 208,
    XED_ICLASS_KNC_VMINPS = 209,
    
    XED_ICLASS_LAST
} xed_iclass_enum_t;

#endif // XED_ICLASS_ENUM_H
