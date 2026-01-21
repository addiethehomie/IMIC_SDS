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

#ifndef XED_OPERAND_ELEMENT_XTYPE_ENUM_H
# define XED_OPERAND_ELEMENT_XTYPE_ENUM_H

typedef enum {
    XED_OPERAND_ELEMENT_XTYPE_INVALID = 0,
    XED_OPERAND_ELEMENT_XTYPE_INT8 = 1,
    XED_OPERAND_ELEMENT_XTYPE_INT16 = 2,
    XED_OPERAND_ELEMENT_XTYPE_INT32 = 3,
    XED_OPERAND_ELEMENT_XTYPE_INT64 = 4,
    XED_OPERAND_ELEMENT_XTYPE_FLOAT32 = 5,
    XED_OPERAND_ELEMENT_XTYPE_FLOAT64 = 6,
    XED_OPERAND_ELEMENT_XTYPE_LAST
} xed_operand_element_xtype_enum_t;

#endif // XED_OPERAND_ELEMENT_XTYPE_ENUM_H
