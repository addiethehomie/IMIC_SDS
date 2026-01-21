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

#ifndef XED_CHIP_ENUM_H
# define XED_CHIP_ENUM_H

typedef enum {
    XED_CHIP_INVALID = 0,
    XED_CHIP_I86 = 1,
    XED_CHIP_I86_64 = 2,
    XED_CHIP_PENTIUM = 3,
    XED_CHIP_PENTIUM_PRO = 4,
    XED_CHIP_PENTIUM_MMX = 5,
    XED_CHIP_PENTIUM_2 = 6,
    XED_CHIP_PENTIUM_3 = 7,
    XED_CHIP_PENTIUM_4 = 8,
    XED_CHIP_CORE = 9,
    XED_CHIP_CORE_2 = 10,
    XED_CHIP_CORE_DUO = 11,
    XED_CHIP_CORE_2_DUO = 12,
    XED_CHIP_CORE_I7 = 13,
    XED_CHIP_NEHALEM = 14,
    XED_CHIP_WESTMERE = 15,
    XED_CHIP_SANDY_BRIDGE = 16,
    XED_CHIP_IVY_BRIDGE = 17,
    XED_CHIP_HASWELL = 18,
    XED_CHIP_BROADWELL = 19,
    XED_CHIP_SKYLAKE = 20,
    XED_CHIP_CASCADE_LAKE = 21,
    XED_CHIP_COFFEE_LAKE = 22,
    XED_CHIP_ICELAKE = 23,
    XED_CHIP_TIGER_LAKE = 24,
    XED_CHIP_ROCKET_LAKE = 25,
    XED_CHIP_ALDER_LAKE = 26,
    XED_CHIP_RAPTOR_LAKE = 27,
    
    // AVX-512 capable chips
    XED_CHIP_KNIGHTS_LANDING = 100,
    XED_CHIP_KNIGHTS_CORNER = 101,
    XED_CHIP_KNIGHTS_MILL = 102,
    XED_CHIP_KNIGHTS_HILL = 103,
    XED_CHIP_SKYLAKE_X = 104,
    XED_CHIP_CASCADE_LAKE_X = 105,
    XED_CHIP_COOPER_LAKE = 106,
    XED_CHIP_SAPPHIRE_RAPIDS = 107,
    XED_CHIP_EMERALD_RAPIDS = 108,
    
    XED_CHIP_LAST
} xed_chip_enum_t;

#endif // XED_CHIP_ENUM_H
