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

#ifndef XED_OPERAND_WIDTH_ENUM_H
# define XED_OPERAND_WIDTH_ENUM_H

typedef enum {
    XED_OPERAND_WIDTH_INVALID = 0,
    XED_OPERAND_WIDTH_8 = 8,
    XED_OPERAND_WIDTH_16 = 16,
    XED_OPERAND_WIDTH_32 = 32,
    XED_OPERAND_WIDTH_64 = 64,
    XED_OPERAND_WIDTH_128 = 128,
    XED_OPERAND_WIDTH_256 = 256,
    XED_OPERAND_WIDTH_512 = 512,
    XED_OPERAND_WIDTH_1024 = 1024,
    XED_OPERAND_WIDTH_LAST
} xed_operand_width_enum_t;

#endif // XED_OPERAND_WIDTH_ENUM_H
