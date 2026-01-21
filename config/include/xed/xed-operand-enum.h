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

#ifndef XED_OPERAND_ENUM_H
# define XED_OPERAND_ENUM_H

typedef enum {
    XED_OPERAND_INVALID = 0,
    XED_OPERAND_REG0 = 1,
    XED_OPERAND_REG1 = 2,
    XED_OPERAND_REG2 = 3,
    XED_OPERAND_REG3 = 4,
    XED_OPERAND_REG4 = 5,
    XED_OPERAND_REG5 = 6,
    XED_OPERAND_REG6 = 7,
    XED_OPERAND_REG7 = 8,
    XED_OPERAND_REG8 = 9,
    XED_OPERAND_REG9 = 10,
    XED_OPERAND_REG10 = 11,
    XED_OPERAND_REG11 = 12,
    XED_OPERAND_REG12 = 13,
    XED_OPERAND_REG13 = 14,
    XED_OPERAND_REG14 = 15,
    XED_OPERAND_REG15 = 16,
    XED_OPERAND_MEM0 = 17,
    XED_OPERAND_MEM1 = 18,
    XED_OPERAND_IMM0 = 19,
    XED_OPERAND_IMM1 = 20,
    XED_OPERAND_RELBR = 21,
    XED_OPERAND_AGEN = 22,
    XED_OPERAND_BASE0 = 23,
    XED_OPERAND_INDEX = 24,
    XED_OPERAND_SEG0 = 25,
    XED_OPERAND_BASE1 = 26,
    XED_OPERAND_SEG1 = 27,
    XED_OPERAND_LAST
} xed_operand_enum_t;

#endif // XED_OPERAND_ENUM_H
