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

#ifndef XED_EXCEPTION_ENUM_H
# define XED_EXCEPTION_ENUM_H

typedef enum {
    XED_EXCEPTION_INVALID = 0,
    XED_EXCEPTION_DIVIDE = 1,
    XED_EXCEPTION_DEBUG = 2,
    XED_EXCEPTION_NMI = 3,
    XED_EXCEPTION_BREAKPOINT = 4,
    XED_EXCEPTION_OVERFLOW = 5,
    XED_EXCEPTION_BOUND = 6,
    XED_EXCEPTION_ILLEGAL = 7,
    XED_EXCEPTION_DEVICE_NOT_AVAILABLE = 8,
    XED_EXCEPTION_DOUBLE_FAULT = 9,
    XED_EXCEPTION_INVALID_TSS = 10,
    XED_EXCEPTION_SEGMENT_NOT_PRESENT = 11,
    XED_EXCEPTION_STACK_FAULT = 12,
    XED_EXCEPTION_GENERAL_PROTECTION = 13,
    XED_EXCEPTION_PAGE_FAULT = 14,
    XED_EXCEPTION_X87_FPU = 15,
    XED_EXCEPTION_ALIGNMENT_CHECK = 16,
    XED_EXCEPTION_MACHINE_CHECK = 17,
    XED_EXCEPTION_SIMD_FP = 18,
    XED_EXCEPTION_VIRTUALIZATION = 19,
    XED_EXCEPTION_SECURITY = 20,
    XED_EXCEPTION_LAST
} xed_exception_enum_t;

#endif // XED_EXCEPTION_ENUM_H
