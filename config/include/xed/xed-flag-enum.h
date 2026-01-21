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

#ifndef XED_FLAG_ENUM_H
# define XED_FLAG_ENUM_H

typedef enum {
    XED_FLAG_INVALID = 0,
    XED_FLAG_CF = 1,
    XED_FLAG_PF = 2,
    XED_FLAG_AF = 3,
    XED_FLAG_ZF = 4,
    XED_FLAG_SF = 5,
    XED_FLAG_TF = 6,
    XED_FLAG_IF = 7,
    XED_FLAG_DF = 8,
    XED_FLAG_OF = 9,
    XED_FLAG_IOPL = 10,
    XED_FLAG_NT = 11,
    XED_FLAG_RF = 12,
    XED_FLAG_VM = 13,
    XED_FLAG_AC = 14,
    XED_FLAG_VIF = 15,
    XED_FLAG_VIP = 16,
    XED_FLAG_ID = 17,
    XED_FLAG_LAST
} xed_flag_enum_t;

#endif // XED_FLAG_ENUM_H
