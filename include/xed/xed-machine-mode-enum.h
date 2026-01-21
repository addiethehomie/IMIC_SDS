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

#ifndef XED_MACHINE_MODE_ENUM_H
# define XED_MACHINE_MODE_ENUM_H

typedef enum {
    XED_MACHINE_MODE_INVALID = 0,
    XED_MACHINE_MODE_LEGACY_16 = 1,
    XED_MACHINE_MODE_LEGACY_32 = 2,
    XED_MACHINE_MODE_LONG_COMPAT_16 = 3,
    XED_MACHINE_MODE_LONG_COMPAT_32 = 4,
    XED_MACHINE_MODE_LONG_64 = 5,
    XED_MACHINE_MODE_REAL_16 = 6,
    XED_MACHINE_MODE_REAL_32 = 7,
    XED_MACHINE_MODE_LAST
} xed_machine_mode_enum_t;

#endif // XED_MACHINE_MODE_ENUM_H
