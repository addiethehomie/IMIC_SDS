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

#ifndef XED_EXTENSION_ENUM_H
# define XED_EXTENSION_ENUM_H

typedef enum {
    XED_EXTENSION_INVALID = 0,
    XED_EXTENSION_LEGACY = 1,
    XED_EXTENSION_3DNOW = 2,
    XED_EXTENSION_AMD = 3,
    XED_EXTENSION_BMI = 4,
    XED_EXTENSION_BMI2 = 5,
    XED_EXTENSION_CET = 6,
    XED_EXTENSION_F16C = 7,
    XED_EXTENSION_FMA3 = 8,
    XED_EXTENSION_FMA4 = 9,
    XED_EXTENSION_FMA = 10,
    XED_EXTENSION_X87 = 11,
    XED_EXTENSION_XOP = 12,
    XED_EXTENSION_AVX = 13,
    XED_EXTENSION_AVX2 = 14,
    XED_EXTENSION_AVX512 = 15,
    XED_EXTENSION_KNC = 16,
    XED_EXTENSION_KNC_GENERIC = 17,
    XED_EXTENSION_KNC_AVX512 = 18,
    XED_EXTENSION_LAST
} xed_extension_enum_t;

#endif // XED_EXTENSION_ENUM_H
