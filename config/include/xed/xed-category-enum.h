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

#ifndef XED_CATEGORY_ENUM_H
# define XED_CATEGORY_ENUM_H

typedef enum {
    XED_CATEGORY_INVALID = 0,
    XED_CATEGORY_BINARY = 1,
    XED_CATEGORY_BITBYTE = 2,
    XED_CATEGORY_CALL = 3,
    XED_CATEGORY_CMOV = 4,
    XED_CATEGORY_COND_BR = 5,
    XED_CATEGORY_CONVERT = 6,
    XED_CATEGORY_DATAXFER = 7,
    XED_CATEGORY_FCMOV = 8,
    XED_CATEGORY_FLAGOP = 9,
    XED_CATEGORY_INTERRUPT = 10,
    XED_CATEGORY_IO = 11,
    XED_CATEGORY_LOGICAL = 12,
    XED_CATEGORY_MISC = 13,
    XED_CATEGORY_NOP = 14,
    XED_CATEGORY_POP = 15,
    XED_CATEGORY_PREFETCH = 16,
    XED_CATEGORY_PUSH = 17,
    XED_CATEGORY_ROTATE = 18,
    XED_CATEGORY_SEMAPHORE = 19,
    XED_CATEGORY_SHIFT = 20,
    XED_CATEGORY_STRINGOP = 21,
    XED_CATEGORY_SYSCALL = 22,
    XED_CATEGORY_SYSRET = 23,
    XED_CATEGORY_UNCOND_BR = 24,
    XED_CATEGORY_VEX = 25,
    XED_CATEGORY_VECTOR = 26,
    XED_CATEGORY_3DNOW = 27,
    XED_CATEGORY_AVX = 28,
    XED_CATEGORY_AVX2 = 29,
    XED_CATEGORY_AVX512 = 30,
    XED_CATEGORY_KNC = 31,
    XED_CATEGORY_LAST
} xed_category_enum_t;

#endif // XED_CATEGORY_ENUM_H
