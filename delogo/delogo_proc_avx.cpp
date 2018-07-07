#define USE_SSE2   1
#define USE_SSE2   1
#define USE_SSSE3  1
#define USE_SSE41  1
#define USE_POPCNT 1
#define USE_AVX    1
#define USE_AVX2   0
#define USE_FMA3   0
#include "delogo_proc_simd.h"
#include "smooth_simd.h"
#include "delogo_proc.h"

BOOL __stdcall func_proc_eraze_logo_avx(FILTER *fp, FILTER_PROC_INFO *fpip, const LOGO_HEADER *lgh, int fade) {
    return func_proc_eraze_logo_simd(fp, fpip, lgh, fade);
}

void __stdcall func_proc_eraze_logo_y_avx(short *buffer_dst, const short *buffer_src, FILTER *fp, const LOGO_HEADER *lgh, int fade) {
    func_proc_eraze_logo_y_simd(buffer_dst, buffer_src, fp, lgh, fade);
}

void __stdcall func_proc_prewitt_filter_avx(short *logo_mask, const LOGO_HEADER *lgh) {
    func_proc_prewitt_filter_simd(logo_mask, lgh);
}

void __stdcall func_proc_weighted_prewitt_filter_avx(short *dst, const short *src, int logo_width, int logo_src_pitch, int height) {
    func_proc_weighted_prewitt_filter_simd(dst, src, logo_width, logo_src_pitch, height);
}

unsigned int __stdcall func_proc_prewitt_evaluate_avx(const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result) {
    return func_proc_prewitt_evaluate_simd(logo_mask, src, lgh, auto_result);
}

unsigned int __stdcall func_proc_prewitt_5x5_evaluate_avx(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result) {
    return func_proc_prewitt_5x5_evaluate_simd<false>(result_buf, logo_mask, src, lgh, auto_result);
}

unsigned int __stdcall func_proc_prewitt_5x5_evaluate_store_avx(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result) {
    return func_proc_prewitt_5x5_evaluate_simd<true>(result_buf, logo_mask, src, lgh, auto_result);
}



void __stdcall func_smooth_3x3_avx(short *dst, short *src, const short *mask, int width, int src_pitch, int height) {
    smooth_simd<1>(dst, src, mask, width, src_pitch, height);
}

void __stdcall func_smooth_5x5_avx(short *dst, short *src, const short *mask, int width, int src_pitch, int height) {
    smooth_simd<2>(dst, src, mask, width, src_pitch, height);
}

void __stdcall func_smooth_7x7_avx(short *dst, short *src, const short *mask, int width, int src_pitch, int height) {
    smooth_simd<3>(dst, src, mask, width, src_pitch, height);
}

void __stdcall func_smooth_9x9_avx(short *dst, short *src, const short *mask, int width, int src_pitch, int height) {
    smooth_simd<4>(dst, src, mask, width, src_pitch, height);
}

void __stdcall func_smooth_yc48_3x3_avx(PIXEL_YC *ptr, const short *mask, int width, int src_pitch, int height) {
    smooth_simd<1>(nullptr, ptr, mask, width, src_pitch, height);
}

void __stdcall func_smooth_yc48_5x5_avx(PIXEL_YC *ptr, const short *mask, int width, int src_pitch, int height) {
    smooth_simd<2>(nullptr, ptr, mask, width, src_pitch, height);
}

void __stdcall func_smooth_yc48_7x7_avx(PIXEL_YC *ptr, const short *mask, int width, int src_pitch, int height) {
    smooth_simd<3>(nullptr, ptr, mask, width, src_pitch, height);
}

void __stdcall func_smooth_yc48_9x9_avx(PIXEL_YC *ptr, const short *mask, int width, int src_pitch, int height) {
    smooth_simd<4>(nullptr, ptr, mask, width, src_pitch, height);
}

int __stdcall func_count_logo_valid_pixels_avx(const short *mask, int width, int logo_pitch, int height) {
    return count_logo_valid_pixels_simd(mask, width, logo_pitch, height);
}
