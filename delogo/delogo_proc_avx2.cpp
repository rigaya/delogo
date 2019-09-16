﻿#define USE_SSE2  1
#define USE_SSSE3 1
#define USE_SSE41 1
#define USE_AVX   1
#define USE_AVX2  1
#define USE_FMA3  1
#include "delogo_proc_simd.h"
#include "delogo_proc.h"

BOOL __stdcall func_proc_eraze_logo_avx2(FILTER *fp, FILTER_PROC_INFO *fpip, LOGO_HEADER *lgh, int fade) {
    return func_proc_eraze_logo_simd(fp, fpip, lgh, fade);
}
