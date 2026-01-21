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

#ifndef XED_ERROR_ENUM_H
# define XED_ERROR_ENUM_H

typedef enum {
    XED_ERROR_NONE = 0,
    XED_ERROR_GENERAL_ERROR = 1,
    XED_ERROR_BUFFER_TOO_SHORT = 2,
    XED_ERROR_INVALID_MODE = 3,
    XED_ERROR_BAD_REGISTER = 4,
    XED_ERROR_BAD_INSTRUCTION = 5
} xed_error_enum_t;

// Function to convert error enum to string
static inline const char* xed_error_enum_t2str(xed_error_enum_t err) {
    switch (err) {
        case XED_ERROR_NONE: return "XED_ERROR_NONE";
        case XED_ERROR_GENERAL_ERROR: return "XED_ERROR_GENERAL_ERROR";
        case XED_ERROR_BUFFER_TOO_SHORT: return "XED_ERROR_BUFFER_TOO_SHORT";
        case XED_ERROR_INVALID_MODE: return "XED_ERROR_INVALID_MODE";
        case XED_ERROR_BAD_REGISTER: return "XED_ERROR_BAD_REGISTER";
        case XED_ERROR_BAD_INSTRUCTION: return "XED_ERROR_BAD_INSTRUCTION";
        default: return "UNKNOWN_ERROR";
    }
}

#endif // XED_ERROR_ENUM_H
