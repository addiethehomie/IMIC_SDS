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

#ifndef XED_ATTRIBUTE_ENUM_H
# define XED_ATTRIBUTE_ENUM_H

typedef enum {
    XED_ATTRIBUTE_INVALID = 0,
    XED_ATTRIBUTE_LOCK = 1,
    XED_ATTRIBUTE_REP = 2,
    XED_ATTRIBUTE_REPNE = 3,
    XED_ATTRIBUTE_BND = 4,
    XED_ATTRIBUTE_NOTRACK = 5,
    XED_ATTRIBUTE_LAST
} xed_attribute_enum_t;

#endif // XED_ATTRIBUTE_ENUM_H
