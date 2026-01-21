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

#ifndef XED_ADDRESS_WIDTH_ENUM_H
# define XED_ADDRESS_WIDTH_ENUM_H

typedef enum {
    XED_ADDRESS_WIDTH_INVALID = 0,
    XED_ADDRESS_WIDTH_16b = 16,
    XED_ADDRESS_WIDTH_32b = 32,
    XED_ADDRESS_WIDTH_64b = 64,
    XED_ADDRESS_WIDTH_LAST
} xed_address_width_enum_t;

#endif // XED_ADDRESS_WIDTH_ENUM_H
