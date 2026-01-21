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

#ifndef XED_ISA_SET_ENUM_H
# define XED_ISA_SET_ENUM_H

typedef enum {
    XED_ISA_SET_INVALID = 0,
    XED_ISA_SET_I86 = 1,
    XED_ISA_SET_I386 = 2,
    XED_ISA_SET_I486 = 3,
    XED_ISA_SET_PENTIUM = 4,
    XED_ISA_SET_PENTIUMPRO = 5,
    XED_ISA_SET_PENTIUMMMX = 6,
    XED_ISA_SET_PENTIUM2 = 7,
    XED_ISA_SET_PENTIUM3 = 8,
    XED_ISA_SET_PENTIUM4 = 9,
    XED_ISA_SET_CORE = 10,
    XED_ISA_SET_CORE2 = 11,
    XED_ISA_SET_COREDUO = 12,
    XED_ISA_SET_CORE2DUO = 13,
    XED_ISA_SET_COREI7 = 14,
    XED_ISA_SET_NEHALEM = 15,
    XED_ISA_SET_WESTMERE = 16,
    XED_ISA_SET_SANDYBRIDGE = 17,
    XED_ISA_SET_IVYBRIDGE = 18,
    XED_ISA_SET_HASWELL = 19,
    XED_ISA_SET_BROADWELL = 20,
    XED_ISA_SET_SKYLAKE = 21,
    XED_ISA_SET_CASCADELAKE = 22,
    XED_ISA_SET_COFFEELAKE = 23,
    XED_ISA_SET_ICELAKE = 24,
    XED_ISA_SET_TIGERLAKE = 25,
    XED_ISA_SET_ROCKETLAKE = 26,
    XED_ISA_SET_ALDERLAKE = 27,
    XED_ISA_SET_RAPTORLAKE = 28,
    
    // AVX-512 ISA sets
    XED_ISA_SET_AVX512F = 100,
    XED_ISA_SET_AVX512CD = 101,
    XED_ISA_SET_AVX512ER = 102,
    XED_ISA_SET_AVX512PF = 103,
    XED_ISA_SET_AVX512BW = 104,
    XED_ISA_SET_AVX512DQ = 105,
    XED_ISA_SET_AVX512VL = 106,
    XED_ISA_SET_AVX512IFMA = 107,
    XED_ISA_SET_AVX512VBMI = 108,
    XED_ISA_SET_AVX512VPOPCNTDQ = 109,
    XED_ISA_SET_AVX512_4FMAPS = 110,
    XED_ISA_SET_AVX512_VPCLMULQDQ = 111,
    XED_ISA_SET_AVX512_VNNI = 112,
    XED_ISA_SET_AVX512_BITALG = 113,
    XED_ISA_SET_AVX512_VBMI2 = 114,
    XED_ISA_SET_AVX512_BF16 = 115,
    XED_ISA_SET_AVX512_FP16 = 116,
    
    // KNC ISA sets
    XED_ISA_SET_KNC = 200,
    XED_ISA_SET_KNC_GENERIC = 201,
    XED_ISA_SET_KNC_AVX512 = 202,
    
    XED_ISA_SET_LAST
} xed_isa_set_enum_t;

#endif // XED_ISA_SET_ENUM_H
