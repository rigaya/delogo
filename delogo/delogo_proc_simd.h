#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "filter.h"
#include "logo.h"
#include "logodef.h"
#include <algorithm>
#include <emmintrin.h> //SSE2
#if USE_SSSE3
#include <tmmintrin.h> //SSSE3
#endif
#if USE_SSE41
#include <smmintrin.h> //SSE4.1
#endif
#if USE_AVX
#include <immintrin.h> //AVX
#endif

#if !USE_SSE2
#include <cmath>
#endif

#define PITCH(x) (((x) + 15) & (~15))

#if USE_AVX2
#define MEM_ALIGN 32
#else
#define MEM_ALIGN 16
#endif

#pragma warning (push)
#pragma warning (disable: 4752) // Intel(R) AVX 命令が見つかりました。/arch:AVX を使用することを検討してください

//SSSE3のpalignrもどき
#define palignr_sse2(a,b,i) _mm_or_si128( _mm_slli_si128(a, 16-i), _mm_srli_si128(b, i) )

#if USE_SSSE3
#define _mm_alignr_epi8_simd _mm_alignr_epi8
#else
#define _mm_alignr_epi8_simd palignr_sse2
#endif

#if USE_SSE41
#define _mm_blendv_epi8_simd _mm_blendv_epi8
#else
static inline __m128i select_by_mask(__m128i a, __m128i b, __m128i mask) {
    return _mm_or_si128( _mm_andnot_si128(mask,a), _mm_and_si128(b,mask) );
}
#define _mm_blendv_epi8_simd select_by_mask
#endif

static const __declspec(align(MEM_ALIGN)) USHORT MASK_16BIT[] = {
    0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000,
#if USE_AVX2
    0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000
#endif
};

#if USE_AVX2
//本来の256bit alignr
#define MM_ABS(x) (((x) < 0) ? -(x) : (x))
#define _mm256_alignr256_epi8(a, b, i) ((i<=16) ? _mm256_alignr_epi8(_mm256_permute2x128_si256(a, b, (0x00<<4) + 0x03), b, i) : _mm256_alignr_epi8(a, _mm256_permute2x128_si256(a, b, (0x00<<4) + 0x03), MM_ABS(i-16)))

//_mm256_srli_si256, _mm256_slli_si256は
//単に128bitシフト×2をするだけの命令である
#define _mm256_bsrli_epi128 _mm256_srli_si256
#define _mm256_bslli_epi128 _mm256_slli_si256
//本当の256bitシフト
#define _mm256_srli256_si256(a, i) ((i<=16) ? _mm256_alignr_epi8(_mm256_permute2x128_si256(a, a, (0x08<<4) + 0x03), a, i) : _mm256_bsrli_epi128(_mm256_permute2x128_si256(a, a, (0x08<<4) + 0x03), MM_ABS(i-16)))
#define _mm256_slli256_si256(a, i) ((i<=16) ? _mm256_alignr_epi8(a, _mm256_permute2x128_si256(a, a, (0x00<<4) + 0x08), MM_ABS(16-i)) : _mm256_bslli_epi128(_mm256_permute2x128_si256(a, a, (0x00<<4) + 0x08), MM_ABS(i-16)))


static const _declspec(align(MEM_ALIGN)) unsigned int ARRAY_0x8000[2][8] = {
    { 0x80008000, 0x80008000, 0x80008000, 0x80008000, 0x80008000, 0x80008000, 0x80008000, 0x80008000 },
    { 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000 },
};
static __forceinline __m256i cvtlo256_epi16_epi32(__m256i y0) {
    __m256i yWordsHi = _mm256_cmpgt_epi16(_mm256_setzero_si256(), y0);
    return _mm256_unpacklo_epi16(y0, yWordsHi);
}

static __forceinline __m256i cvthi256_epi16_epi32(__m256i y0) {
    __m256i yWordsHi = _mm256_cmpgt_epi16(_mm256_setzero_si256(), y0);
    return _mm256_unpackhi_epi16(y0, yWordsHi);
}

static __forceinline __m256i _mm256_neg_epi32(__m256i y) {
    return _mm256_sub_epi32(_mm256_setzero_si256(), y);
}
static __forceinline __m256i _mm256_neg_epi16(__m256i y) {
    return _mm256_sub_epi16(_mm256_setzero_si256(), y);
}
static __forceinline __m256 _mm256_rcp_ps_hp(__m256 y0) {
    __m256 y1, y2;
    y1 = _mm256_rcp_ps(y0);
    y0 = _mm256_mul_ps(y0, y1);
    y2 = _mm256_add_ps(y1, y1);
#if USE_FMA3
    y2 = _mm256_fnmadd_ps(y0, y1, y2);
#else
    y0 = _mm256_mul_ps(y0, y1);
    y2 = _mm256_sub_ps(y2, y0);
#endif
    return y2;
}
//約22cycle
static __forceinline __m256 _mm256_sqrt_ps_hp(__m256 y0) {
    __m256 yRsqrt, y0_Rsqrt, yNegHalfRsqrt;
    yRsqrt        = _mm256_rsqrt_ps(y0);
    y0_Rsqrt      = _mm256_mul_ps(yRsqrt, y0);
    yNegHalfRsqrt = _mm256_mul_ps(yRsqrt, _mm256_set1_ps(-0.5f));
#if USE_FMA3
    y0 = _mm256_fmadd_ps(yNegHalfRsqrt, y0_Rsqrt, _mm256_set1_ps(-1.5f));
#else
    y0 = _mm256_add_ps(_mm256_mul_ps(yNegHalfRsqrt, y0_Rsqrt), _mm256_set1_ps(-1.5f));
#endif
    return _mm256_mul_ps(y0, y0_Rsqrt);
}

static __forceinline __m256i _mm256_alpha_max_plus_beta_min_epi16(__m256i yA, __m256i yB) {
    //参考... https://en.wikipedia.org/wiki/Alpha_max_plus_beta_min_algorithm
    //z = sqrt(x*x + y*y)
    //を以下のように近似する
    //z = alpha * max(abs(x), abs(y)) + beta * min(abs(x), abs(y))
    //alpha = 0.96043387... = 123 / 128
    //beta  = 0.3978273...  =  51 / 128
    static const __m256i yAlpha_Beta = _mm256_set1_epi32((51 << 16) | 123);
    __m256i yAbsA = _mm256_abs_epi16(yA);
    __m256i yAbsB = _mm256_abs_epi16(yB);
    __m256i yMax = _mm256_max_epi16(yAbsA, yAbsB);
    __m256i yMin = _mm256_min_epi16(yAbsA, yAbsB);
    __m256i y0 = _mm256_unpacklo_epi16(yMax, yMin);
    __m256i y1 = _mm256_unpackhi_epi16(yMax, yMin);
    y0 = _mm256_madd_epi16(y0, yAlpha_Beta);
    y1 = _mm256_madd_epi16(y1, yAlpha_Beta);
    y0 = _mm256_srai_epi32(y0, 7);
    y1 = _mm256_srai_epi32(y1, 7);
    //x0, x1ともに正の整数で、かつshortの範囲を超える可能性があるので、
    //unsignedとして扱うことで飽和を回避する
    return _mm256_packus_epi32(y0, y1);
}
static inline int limit_1_to_16(int value) {
    int cmp_ret = (value>=16);
    return (cmp_ret<<4) + ((value & 0x0f) & (cmp_ret-1)) + (value == 0);
}
#elif USE_SSE2
static const _declspec(align(MEM_ALIGN)) unsigned int ARRAY_0x8000[2][4] = {
    { 0x80008000, 0x80008000, 0x80008000, 0x80008000 },
    { 0x00008000, 0x00008000, 0x00008000, 0x00008000 },
};
static __forceinline __m128i _mm_neg_epi32(__m128i y) {
    return _mm_sub_epi32(_mm_setzero_si128(), y);
}
static __forceinline __m128i _mm_neg_epi16(__m128i y) {
    return _mm_sub_epi16(_mm_setzero_si128(), y);
}
static __forceinline __m128 _mm_rcp_ps_hp(__m128 x0) {
    __m128 x1, x2;
    x1 = _mm_rcp_ps(x0);
    x0 = _mm_mul_ps(x0, x1);
    x2 = _mm_add_ps(x1, x1);
    x0 = _mm_mul_ps(x0, x1);
    x2 = _mm_sub_ps(x2, x0);
    return x2;
}
//約20cycle at Haswell
static __forceinline __m128 _mm_sqrt_ps_hp(__m128 x0) {
    __m128 xRsqrt, x0_Rsqrt, xNegHalfRsqrt;
    xRsqrt        = _mm_rsqrt_ps(x0);
    x0_Rsqrt      = _mm_mul_ps(xRsqrt, x0);
    xNegHalfRsqrt = _mm_mul_ps(xRsqrt, _mm_set1_ps(-0.5f));
#if USE_FMA3
    x0 = _mm_fmadd_ps(xNegHalfRsqrt, x0_Rsqrt, _mm_set1_ps(-1.5f));
#else
    x0 = _mm_add_ps(_mm_mul_ps(xNegHalfRsqrt, x0_Rsqrt), _mm_set1_ps(-1.5f));
#endif
    return _mm_mul_ps(x0, x0_Rsqrt);
}

static inline __m128i _mm_abs_epi16_simd(__m128i x0) {
#if USE_SSSE3
    x0 = _mm_abs_epi16(x0);
#else
    __m128i x1;
    x1 = _mm_setzero_si128();
    x1 = _mm_cmpgt_epi16(x1, x0);
    x0 = _mm_xor_si128(x0, x1);
    x0 = _mm_subs_epi16(x0, x1);
#endif
    return x0;
}

static __forceinline __m128i _mm_packus_epi32_simd(__m128i a, __m128i b) {
#if USE_SSE41
    return _mm_packus_epi32(a, b);
#else
    static const _declspec(align(64)) DWORD VAL[2][4] = {
        { 0x00008000, 0x00008000, 0x00008000, 0x00008000 },
        { 0x80008000, 0x80008000, 0x80008000, 0x80008000 }
    };
#define LOAD_32BIT_0x8000 _mm_load_si128((__m128i *)VAL[0])
#define LOAD_16BIT_0x8000 _mm_load_si128((__m128i *)VAL[1])
    a = _mm_sub_epi32(a, LOAD_32BIT_0x8000);
    b = _mm_sub_epi32(b, LOAD_32BIT_0x8000);
    a = _mm_packs_epi32(a, b);
    return _mm_add_epi16(a, LOAD_16BIT_0x8000);
#undef LOAD_32BIT_0x8000
#undef LOAD_16BIT_0x8000
#endif
}

static __forceinline __m128i _mm_alpha_max_plus_beta_min_epi16(__m128i xA, __m128i xB) {
    //参考... https://en.wikipedia.org/wiki/Alpha_max_plus_beta_min_algorithm
    //z = sqrt(x*x + y*y)
    //を以下のように近似する
    //z = alpha * max(abs(x), abs(y)) + beta * min(abs(x), abs(y))
    //alpha = 0.96043387... = 123 / 128
    //beta  = 0.3978273...  =  51 / 128
    static const __m128i xAlpha_Beta = _mm_set1_epi32((51 << 16) | 123);
    __m128i xAbsA = _mm_abs_epi16_simd(xA);
    __m128i xAbsB = _mm_abs_epi16_simd(xB);
    __m128i xMax = _mm_max_epi16(xAbsA, xAbsB);
    __m128i xMin = _mm_min_epi16(xAbsA, xAbsB);
    __m128i x0 = _mm_unpacklo_epi16(xMax, xMin);
    __m128i x1 = _mm_unpackhi_epi16(xMax, xMin);
    x0 = _mm_madd_epi16(x0, xAlpha_Beta);
    x1 = _mm_madd_epi16(x1, xAlpha_Beta);
    x0 = _mm_srai_epi32(x0, 7);
    x1 = _mm_srai_epi32(x1, 7);
    //x0, x1ともに正の整数で、かつshortの範囲を超える可能性があるので、
    //unsignedとして扱うことで飽和を回避する
    return _mm_packus_epi32_simd(x0, x1);
}

static __forceinline __m128i _mm_mullo_epi32_simd(__m128i x0, __m128i x1) {
#if USE_SSE41
    return _mm_mullo_epi32(x0, x1);
#else
    __m128i x2 = _mm_mul_epu32(x0, x1);
    __m128i x3 = _mm_mul_epu32(_mm_shuffle_epi32(x0, 0xB1), _mm_shuffle_epi32(x1, 0xB1));

    x2 = _mm_shuffle_epi32(x2, 0xD8);
    x3 = _mm_shuffle_epi32(x3, 0xD8);

    return _mm_unpacklo_epi32(x2, x3);
#endif
}

static __forceinline __m128i cvtlo_epi16_epi32(__m128i x0) {
#if USE_SSE41
    return _mm_cvtepi16_epi32(x0);
#else
    __m128i xWordsHi = _mm_cmpgt_epi16(_mm_setzero_si128(), x0);
    return _mm_unpacklo_epi16(x0, xWordsHi);
#endif
}

static __forceinline __m128i cvthi_epi16_epi32(__m128i x0) {
#if USE_SSE41
    return _mm_cvtepi16_epi32(_mm_srli_si128(x0, 8));
#else
    __m128i xWordsHi = _mm_cmpgt_epi16(_mm_setzero_si128(), x0);
    return _mm_unpackhi_epi16(x0, xWordsHi);
#endif
}

static __forceinline __m128i blendv_epi8_simd(__m128i a, __m128i b, __m128i mask) {
#if USE_SSE41
    return _mm_blendv_epi8(a, b, mask);
#else
    return _mm_or_si128( _mm_andnot_si128(mask,a), _mm_and_si128(b,mask) );
#endif
}
static inline int limit_1_to_8(int value) {
    int cmp_ret = (value>=8);
    return (cmp_ret<<3) + ((value & 0x07) & (cmp_ret-1)) + (value == 0);
}
#if USE_POPCNT
#define popcnt32(x) _mm_popcnt_u32(x)
#else
static inline int popcnt32_c(DWORD bits) {
    bits = (bits & 0x55555555) + (bits >> 1 & 0x55555555);
    bits = (bits & 0x33333333) + (bits >> 2 & 0x33333333);
    bits = (bits & 0x0f0f0f0f) + (bits >> 4 & 0x0f0f0f0f);
    bits = (bits & 0x00ff00ff) + (bits >> 8 & 0x00ff00ff);
    bits = (bits & 0x0000ffff) + (bits >>16 & 0x0000ffff);
    return bits;
}
#define popcnt32(x) popcnt32_c(x)
#endif
#else
#define Clamp(n,l,h) ((n<l) ? l : (n>h) ? h : n)
#endif

static const __declspec(align(MEM_ALIGN)) USHORT YC48_MASK[] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
#if USE_AVX2
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
#endif
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
};
/*--------------------------------------------------------------------
* 	func_proc_eraze_logo()	ロゴ除去モード
*-------------------------------------------------------------------*/
static __forceinline BOOL func_proc_eraze_logo_simd(FILTER *fp, FILTER_PROC_INFO *fpip, const LOGO_HEADER *lgh, int fade) {
    const int max_w = fpip->max_w;
    const int logo_width = lgh->w;
    const int logo_depth_mul_fade = fp->track[2] * fade;
    const short py_offset = (short)(fp->track[3] << 4);
    const short cb_offset = (short)(fp->track[4] << 4);
    const short cr_offset = (short)(fp->track[5] << 4);

    // LOGO_PIXELデータへのポインタ
    LOGO_PIXEL *lgp_line = (LOGO_PIXEL *)(lgh + 1);

    // 左上の位置へ移動
    PIXEL_YC *ycp_line = fpip->ycp_edit + lgh->x + lgh->y * max_w;
    const int y_fin = std::min(fpip->h - lgh->y, (int)lgh->h);
    const int x_fin = std::min(fpip->w - lgh->x, (int)lgh->w);

    static const int Y_MIN = -128, Y_MAX = 4096+128;
    static const int C_MIN = -128-2048, C_MAX = 128+2048;
    static const __declspec(align(MEM_ALIGN)) short YC48_MIN[] = {
    #if USE_AVX2
        Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN,
        C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN,
        C_MIN, C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN, C_MIN, C_MIN, C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN, C_MIN
    #else
        Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN,
        C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN,
        C_MIN, C_MIN, Y_MIN, C_MIN, C_MIN, Y_MIN, C_MIN, C_MIN
    #endif
    };
    static const __declspec(align(MEM_ALIGN)) short YC48_MAX[] = {
    #if USE_AVX2
        Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX,
        C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX,
        C_MAX, C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX, C_MAX, C_MAX, C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX, C_MAX
    #else
        Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX,
        C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX,
        C_MAX, C_MAX, Y_MAX, C_MAX, C_MAX, Y_MAX, C_MAX, C_MAX
    #endif
    };

    __declspec(align(MEM_ALIGN)) short YC48_OFFSET[24<<(USE_AVX2 ? 1 : 0)] = { 0 };
    {
#if USE_AVX2
        for (int i = 0; i < 24;) {
            YC48_OFFSET[((i&(~7))<<1)+(i&7)] = py_offset; i++;
            YC48_OFFSET[((i&(~7))<<1)+(i&7)] = cb_offset; i++;
            YC48_OFFSET[((i&(~7))<<1)+(i&7)] = cr_offset; i++;
        }
        __m128i x0 = _mm_load_si128((__m128i *)(YC48_OFFSET +  0));
        __m128i x1 = _mm_load_si128((__m128i *)(YC48_OFFSET + 16));
        __m128i x2 = _mm_load_si128((__m128i *)(YC48_OFFSET + 32));
        _mm_store_si128((__m128i *)(YC48_OFFSET +  8), x0);
        _mm_store_si128((__m128i *)(YC48_OFFSET + 24), x1);
        _mm_store_si128((__m128i *)(YC48_OFFSET + 40), x2);
#else
        for (int i = 0; i < 8; i++) {
            YC48_OFFSET[3*i+0] = py_offset;
            YC48_OFFSET[3*i+1] = cb_offset;
            YC48_OFFSET[3*i+2] = cr_offset;
        }
#endif
    }

    for (int y = 0; y < y_fin; y++, ycp_line += max_w, lgp_line += logo_width) {
        PIXEL_YC *ycp = ycp_line;
        LOGO_PIXEL *lgp = lgp_line;
#if USE_AVX2
        for (int x = x_fin - 16, step = 16; x >= 0; x -= step, lgp += step, ycp += step) {
            __m256i y0, y1, y2, y3, y4, y5, yDp0, yDp1, yDp2, yDp3, yDp4, yDp5, ySrc0, ySrc1, ySrc2;
            BYTE *ptr_lgp = (BYTE *)lgp;
            BYTE *ptr_ycp = (BYTE *)ycp;
            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr_lgp +  96)), _mm_loadu_si128((__m128i *)(ptr_lgp +  0)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr_lgp + 112)), _mm_loadu_si128((__m128i *)(ptr_lgp + 16)));
            y2 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr_lgp + 128)), _mm_loadu_si128((__m128i *)(ptr_lgp + 32)));
            y3 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr_lgp + 144)), _mm_loadu_si128((__m128i *)(ptr_lgp + 48)));
            y4 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr_lgp + 160)), _mm_loadu_si128((__m128i *)(ptr_lgp + 64)));
            y5 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr_lgp + 176)), _mm_loadu_si128((__m128i *)(ptr_lgp + 80)));

            // 不透明度情報のみ取り出し
            yDp0 = _mm256_and_si256(y0, _mm256_load_si256((__m256i *)MASK_16BIT));
            yDp1 = _mm256_and_si256(y1, _mm256_load_si256((__m256i *)MASK_16BIT));
            yDp2 = _mm256_and_si256(y2, _mm256_load_si256((__m256i *)MASK_16BIT));
            yDp3 = _mm256_and_si256(y3, _mm256_load_si256((__m256i *)MASK_16BIT));
            yDp4 = _mm256_and_si256(y4, _mm256_load_si256((__m256i *)MASK_16BIT));
            yDp5 = _mm256_and_si256(y5, _mm256_load_si256((__m256i *)MASK_16BIT));

            //16bit→32bit
            yDp0 = _mm256_sub_epi32(_mm256_add_epi16(yDp0, _mm256_load_si256((__m256i *)ARRAY_0x8000[1])), _mm256_load_si256((__m256i *)ARRAY_0x8000[1]));
            yDp1 = _mm256_sub_epi32(_mm256_add_epi16(yDp1, _mm256_load_si256((__m256i *)ARRAY_0x8000[1])), _mm256_load_si256((__m256i *)ARRAY_0x8000[1]));
            yDp2 = _mm256_sub_epi32(_mm256_add_epi16(yDp2, _mm256_load_si256((__m256i *)ARRAY_0x8000[1])), _mm256_load_si256((__m256i *)ARRAY_0x8000[1]));
            yDp3 = _mm256_sub_epi32(_mm256_add_epi16(yDp3, _mm256_load_si256((__m256i *)ARRAY_0x8000[1])), _mm256_load_si256((__m256i *)ARRAY_0x8000[1]));
            yDp4 = _mm256_sub_epi32(_mm256_add_epi16(yDp4, _mm256_load_si256((__m256i *)ARRAY_0x8000[1])), _mm256_load_si256((__m256i *)ARRAY_0x8000[1]));
            yDp5 = _mm256_sub_epi32(_mm256_add_epi16(yDp5, _mm256_load_si256((__m256i *)ARRAY_0x8000[1])), _mm256_load_si256((__m256i *)ARRAY_0x8000[1]));

            //lgp->dp_y * logo_depth_mul_fade)/128 /LOGO_FADE_MAX;
            yDp0 = _mm256_srai_epi32(_mm256_mullo_epi32(yDp0, _mm256_set1_epi32(logo_depth_mul_fade)), 15);
            yDp1 = _mm256_srai_epi32(_mm256_mullo_epi32(yDp1, _mm256_set1_epi32(logo_depth_mul_fade)), 15);
            yDp2 = _mm256_srai_epi32(_mm256_mullo_epi32(yDp2, _mm256_set1_epi32(logo_depth_mul_fade)), 15);
            yDp3 = _mm256_srai_epi32(_mm256_mullo_epi32(yDp3, _mm256_set1_epi32(logo_depth_mul_fade)), 15);
            yDp4 = _mm256_srai_epi32(_mm256_mullo_epi32(yDp4, _mm256_set1_epi32(logo_depth_mul_fade)), 15);
            yDp5 = _mm256_srai_epi32(_mm256_mullo_epi32(yDp5, _mm256_set1_epi32(logo_depth_mul_fade)), 15);

            yDp0 = _mm256_packs_epi32(yDp0, yDp1);
            yDp1 = _mm256_packs_epi32(yDp2, yDp3);
            yDp2 = _mm256_packs_epi32(yDp4, yDp5);

            //dp -= (dp==LOGO_MAX_DP)
            //dp = -dp
            yDp0 = _mm256_neg_epi16(_mm256_add_epi16(yDp0, _mm256_cmpeq_epi16(yDp0, _mm256_set1_epi16(LOGO_MAX_DP)))); // -dp
            yDp1 = _mm256_neg_epi16(_mm256_add_epi16(yDp1, _mm256_cmpeq_epi16(yDp1, _mm256_set1_epi16(LOGO_MAX_DP)))); // -dp
            yDp2 = _mm256_neg_epi16(_mm256_add_epi16(yDp2, _mm256_cmpeq_epi16(yDp2, _mm256_set1_epi16(LOGO_MAX_DP)))); // -dp

            //ロゴ色データの取り出し
            y0 = _mm256_packs_epi32(_mm256_srai_epi32(y0, 16), _mm256_srai_epi32(y1, 16)); // lgp->yの抽出
            y1 = _mm256_packs_epi32(_mm256_srai_epi32(y2, 16), _mm256_srai_epi32(y3, 16));
            y2 = _mm256_packs_epi32(_mm256_srai_epi32(y4, 16), _mm256_srai_epi32(y5, 16));

            y0 = _mm256_add_epi16(y0, _mm256_load_si256((__m256i *)(YC48_OFFSET +  0))); //lgp->y + py_offset
            y1 = _mm256_add_epi16(y1, _mm256_load_si256((__m256i *)(YC48_OFFSET + 16))); //lgp->y + py_offset
            y2 = _mm256_add_epi16(y2, _mm256_load_si256((__m256i *)(YC48_OFFSET + 32))); //lgp->y + py_offset

            ySrc0 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr_ycp + 48)), _mm_loadu_si128((__m128i *)(ptr_ycp +  0)));
            ySrc1 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr_ycp + 64)), _mm_loadu_si128((__m128i *)(ptr_ycp + 16)));
            ySrc2 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr_ycp + 80)), _mm_loadu_si128((__m128i *)(ptr_ycp + 32)));

            y5 = _mm256_madd_epi16(_mm256_unpackhi_epi16(ySrc2, y2), _mm256_unpackhi_epi16(_mm256_set1_epi16(LOGO_MAX_DP), yDp2));
            y4 = _mm256_madd_epi16(_mm256_unpacklo_epi16(ySrc2, y2), _mm256_unpacklo_epi16(_mm256_set1_epi16(LOGO_MAX_DP), yDp2));
            y3 = _mm256_madd_epi16(_mm256_unpackhi_epi16(ySrc1, y1), _mm256_unpackhi_epi16(_mm256_set1_epi16(LOGO_MAX_DP), yDp1));
            y2 = _mm256_madd_epi16(_mm256_unpacklo_epi16(ySrc1, y1), _mm256_unpacklo_epi16(_mm256_set1_epi16(LOGO_MAX_DP), yDp1));
            y1 = _mm256_madd_epi16(_mm256_unpackhi_epi16(ySrc0, y0), _mm256_unpackhi_epi16(_mm256_set1_epi16(LOGO_MAX_DP), yDp0)); //xSrc0 * LOGO_MAX_DP + x0 * xDp0(-dp)
            y0 = _mm256_madd_epi16(_mm256_unpacklo_epi16(ySrc0, y0), _mm256_unpacklo_epi16(_mm256_set1_epi16(LOGO_MAX_DP), yDp0)); //xSrc0 * LOGO_MAX_DP + x0 * xDp0(-dp)

            yDp0 = _mm256_adds_epi16(_mm256_set1_epi16(LOGO_MAX_DP), yDp0); // LOGO_MAX_DP + (-dp)
            yDp1 = _mm256_adds_epi16(_mm256_set1_epi16(LOGO_MAX_DP), yDp1); // LOGO_MAX_DP + (-dp)
            yDp2 = _mm256_adds_epi16(_mm256_set1_epi16(LOGO_MAX_DP), yDp2); // LOGO_MAX_DP + (-dp)

            //(ycp->y * LOGO_MAX_DP + yc * (-dp)) / (LOGO_MAX_DP +(-dp));
            y0 = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_cvtepi32_ps(y0), _mm256_rcp_ps_hp(_mm256_cvtepi32_ps(cvtlo256_epi16_epi32(yDp0)))));
            y1 = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_cvtepi32_ps(y1), _mm256_rcp_ps_hp(_mm256_cvtepi32_ps(cvthi256_epi16_epi32(yDp0)))));
            y2 = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_cvtepi32_ps(y2), _mm256_rcp_ps_hp(_mm256_cvtepi32_ps(cvtlo256_epi16_epi32(yDp1)))));
            y3 = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_cvtepi32_ps(y3), _mm256_rcp_ps_hp(_mm256_cvtepi32_ps(cvthi256_epi16_epi32(yDp1)))));
            y4 = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_cvtepi32_ps(y4), _mm256_rcp_ps_hp(_mm256_cvtepi32_ps(cvtlo256_epi16_epi32(yDp2)))));
            y5 = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_cvtepi32_ps(y5), _mm256_rcp_ps_hp(_mm256_cvtepi32_ps(cvthi256_epi16_epi32(yDp2)))));

            y0 = _mm256_packs_epi32(y0, y1);
            y1 = _mm256_packs_epi32(y2, y3);
            y2 = _mm256_packs_epi32(y4, y5);

            y0 = _mm256_max_epi16(_mm256_min_epi16(y0, _mm256_load_si256((__m256i *)(YC48_MAX +  0))), _mm256_load_si256((__m256i *)(YC48_MIN +  0)));
            y1 = _mm256_max_epi16(_mm256_min_epi16(y1, _mm256_load_si256((__m256i *)(YC48_MAX + 16))), _mm256_load_si256((__m256i *)(YC48_MIN + 16)));
            y2 = _mm256_max_epi16(_mm256_min_epi16(y2, _mm256_load_si256((__m256i *)(YC48_MAX + 32))), _mm256_load_si256((__m256i *)(YC48_MIN + 32)));

            //多重計算にならないよう、一度計算したところは計算しないでおく
            const USHORT *ptr_yc48_mask = YC48_MASK + step * 3;
            y0 = _mm256_blendv_epi8(ySrc0, y0, _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr_yc48_mask + 24)), _mm_loadu_si128((__m128i *)(ptr_yc48_mask +  0))));
            y1 = _mm256_blendv_epi8(ySrc1, y1, _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr_yc48_mask + 32)), _mm_loadu_si128((__m128i *)(ptr_yc48_mask +  8))));
            y2 = _mm256_blendv_epi8(ySrc2, y2, _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr_yc48_mask + 40)), _mm_loadu_si128((__m128i *)(ptr_yc48_mask + 16))));

            //permute
            _mm256_storeu_si256((__m256i *)(ptr_ycp +  0), _mm256_permute2x128_si256(y0, y1, (0x02<<4)+0x00)); // 128,   0
            _mm256_storeu_si256((__m256i *)(ptr_ycp + 32), _mm256_blend_epi32(       y0, y2, (0x00<<4)+0x0f)); // 384, 256
            _mm256_storeu_si256((__m256i *)(ptr_ycp + 64), _mm256_permute2x128_si256(y1, y2, (0x03<<4)+0x01)); // 768, 512

            step = limit_1_to_16(x);
        }
#elif USE_SSE2
        for (int x = x_fin - 8, step = 8; x >= 0; x -= step, lgp += step, ycp += step) {
            __m128i x0, x1, x2, x3, x4, x5, xDp0, xDp1, xDp2, xDp3, xDp4, xDp5, xSrc0, xSrc1, xSrc2;
            BYTE *ptr_lgp = (BYTE *)lgp;
            BYTE *ptr_ycp = (BYTE *)ycp;
            x0 = _mm_loadu_si128((__m128i *)(ptr_lgp +  0));
            x1 = _mm_loadu_si128((__m128i *)(ptr_lgp + 16));
            x2 = _mm_loadu_si128((__m128i *)(ptr_lgp + 32));
            x3 = _mm_loadu_si128((__m128i *)(ptr_lgp + 48));
            x4 = _mm_loadu_si128((__m128i *)(ptr_lgp + 64));
            x5 = _mm_loadu_si128((__m128i *)(ptr_lgp + 80));

            // 不透明度情報のみ取り出し
            xDp0 = _mm_and_si128(x0, _mm_load_si128((__m128i *)MASK_16BIT));
            xDp1 = _mm_and_si128(x1, _mm_load_si128((__m128i *)MASK_16BIT));
            xDp2 = _mm_and_si128(x2, _mm_load_si128((__m128i *)MASK_16BIT));
            xDp3 = _mm_and_si128(x3, _mm_load_si128((__m128i *)MASK_16BIT));
            xDp4 = _mm_and_si128(x4, _mm_load_si128((__m128i *)MASK_16BIT));
            xDp5 = _mm_and_si128(x5, _mm_load_si128((__m128i *)MASK_16BIT));

            //16bit→32bit
            xDp0 = _mm_sub_epi32(_mm_add_epi16(xDp0, _mm_load_si128((__m128i *)ARRAY_0x8000[1])), _mm_load_si128((__m128i *)ARRAY_0x8000[1]));
            xDp1 = _mm_sub_epi32(_mm_add_epi16(xDp1, _mm_load_si128((__m128i *)ARRAY_0x8000[1])), _mm_load_si128((__m128i *)ARRAY_0x8000[1]));
            xDp2 = _mm_sub_epi32(_mm_add_epi16(xDp2, _mm_load_si128((__m128i *)ARRAY_0x8000[1])), _mm_load_si128((__m128i *)ARRAY_0x8000[1]));
            xDp3 = _mm_sub_epi32(_mm_add_epi16(xDp3, _mm_load_si128((__m128i *)ARRAY_0x8000[1])), _mm_load_si128((__m128i *)ARRAY_0x8000[1]));
            xDp4 = _mm_sub_epi32(_mm_add_epi16(xDp4, _mm_load_si128((__m128i *)ARRAY_0x8000[1])), _mm_load_si128((__m128i *)ARRAY_0x8000[1]));
            xDp5 = _mm_sub_epi32(_mm_add_epi16(xDp5, _mm_load_si128((__m128i *)ARRAY_0x8000[1])), _mm_load_si128((__m128i *)ARRAY_0x8000[1]));

            //lgp->dp_y * logo_depth_mul_fade)/128 /LOGO_FADE_MAX;
            xDp0 = _mm_srai_epi32(_mm_mullo_epi32_simd(xDp0, _mm_set1_epi32(logo_depth_mul_fade)), 15);
            xDp1 = _mm_srai_epi32(_mm_mullo_epi32_simd(xDp1, _mm_set1_epi32(logo_depth_mul_fade)), 15);
            xDp2 = _mm_srai_epi32(_mm_mullo_epi32_simd(xDp2, _mm_set1_epi32(logo_depth_mul_fade)), 15);
            xDp3 = _mm_srai_epi32(_mm_mullo_epi32_simd(xDp3, _mm_set1_epi32(logo_depth_mul_fade)), 15);
            xDp4 = _mm_srai_epi32(_mm_mullo_epi32_simd(xDp4, _mm_set1_epi32(logo_depth_mul_fade)), 15);
            xDp5 = _mm_srai_epi32(_mm_mullo_epi32_simd(xDp5, _mm_set1_epi32(logo_depth_mul_fade)), 15);

            xDp0 = _mm_packs_epi32(xDp0, xDp1);
            xDp1 = _mm_packs_epi32(xDp2, xDp3);
            xDp2 = _mm_packs_epi32(xDp4, xDp5);

            //ロゴ色データの取り出し
            x0   = _mm_packs_epi32(_mm_srai_epi32(x0, 16), _mm_srai_epi32(x1, 16));
            x1   = _mm_packs_epi32(_mm_srai_epi32(x2, 16), _mm_srai_epi32(x3, 16));
            x2   = _mm_packs_epi32(_mm_srai_epi32(x4, 16), _mm_srai_epi32(x5, 16));

            x0   = _mm_add_epi16(x0, _mm_load_si128((__m128i *)(YC48_OFFSET +  0))); //lgp->y + py_offset
            x1   = _mm_add_epi16(x1, _mm_load_si128((__m128i *)(YC48_OFFSET +  8))); //lgp->y + py_offset
            x2   = _mm_add_epi16(x2, _mm_load_si128((__m128i *)(YC48_OFFSET + 16))); //lgp->y + py_offset

            //dp -= (dp==LOGO_MAX_DP)
            //dp = -dp
            xDp0 = _mm_neg_epi16(_mm_add_epi16(xDp0, _mm_cmpeq_epi16(xDp0, _mm_set1_epi16(LOGO_MAX_DP)))); // -dp
            xDp1 = _mm_neg_epi16(_mm_add_epi16(xDp1, _mm_cmpeq_epi16(xDp1, _mm_set1_epi16(LOGO_MAX_DP)))); // -dp
            xDp2 = _mm_neg_epi16(_mm_add_epi16(xDp2, _mm_cmpeq_epi16(xDp2, _mm_set1_epi16(LOGO_MAX_DP)))); // -dp

            xSrc0 = _mm_loadu_si128((__m128i *)(ptr_ycp +  0));
            xSrc1 = _mm_loadu_si128((__m128i *)(ptr_ycp + 16));
            xSrc2 = _mm_loadu_si128((__m128i *)(ptr_ycp + 32));

            x5 = _mm_madd_epi16(_mm_unpackhi_epi16(xSrc2, x2), _mm_unpackhi_epi16(_mm_set1_epi16(LOGO_MAX_DP), xDp2));
            x4 = _mm_madd_epi16(_mm_unpacklo_epi16(xSrc2, x2), _mm_unpacklo_epi16(_mm_set1_epi16(LOGO_MAX_DP), xDp2));
            x3 = _mm_madd_epi16(_mm_unpackhi_epi16(xSrc1, x1), _mm_unpackhi_epi16(_mm_set1_epi16(LOGO_MAX_DP), xDp1));
            x2 = _mm_madd_epi16(_mm_unpacklo_epi16(xSrc1, x1), _mm_unpacklo_epi16(_mm_set1_epi16(LOGO_MAX_DP), xDp1));
            x1 = _mm_madd_epi16(_mm_unpackhi_epi16(xSrc0, x0), _mm_unpackhi_epi16(_mm_set1_epi16(LOGO_MAX_DP), xDp0)); //xSrc0 * LOGO_MAX_DP + x0 * xDp0(-dp)
            x0 = _mm_madd_epi16(_mm_unpacklo_epi16(xSrc0, x0), _mm_unpacklo_epi16(_mm_set1_epi16(LOGO_MAX_DP), xDp0)); //xSrc0 * LOGO_MAX_DP + x0 * xDp0(-dp)

            xDp0 = _mm_adds_epi16(_mm_set1_epi16(LOGO_MAX_DP), xDp0); // LOGO_MAX_DP + (-dp)
            xDp1 = _mm_adds_epi16(_mm_set1_epi16(LOGO_MAX_DP), xDp1); // LOGO_MAX_DP + (-dp)
            xDp2 = _mm_adds_epi16(_mm_set1_epi16(LOGO_MAX_DP), xDp2); // LOGO_MAX_DP + (-dp)

            //(ycp->y * LOGO_MAX_DP + yc * (-dp)) / (LOGO_MAX_DP +(-dp));
            x0 = _mm_cvtps_epi32(_mm_mul_ps(_mm_cvtepi32_ps(x0), _mm_rcp_ps_hp(_mm_cvtepi32_ps(cvtlo_epi16_epi32(xDp0)))));
            x1 = _mm_cvtps_epi32(_mm_mul_ps(_mm_cvtepi32_ps(x1), _mm_rcp_ps_hp(_mm_cvtepi32_ps(cvthi_epi16_epi32(xDp0)))));
            x2 = _mm_cvtps_epi32(_mm_mul_ps(_mm_cvtepi32_ps(x2), _mm_rcp_ps_hp(_mm_cvtepi32_ps(cvtlo_epi16_epi32(xDp1)))));
            x3 = _mm_cvtps_epi32(_mm_mul_ps(_mm_cvtepi32_ps(x3), _mm_rcp_ps_hp(_mm_cvtepi32_ps(cvthi_epi16_epi32(xDp1)))));
            x4 = _mm_cvtps_epi32(_mm_mul_ps(_mm_cvtepi32_ps(x4), _mm_rcp_ps_hp(_mm_cvtepi32_ps(cvtlo_epi16_epi32(xDp2)))));
            x5 = _mm_cvtps_epi32(_mm_mul_ps(_mm_cvtepi32_ps(x5), _mm_rcp_ps_hp(_mm_cvtepi32_ps(cvthi_epi16_epi32(xDp2)))));

            x0 = _mm_packs_epi32(x0, x1);
            x1 = _mm_packs_epi32(x2, x3);
            x2 = _mm_packs_epi32(x4, x5);

            x0 = _mm_max_epi16(_mm_min_epi16(x0, _mm_load_si128((__m128i *)(YC48_MAX +  0))), _mm_load_si128((__m128i *)(YC48_MIN +  0)));
            x1 = _mm_max_epi16(_mm_min_epi16(x1, _mm_load_si128((__m128i *)(YC48_MAX +  8))), _mm_load_si128((__m128i *)(YC48_MIN +  8)));
            x2 = _mm_max_epi16(_mm_min_epi16(x2, _mm_load_si128((__m128i *)(YC48_MAX + 16))), _mm_load_si128((__m128i *)(YC48_MIN + 16)));

            //多重計算にならないよう、一度計算したところは計算しないでおく
            const USHORT *ptr_yc48_mask = YC48_MASK + step * 3;
            x0 = blendv_epi8_simd(xSrc0, x0, _mm_loadu_si128((__m128i *)(ptr_yc48_mask +  0)));
            x1 = blendv_epi8_simd(xSrc1, x1, _mm_loadu_si128((__m128i *)(ptr_yc48_mask +  8)));
            x2 = blendv_epi8_simd(xSrc2, x2, _mm_loadu_si128((__m128i *)(ptr_yc48_mask + 16)));

            _mm_storeu_si128((__m128i *)(ptr_ycp +  0), x0);
            _mm_storeu_si128((__m128i *)(ptr_ycp + 16), x1);
            _mm_storeu_si128((__m128i *)(ptr_ycp + 32), x2);

            step = limit_1_to_8(x);
        }
#else
        for (int x = 0; x < x_fin; x++, lgp++, ycp++) {
            int dp, yc;
            // 輝度
            dp = (lgp->dp_y * logo_depth_mul_fade +64)/128 /LOGO_FADE_MAX;
            if(dp){
                dp -= (dp==LOGO_MAX_DP); // 0での除算回避
                yc = lgp->y + py_offset;
                yc = (ycp->y*LOGO_MAX_DP - yc*dp +(LOGO_MAX_DP-dp)/2) /(LOGO_MAX_DP - dp);	// 逆算
                ycp->y = (short)Clamp(yc, Y_MIN, Y_MAX);
            }

            // 色差(青)
            dp = (lgp->dp_cb * logo_depth_mul_fade +64)/128 /LOGO_FADE_MAX;
            if(dp){
                dp -= (dp==LOGO_MAX_DP); // 0での除算回避
                yc = lgp->cb + cb_offset;
                yc = (ycp->cb*LOGO_MAX_DP - yc*dp +(LOGO_MAX_DP-dp)/2) /(LOGO_MAX_DP - dp);
                ycp->cb = (short)Clamp(yc, C_MIN, C_MAX);
            }

            // 色差(赤)
            dp = (lgp->dp_cr * logo_depth_mul_fade +64)/128 /LOGO_FADE_MAX;
            if(dp){
                dp -= (dp==LOGO_MAX_DP); // 0での除算回避
                yc = lgp->cr + cr_offset;
                yc = (ycp->cr*LOGO_MAX_DP - yc*dp +(LOGO_MAX_DP-dp)/2) /(LOGO_MAX_DP - dp);
                ycp->cr = (short)Clamp(yc, C_MIN, C_MAX);
            }
        }
#endif
    }
#if USE_AVX2
    _mm256_zeroupper();
#endif
    return TRUE;
}


#if USE_AVX2
static __forceinline __m256i get_logo_y(const short *ptr_lgp, int logo_width, int x_offset, int y_offset) {
    ptr_lgp += x_offset;
    ptr_lgp += y_offset * logo_width;
    return _mm256_loadu_si256((__m256i *)ptr_lgp);
}
#elif USE_SSE2
static __forceinline __m128i get_logo_y(const short *ptr_lgp, int logo_width, int x_offset, int y_offset) {
    ptr_lgp += x_offset;
    ptr_lgp += y_offset * logo_width;
    return _mm_loadu_si128((__m128i *)ptr_lgp);
}
#endif

#if USE_AVX2
static __forceinline void get_logo_y(__m256i& yY0, __m256i& yY1, __m256i& yDpY0, __m256i& yDpY1, const BYTE *ptr) {
    __m256i yA0, yA1, yA2, yA3, yA4, yA5;
    __m256i y0, y1, y2, y3, y4, y5;
    yA0 = _mm256_loadu_si256((__m256i *)(ptr +   0));
    yA1 = _mm256_loadu_si256((__m256i *)(ptr +  32));
    yA2 = _mm256_loadu_si256((__m256i *)(ptr +  64));
    yA3 = _mm256_loadu_si256((__m256i *)(ptr +  96));
    yA4 = _mm256_loadu_si256((__m256i *)(ptr + 128));
    yA5 = _mm256_loadu_si256((__m256i *)(ptr + 160));

    y0 = _mm256_blend_epi32(yA0, yA1, 0xf0);                    // 384, 0
    y1 = _mm256_permute2x128_si256(yA0, yA2, (0x02<<4) + 0x01); // 512, 128
    y2 = _mm256_blend_epi32(yA1, yA2, 0xf0);                    // 640, 256

    y3 = _mm256_blend_epi32(yA3, yA4, 0xf0);                    // 384, 0
    y4 = _mm256_permute2x128_si256(yA3, yA5, (0x02<<4) + 0x01); // 512, 128
    y5 = _mm256_blend_epi32(yA4, yA5, 0xf0);                    // 640, 256

    y0 = _mm256_blend_epi32(y0, y1, 0x44);
    y3 = _mm256_blend_epi32(y3, y4, 0x44);
    y0 = _mm256_blend_epi32(y0, y2, 0x22);
    y3 = _mm256_blend_epi32(y3, y5, 0x22);

    y0 = _mm256_shuffle_epi32(y0, _MM_SHUFFLE(1,2,3,0));
    y3 = _mm256_shuffle_epi32(y3, _MM_SHUFFLE(1,2,3,0));

    yY0 = _mm256_srai_epi32(y0, 16);
    yY1 = _mm256_srai_epi32(y3, 16);

    yDpY0 = _mm256_add_epi16(y0, _mm256_load_si256((__m256i *)ARRAY_0x8000[1]));
    yDpY1 = _mm256_add_epi16(y3, _mm256_load_si256((__m256i *)ARRAY_0x8000[1]));
    yDpY0 = _mm256_and_si256(yDpY0, _mm256_load_si256((__m256i *)MASK_16BIT));
    yDpY1 = _mm256_and_si256(yDpY1, _mm256_load_si256((__m256i *)MASK_16BIT));
    yDpY0 = _mm256_sub_epi32(yDpY0, _mm256_load_si256((__m256i *)ARRAY_0x8000[1]));
    yDpY1 = _mm256_sub_epi32(yDpY1, _mm256_load_si256((__m256i *)ARRAY_0x8000[1]));
}

static __forceinline __m256i get_logo_y(const LOGO_PIXEL *ptr_lgp, int logo_width, int x_offset, int y_offset) {
    ptr_lgp += x_offset;
    ptr_lgp += y_offset * logo_width;
    BYTE *ptr = (BYTE *)ptr_lgp;
    __m256i yY0, yY1, yDpY0, yDpY1;
    get_logo_y(yY0, yY1, yDpY0, yDpY1, ptr);

    yY0 = _mm256_packs_epi32(yY0, yY1);
    yDpY0 = _mm256_packs_epi32(yDpY0, yDpY1);
    //    yY * yDpY / 256
    // =  yY  * yDpY * 256 / 65536
    // =  (yY * 2)  * (yDpY * 128) / 65536
    // = ((yY << 1) * (xDpy << 7)) >> 16
    __m256i yResult = _mm256_mulhi_epi16(_mm256_slli_epi16(yY0, 1), _mm256_slli_epi16(yDpY0, 7));
    return _mm256_permute4x64_epi64(yResult, _MM_SHUFFLE(3, 1, 2, 0));
}
#elif USE_SSE2
static __forceinline void get_logo_y(__m128i& xY0, __m128i& xY1, __m128i& xDpY0, __m128i& xDpY1, const BYTE *ptr) {
    __m128i x0, x1, x2, x3, x4, x5;
    x0 = _mm_loadu_si128((__m128i *)(ptr +  0));
    x1 = _mm_loadu_si128((__m128i *)(ptr + 16));
    x2 = _mm_loadu_si128((__m128i *)(ptr + 32));
    x3 = _mm_loadu_si128((__m128i *)(ptr + 48));
    x4 = _mm_loadu_si128((__m128i *)(ptr + 64));
    x5 = _mm_loadu_si128((__m128i *)(ptr + 80));
#if USE_SSE41
    x0 = _mm_blend_epi16(x0, x1, 0x20+0x10);
    x3 = _mm_blend_epi16(x3, x4, 0x20+0x10);
    x0 = _mm_blend_epi16(x0, x2, 0x08+0x04);
    x3 = _mm_blend_epi16(x3, x5, 0x08+0x04);
#else
    static const __declspec(align(16)) DWORD Y_EXTRACT_MASK[] = { 0x00000000, 0x00000000, 0xffffffff, 0x00000000 };
    __m128i xMask = _mm_load_si128((__m128i *)Y_EXTRACT_MASK);
    x0 = blendv_epi8_simd(x0, x1, xMask);
    x3 = blendv_epi8_simd(x3, x4, xMask);
    xMask = _mm_srli_si128(xMask, 4);
    x0 = blendv_epi8_simd(x0, x2, xMask);
    x3 = blendv_epi8_simd(x3, x5, xMask);
#endif
    x0 = _mm_shuffle_epi32(x0, _MM_SHUFFLE(1,2,3,0));
    x3 = _mm_shuffle_epi32(x3, _MM_SHUFFLE(1,2,3,0));

    xY0 = _mm_srai_epi32(x0, 16);
    xY1 = _mm_srai_epi32(x3, 16);

    xDpY0 = _mm_add_epi16(x0, _mm_load_si128((__m128i *)ARRAY_0x8000[1]));
    xDpY1 = _mm_add_epi16(x3, _mm_load_si128((__m128i *)ARRAY_0x8000[1]));
    xDpY0 = _mm_and_si128(xDpY0, _mm_load_si128((__m128i *)MASK_16BIT));
    xDpY1 = _mm_and_si128(xDpY1, _mm_load_si128((__m128i *)MASK_16BIT));
    xDpY0 = _mm_sub_epi32(xDpY0, _mm_load_si128((__m128i *)ARRAY_0x8000[1]));
    xDpY1 = _mm_sub_epi32(xDpY1, _mm_load_si128((__m128i *)ARRAY_0x8000[1]));
}

static __forceinline __m128i get_logo_y(const LOGO_PIXEL *ptr_lgp, int logo_width, int x_offset, int y_offset) {
    ptr_lgp += x_offset;
    ptr_lgp += y_offset * logo_width;
    BYTE *ptr = (BYTE *)ptr_lgp;
    __m128i xY0, xY1, xDpY0, xDpY1;
    get_logo_y(xY0, xY1, xDpY0, xDpY1, ptr);

    xY0 = _mm_packs_epi32(xY0, xY1);
    xDpY0 = _mm_packs_epi32(xDpY0, xDpY1);

    //    xY * xDpY / 256
    // =  xY  * xDpY * 256 / 65536
    // =  (xY * 2)  * (xDpY * 128) / 65536
    // = ((xY << 1) * (xDpy << 7)) >> 16
    return _mm_mulhi_epi16(_mm_slli_epi16(xY0, 1), _mm_slli_epi16(xDpY0, 7));
}
#endif

static __forceinline void func_proc_eraze_logo_y_simd(short *buffer_dst, const short *buffer_src, FILTER *fp, const LOGO_HEADER *lgh, int fade) {
    const int logo_width = lgh->w;
    const int logo_pitch = PITCH(logo_width);

    // LOGO_PIXELデータへのポインタ
    LOGO_PIXEL *lgp_line = (LOGO_PIXEL *)(lgh + 1);

    // 左上の位置へ移動
    const short *src_line = buffer_src;
    short *dst_line = buffer_dst;
    const int y_fin = lgh->h;
    const int x_fin = logo_pitch;

//	static const int Y_MIN = -128, Y_MAX = 4096+128;
    static const int Y_MIN = -2048, Y_MAX = 4096+2048;	// (2015/08/11:+h37)
#if USE_SSE2
    const int logo_depth_mul_fade = fp->track[2] * fade;
    const short py_offset = (short)(fp->track[3] << 4);
#if USE_AVX2
    const __m256i yPY_OFFSET = _mm256_set1_epi16(py_offset);
#elif USE_SSE2
    const __m128i xPY_OFFSET = _mm_set1_epi16(py_offset);
#endif
#endif

    for (int y = 0; y < y_fin; y++, src_line += logo_pitch, dst_line += logo_pitch, lgp_line += logo_width) {
        const short *src = src_line;
        short *dst = dst_line;
        LOGO_PIXEL *lgp = lgp_line;
#if USE_AVX2
        for (int x = x_fin - 16, step = 16; x >= 0; x -= step, lgp += step, src += step, dst += step) {
            __m256i y0, y1, yDp0, yDp1, ySrc0, ySrc1;
            get_logo_y(y0, y1, yDp0, yDp1, (BYTE *)lgp);

            //lgp->dp_y * logo_depth_mul_fade)/128 /LOGO_FADE_MAX;
            yDp0 = _mm256_srai_epi32(_mm256_mullo_epi32(yDp0, _mm256_set1_epi32(logo_depth_mul_fade)), 15);
            yDp1 = _mm256_srai_epi32(_mm256_mullo_epi32(yDp1, _mm256_set1_epi32(logo_depth_mul_fade)), 15);

            yDp0 = _mm256_packs_epi32(yDp0, yDp1); //3,1,2,0

            //dp -= (dp==LOGO_MAX_DP)
            //dp = -dp
            yDp0 = _mm256_neg_epi16(_mm256_add_epi16(yDp0, _mm256_cmpeq_epi16(yDp0, _mm256_set1_epi16(LOGO_MAX_DP)))); // -dp

            //ロゴ色データの取り出し
            y0 = _mm256_packs_epi32(y0, y1); // lgp->yの抽出 //3,1,2,0

            y0 = _mm256_add_epi16(y0, yPY_OFFSET); //lgp->y + py_offset

            ySrc0 = _mm256_loadu_si256((__m256i *)src);
            ySrc1 = _mm256_permute4x64_epi64(ySrc0, _MM_SHUFFLE(3, 1, 2, 0));

            y1 = _mm256_madd_epi16(_mm256_unpackhi_epi16(ySrc1, y0), _mm256_unpackhi_epi16(_mm256_set1_epi16(LOGO_MAX_DP), yDp0)); //xSrc0 * LOGO_MAX_DP + x0 * xDp0(-dp) //1, 0
            y0 = _mm256_madd_epi16(_mm256_unpacklo_epi16(ySrc1, y0), _mm256_unpacklo_epi16(_mm256_set1_epi16(LOGO_MAX_DP), yDp0)); //xSrc0 * LOGO_MAX_DP + x0 * xDp0(-dp) //3, 2

            yDp0 = _mm256_adds_epi16(_mm256_set1_epi16(LOGO_MAX_DP), yDp0); // LOGO_MAX_DP + (-dp)

            //(ycp->y * LOGO_MAX_DP + yc * (-dp)) / (LOGO_MAX_DP +(-dp));
            y0 = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_cvtepi32_ps(y0), _mm256_rcp_ps_hp(_mm256_cvtepi32_ps(cvtlo256_epi16_epi32(yDp0))))); //1, 0
            y1 = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_cvtepi32_ps(y1), _mm256_rcp_ps_hp(_mm256_cvtepi32_ps(cvthi256_epi16_epi32(yDp0))))); //3, 2

            y0 = _mm256_packs_epi32(y0, y1); //3, 1, 2, 0

            y0 = _mm256_max_epi16(_mm256_min_epi16(y0, _mm256_set1_epi16(Y_MAX)), _mm256_set1_epi16(Y_MIN));

            y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3, 1, 2, 0)); //3,1,2,0 -> 3,2,1,0

            //多重計算にならないよう、一度計算したところは計算しないでおく
            y0 = _mm256_blendv_epi8(ySrc0, y0, _mm256_loadu_si256((__m256i *)(YC48_MASK + 32 + step)));

            _mm256_storeu_si256((__m256i *)dst, y0);
        }
#elif USE_SSE2
        for (int x = x_fin - 8, step = 8; x >= 0; x -= step, lgp += step, src += step, dst += step) {
            __m128i x0, x1, xDp0, xDp1, xSrc0;
            get_logo_y(x0, x1, xDp0, xDp1, (BYTE *)lgp);

            //lgp->dp_y * logo_depth_mul_fade)/128 /LOGO_FADE_MAX;
            xDp0 = _mm_srai_epi32(_mm_mullo_epi32_simd(xDp0, _mm_set1_epi32(logo_depth_mul_fade)), 15);
            xDp1 = _mm_srai_epi32(_mm_mullo_epi32_simd(xDp1, _mm_set1_epi32(logo_depth_mul_fade)), 15);

            xDp0 = _mm_packs_epi32(xDp0, xDp1);

            //ロゴ色データの取り出し
            x0   = _mm_packs_epi32(x0, x1);

            x0   = _mm_add_epi16(x0, xPY_OFFSET); //lgp->y + py_offset

            //dp -= (dp==LOGO_MAX_DP)
            //dp = -dp
            xDp0 = _mm_neg_epi16(_mm_add_epi16(xDp0, _mm_cmpeq_epi16(xDp0, _mm_set1_epi16(LOGO_MAX_DP)))); // -dp

            xSrc0 = _mm_loadu_si128((__m128i *)src);

            x1 = _mm_madd_epi16(_mm_unpackhi_epi16(xSrc0, x0), _mm_unpackhi_epi16(_mm_set1_epi16(LOGO_MAX_DP), xDp0)); //xSrc0 * LOGO_MAX_DP + x0 * xDp0(-dp)
            x0 = _mm_madd_epi16(_mm_unpacklo_epi16(xSrc0, x0), _mm_unpacklo_epi16(_mm_set1_epi16(LOGO_MAX_DP), xDp0)); //xSrc0 * LOGO_MAX_DP + x0 * xDp0(-dp)

            xDp0 = _mm_adds_epi16(_mm_set1_epi16(LOGO_MAX_DP), xDp0); // LOGO_MAX_DP + (-dp)

            //(ycp->y * LOGO_MAX_DP + yc * (-dp)) / (LOGO_MAX_DP +(-dp));
            x0 = _mm_cvtps_epi32(_mm_mul_ps(_mm_cvtepi32_ps(x0), _mm_rcp_ps_hp(_mm_cvtepi32_ps(cvtlo_epi16_epi32(xDp0)))));
            x1 = _mm_cvtps_epi32(_mm_mul_ps(_mm_cvtepi32_ps(x1), _mm_rcp_ps_hp(_mm_cvtepi32_ps(cvthi_epi16_epi32(xDp0)))));

            x0 = _mm_packs_epi32(x0, x1);

            x0 = _mm_max_epi16(_mm_min_epi16(x0, _mm_set1_epi16(Y_MAX)), _mm_set1_epi16(Y_MIN));

            //多重計算にならないよう、一度計算したところは計算しないでおく
            x0 = blendv_epi8_simd(xSrc0, x0, _mm_loadu_si128((__m128i *)(YC48_MASK + 16 + step)));

            _mm_storeu_si128((__m128i *)dst, x0);
        }
#else
        for (int x = 0; x < x_fin; x++, lgp++, dst++, src++) {
            int dp = (lgp->dp_y * fp->track[2] * fade +64)/128 /LOGO_FADE_MAX;
            if (dp) {
                if (dp==LOGO_MAX_DP) dp--;	// 0での除算回避
                int yc = lgp->y + fp->track[3] * 16;
                yc = ((*src) * LOGO_MAX_DP - yc * dp + (LOGO_MAX_DP - dp) / 2) /(LOGO_MAX_DP - dp);	// 逆算
//				(*dst) = (short)Clamp(yc,-128,4096+128);
                (*dst) = (short)Clamp(yc,Y_MIN,Y_MAX);	// (2015/08/11:+h37)
            } else {
                *dst = *src;
            }
        }
#endif
    }
#if USE_AVX2
    _mm256_zeroupper();
#endif
}

#if USE_AVX2
static __forceinline void prewitt_filter_simd(__m256i& yResultH, __m256i& yResultV,  const __m256i& yY0High, const __m256i& yY1High, const __m256i& yY0Middle, const __m256i& yY1Middle, const __m256i& yY0Lower, const __m256i& yY1Lower) {
    __m256i yResultV0, yResultV1, yResultH0, yResultH1;
    //Horizontal
    yResultH0 = _mm256_adds_epi16(yY0High, _mm256_adds_epi16(yY0Middle, yY0Lower));
    yResultH1 = _mm256_alignr256_epi8(yY1High, yY0High, 4);
    yResultH1 = _mm256_adds_epi16(yResultH1, _mm256_alignr256_epi8(yY1Middle, yY0Middle, 4));
    yResultH1 = _mm256_adds_epi16(yResultH1, _mm256_alignr256_epi8(yY1Lower,  yY0Lower,  4));

    //Vertical
    yResultV0 = yY0High;
    yResultV0 = _mm256_adds_epi16(yResultV0, _mm256_alignr256_epi8(yY1High, yY0High, 2));
    yResultV0 = _mm256_adds_epi16(yResultV0, _mm256_alignr256_epi8(yY1High, yY0High, 4));
    yResultV1 = yY0Lower;
    yResultV1 = _mm256_adds_epi16(yResultV1, _mm256_alignr256_epi8(yY1Lower, yY0Lower, 2));
    yResultV1 = _mm256_adds_epi16(yResultV1, _mm256_alignr256_epi8(yY1Lower, yY0Lower, 4));

    yResultH = _mm256_subs_epi16(yResultH1, yResultH0);
    yResultV = _mm256_subs_epi16(yResultV1, yResultV0);
}

static __forceinline __m256i prewitt_filter_simd(const __m256i& yY0High, const __m256i& yY1High, const __m256i& yY0Middle, const __m256i& yY1Middle, const __m256i& yY0Lower, const __m256i& yY1Lower) {
    __m256i yResultH, yResultV;
    prewitt_filter_simd(yResultH, yResultV, yY0High, yY1High, yY0Middle, yY1Middle, yY0Lower, yY1Lower);
    return _mm256_alpha_max_plus_beta_min_epi16(yResultH, yResultV);
}

static __forceinline __m256i prewitt_filter_simd_with_weight(__m256i ySrc, const __m256i& yY0High, const __m256i& yY1High, const __m256i& yY0Middle, const __m256i& yY1Middle, const __m256i& yY0Lower, const __m256i& yY1Lower) {
    __m256i yResultH, yResultV;
    prewitt_filter_simd(yResultH, yResultV, yY0High, yY1High, yY0Middle, yY1Middle, yY0Lower, yY1Lower);

    __m256 y0, y1, y2, y3, yWeight0, yWeight1;
    y0 = _mm256_cvtepi32_ps(cvtlo256_epi16_epi32(yResultH));
    y1 = _mm256_cvtepi32_ps(cvthi256_epi16_epi32(yResultH));
    y2 = _mm256_cvtepi32_ps(cvtlo256_epi16_epi32(yResultV));
    y3 = _mm256_cvtepi32_ps(cvthi256_epi16_epi32(yResultV));
    ySrc = _mm256_min_epi16(ySrc, _mm256_set1_epi16(4096));
    yWeight0 = _mm256_cvtepi32_ps(cvtlo256_epi16_epi32(ySrc));
    yWeight1 = _mm256_cvtepi32_ps(cvthi256_epi16_epi32(ySrc));
#if USE_FMA3
    yWeight0 = _mm256_fmadd_ps(yWeight0, _mm256_set1_ps(-1 * 4.0f / (2200.0f * 9.0f)), _mm256_set1_ps(1.0f));
    yWeight1 = _mm256_fmadd_ps(yWeight1, _mm256_set1_ps(-1 * 4.0f / (2200.0f * 9.0f)), _mm256_set1_ps(1.0f));
    y0 = _mm256_sqrt_ps(_mm256_fmadd_ps(y0, y0, _mm256_mul_ps(y2, y2)));
    y1 = _mm256_sqrt_ps(_mm256_fmadd_ps(y1, y1, _mm256_mul_ps(y3, y3)));
#else
    yWeight0 = _mm256_add_ps(_mm256_mul_ps(yWeight0, _mm256_set1_ps(-1 * 4.0f / (2200.0f * 9.0f))), _mm256_set1_ps(1.0f));
    yWeight1 = _mm256_add_ps(_mm256_mul_ps(yWeight1, _mm256_set1_ps(-1 * 4.0f / (2200.0f * 9.0f))), _mm256_set1_ps(1.0f));
    y0 = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(y0, y0), _mm256_mul_ps(y2, y2)));
    y1 = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(y1, y1), _mm256_mul_ps(y3, y3)));
#endif
    yWeight0 = _mm256_rcp_ps_hp(yWeight0);
    yWeight1 = _mm256_rcp_ps_hp(yWeight1);
    y0 = _mm256_mul_ps(y0, yWeight0);
    y1 = _mm256_mul_ps(y1, yWeight1);

    return _mm256_packs_epi32(_mm256_cvtps_epi32(y0), _mm256_cvtps_epi32(y1));
}

static __forceinline __m256i prewitt_filter_5x5_add_simd(__m256i y0, __m256i y1) {
    __m256i ySumHi, ySumLo;
    ySumLo = cvtlo256_epi16_epi32(y0);
    ySumHi = cvthi256_epi16_epi32(y0);
    ySumLo = _mm256_add_epi32(ySumLo, cvtlo256_epi16_epi32(y1));
    ySumHi = _mm256_add_epi32(ySumHi, cvthi256_epi16_epi32(y1));
    ySumLo = _mm256_srai_epi32(ySumLo, 2);
    ySumHi = _mm256_srai_epi32(ySumHi, 2);
    return _mm256_packs_epi32(ySumLo, ySumHi);
}

static __forceinline __m256i prewitt_filter_5x5_simd(
    const __m256i& yY0_0, const __m256i& yY0_1,
    const __m256i& yY1_0, const __m256i& yY1_1,
    const __m256i& yY2_0, const __m256i& yY2_1,
    const __m256i& yY3_0, const __m256i& yY3_1,
    const __m256i& yY4_0, const __m256i& yY4_1) {
    __m256i yResultV0, yResultV1, yResultV2, yResultV3, yResultH0, yResultH1, yResultH2, yResultH3;
    //Horizontal
    yResultH0 = yY0_0;
    yResultH0 = _mm256_adds_epi16(yResultH0, _mm256_adds_epi16(_mm256_adds_epi16(yY1_0, yY2_0), _mm256_adds_epi16(yY3_0, yY4_0)));
    yResultH0 = _mm256_adds_epi16(yResultH0, _mm256_adds_epi16(_mm256_alignr256_epi8(yY0_1, yY0_0, 2), _mm256_alignr256_epi8(yY4_1, yY4_0, 2)));

    yResultH1 = _mm256_alignr256_epi8(yY0_1, yY0_0, 8);
    yResultH1 = _mm256_adds_epi16(yResultH1, _mm256_adds_epi16(_mm256_alignr256_epi8(yY1_1, yY1_0, 8), _mm256_alignr256_epi8(yY2_1, yY2_0, 8)));
    yResultH1 = _mm256_adds_epi16(yResultH1, _mm256_adds_epi16(_mm256_alignr256_epi8(yY3_1, yY3_0, 8), _mm256_alignr256_epi8(yY4_1, yY4_0, 8)));
    yResultH1 = _mm256_adds_epi16(yResultH1, _mm256_adds_epi16(_mm256_alignr256_epi8(yY0_1, yY0_0, 6), _mm256_alignr256_epi8(yY4_1, yY4_0, 6)));
    yResultH0 = _mm256_subs_epi16(yResultH0, yResultH1);

    yResultH2 = _mm256_alignr256_epi8(yY1_1, yY1_0, 2);
    yResultH3 = _mm256_alignr256_epi8(yY1_1, yY1_0, 6);
    yResultH2 = _mm256_adds_epi16(yResultH2, _mm256_adds_epi16(_mm256_alignr256_epi8(yY2_1,  yY2_0, 2), _mm256_alignr256_epi8(yY3_1,  yY3_0, 2)));
    yResultH3 = _mm256_adds_epi16(yResultH3, _mm256_adds_epi16(_mm256_alignr256_epi8(yY2_1,  yY2_0, 6), _mm256_alignr256_epi8(yY3_1,  yY3_0, 6)));
    yResultH2 = _mm256_subs_epi16(yResultH2, yResultH3);

    //Vertical
    yResultV0 = yY0_0;
    yResultV0 = _mm256_adds_epi16(yResultV0, _mm256_adds_epi16(_mm256_alignr256_epi8(yY0_1, yY0_0, 2), _mm256_alignr256_epi8(yY0_1, yY0_0, 4)));
    yResultV0 = _mm256_adds_epi16(yResultV0, _mm256_adds_epi16(_mm256_alignr256_epi8(yY0_1, yY0_0, 6), _mm256_alignr256_epi8(yY0_1, yY0_0, 8)));
    yResultV0 = _mm256_adds_epi16(yResultV0, _mm256_adds_epi16(yY1_0,                                  _mm256_alignr256_epi8(yY1_1, yY1_0, 8)));

    yResultV1 = yY4_0;
    yResultV1 = _mm256_adds_epi16(yResultV1, _mm256_adds_epi16(_mm256_alignr256_epi8(yY4_1, yY4_0, 2), _mm256_alignr256_epi8(yY4_1, yY4_0, 4)));
    yResultV1 = _mm256_adds_epi16(yResultV1, _mm256_adds_epi16(_mm256_alignr256_epi8(yY4_1, yY4_0, 6), _mm256_alignr256_epi8(yY4_1, yY4_0, 8)));
    yResultV1 = _mm256_adds_epi16(yResultV1, _mm256_adds_epi16(yY3_0,                                  _mm256_alignr256_epi8(yY3_1, yY3_0, 8)));
    yResultV0 = _mm256_subs_epi16(yResultV0, yResultV1);

    yResultV2 = _mm256_alignr256_epi8(yY1_1, yY1_0, 2);
    yResultV3 = _mm256_alignr256_epi8(yY3_1, yY3_0, 2);
    yResultV2 = _mm256_adds_epi16(yResultV2, _mm256_adds_epi16(_mm256_alignr256_epi8(yY1_1, yY1_0, 4), _mm256_alignr256_epi8(yY1_1, yY1_0, 6)));
    yResultV3 = _mm256_adds_epi16(yResultV3, _mm256_adds_epi16(_mm256_alignr256_epi8(yY3_1, yY3_0, 4), _mm256_alignr256_epi8(yY3_1, yY3_0, 6)));
    yResultV2 = _mm256_subs_epi16(yResultV2, yResultV3);

    yResultH0 = prewitt_filter_5x5_add_simd(yResultH0, yResultH2);
    yResultV0 = prewitt_filter_5x5_add_simd(yResultV0, yResultV2);

    return _mm256_alpha_max_plus_beta_min_epi16(yResultH0, yResultV0);
}

#elif USE_SSE2
static __forceinline void prewitt_filter_simd(__m128i& xResultH, __m128i& xResultV, const __m128i& xY0High, const __m128i& xY1High, const __m128i& xY0Middle, const __m128i& xY1Middle, const __m128i& xY0Lower, const __m128i& xY1Lower) {
    __m128i xResultV0, xResultV1, xResultH0, xResultH1;
    //Horizontal
    xResultH0 = _mm_adds_epi16(xY0High, _mm_adds_epi16(xY0Middle, xY0Lower));
    xResultH1 = _mm_alignr_epi8_simd(xY1High, xY0High, 4);
    xResultH1 = _mm_adds_epi16(xResultH1, _mm_alignr_epi8_simd(xY1Middle, xY0Middle, 4));
    xResultH1 = _mm_adds_epi16(xResultH1, _mm_alignr_epi8_simd(xY1Lower,  xY0Lower, 4));

    //Vertical
    xResultV0 = xY0High;
    xResultV0 = _mm_adds_epi16(xResultV0, _mm_alignr_epi8_simd(xY1High, xY0High, 2));
    xResultV0 = _mm_adds_epi16(xResultV0, _mm_alignr_epi8_simd(xY1High, xY0High, 4));
    xResultV1 = xY0Lower;
    xResultV1 = _mm_adds_epi16(xResultV1, _mm_alignr_epi8_simd(xY1Lower, xY0Lower, 2));
    xResultV1 = _mm_adds_epi16(xResultV1, _mm_alignr_epi8_simd(xY1Lower, xY0Lower, 4));

    xResultH = _mm_subs_epi16(xResultH1, xResultH0);
    xResultV = _mm_subs_epi16(xResultV1, xResultV0);
}

static __forceinline __m128i prewitt_filter_simd(const __m128i& xY0High, const __m128i& xY1High, const __m128i& xY0Middle, const __m128i& xY1Middle, const __m128i& xY0Lower, const __m128i& xY1Lower) {
    __m128i xResultH, xResultV;
    prewitt_filter_simd(xResultH, xResultV, xY0High, xY1High, xY0Middle, xY1Middle, xY0Lower, xY1Lower);
    return _mm_alpha_max_plus_beta_min_epi16(xResultH, xResultV);
}

static __forceinline __m128i prewitt_filter_simd_with_weight(__m128i xSrc, const __m128i& xY0High, const __m128i& xY1High, const __m128i& xY0Middle, const __m128i& xY1Middle, const __m128i& xY0Lower, const __m128i& xY1Lower) {
    __m128i xResultH, xResultV;
    prewitt_filter_simd(xResultH, xResultV, xY0High, xY1High, xY0Middle, xY1Middle, xY0Lower, xY1Lower);

    __m128 x0, x1, x2, x3, xWeight0, xWeight1;
    x0 = _mm_cvtepi32_ps(cvtlo_epi16_epi32(xResultH));
    x1 = _mm_cvtepi32_ps(cvthi_epi16_epi32(xResultH));
    x2 = _mm_cvtepi32_ps(cvtlo_epi16_epi32(xResultV));
    x3 = _mm_cvtepi32_ps(cvthi_epi16_epi32(xResultV));
    xSrc = _mm_min_epi16(xSrc, _mm_set1_epi16(4096));
    xWeight0 = _mm_cvtepi32_ps(cvtlo_epi16_epi32(xSrc));
    xWeight1 = _mm_cvtepi32_ps(cvthi_epi16_epi32(xSrc));
    xWeight0 = _mm_add_ps(_mm_mul_ps(xWeight0, _mm_set1_ps(-1 * 4.0f / (2200.0f * 9.0f))), _mm_set1_ps(1.0f));
    xWeight1 = _mm_add_ps(_mm_mul_ps(xWeight1, _mm_set1_ps(-1 * 4.0f / (2200.0f * 9.0f))), _mm_set1_ps(1.0f));
    x0 = _mm_sqrt_ps(_mm_add_ps(_mm_mul_ps(x0, x0), _mm_mul_ps(x2, x2)));
    x1 = _mm_sqrt_ps(_mm_add_ps(_mm_mul_ps(x1, x1), _mm_mul_ps(x3, x3)));
    xWeight0 = _mm_rcp_ps_hp(xWeight0);
    xWeight1 = _mm_rcp_ps_hp(xWeight1);
    x0 = _mm_mul_ps(x0, xWeight0);
    x1 = _mm_mul_ps(x1, xWeight1);

    return _mm_packs_epi32(_mm_cvtps_epi32(x0), _mm_cvtps_epi32(x1));
}

static __forceinline __m128i prewitt_filter_5x5_add_simd(__m128i x0, __m128i x1) {
    __m128i xSumHi, xSumLo;
    xSumLo = cvtlo_epi16_epi32(x0);
    xSumHi = cvthi_epi16_epi32(x0);
    xSumLo = _mm_add_epi32(xSumLo, cvtlo_epi16_epi32(x1));
    xSumHi = _mm_add_epi32(xSumHi, cvthi_epi16_epi32(x1));
    xSumLo = _mm_srai_epi32(xSumLo, 2);
    xSumHi = _mm_srai_epi32(xSumHi, 2);
    return _mm_packs_epi32(xSumLo, xSumHi);
}

static __forceinline __m128i prewitt_filter_5x5_simd(
    const __m128i& xY0_0, const __m128i& xY0_1,
    const __m128i& xY1_0, const __m128i& xY1_1,
    const __m128i& xY2_0, const __m128i& xY2_1,
    const __m128i& xY3_0, const __m128i& xY3_1,
    const __m128i& xY4_0, const __m128i& xY4_1) {
    __m128i xResultV0, xResultV1, xResultV2, xResultV3, xResultH0, xResultH1, xResultH2, xResultH3;
    //Horizontal
    xResultH0 = xY0_0;
    xResultH0 = _mm_adds_epi16(xResultH0, _mm_adds_epi16(_mm_adds_epi16(xY1_0, xY2_0), _mm_adds_epi16(xY3_0, xY4_0)));
    xResultH0 = _mm_adds_epi16(xResultH0, _mm_adds_epi16(_mm_alignr_epi8_simd(xY0_1, xY0_0, 2), _mm_alignr_epi8_simd(xY4_1, xY4_0, 2)));

    xResultH1 = _mm_alignr_epi8_simd(xY0_1, xY0_0, 8);
    xResultH1 = _mm_adds_epi16(xResultH1, _mm_adds_epi16(_mm_alignr_epi8_simd(xY1_1, xY1_0, 8), _mm_alignr_epi8_simd(xY2_1, xY2_0, 8)));
    xResultH1 = _mm_adds_epi16(xResultH1, _mm_adds_epi16(_mm_alignr_epi8_simd(xY3_1, xY3_0, 8), _mm_alignr_epi8_simd(xY4_1, xY4_0, 8)));
    xResultH1 = _mm_adds_epi16(xResultH1, _mm_adds_epi16(_mm_alignr_epi8_simd(xY0_1, xY0_0, 6), _mm_alignr_epi8_simd(xY4_1, xY4_0, 6)));
    xResultH0 = _mm_subs_epi16(xResultH0, xResultH1);

    xResultH2 = _mm_alignr_epi8_simd(xY1_1, xY1_0, 2);
    xResultH3 = _mm_alignr_epi8_simd(xY1_1, xY1_0, 6);
    xResultH2 = _mm_adds_epi16(xResultH2, _mm_adds_epi16(_mm_alignr_epi8_simd(xY2_1,  xY2_0, 2), _mm_alignr_epi8_simd(xY3_1,  xY3_0, 2)));
    xResultH3 = _mm_adds_epi16(xResultH3, _mm_adds_epi16(_mm_alignr_epi8_simd(xY2_1,  xY2_0, 6), _mm_alignr_epi8_simd(xY3_1,  xY3_0, 6)));
    xResultH2 = _mm_subs_epi16(xResultH2, xResultH3);

    //Vertical
    xResultV0 = xY0_0;
    xResultV0 = _mm_adds_epi16(xResultV0, _mm_adds_epi16(_mm_alignr_epi8_simd(xY0_1, xY0_0, 2), _mm_alignr_epi8_simd(xY0_1, xY0_0, 4)));
    xResultV0 = _mm_adds_epi16(xResultV0, _mm_adds_epi16(_mm_alignr_epi8_simd(xY0_1, xY0_0, 6), _mm_alignr_epi8_simd(xY0_1, xY0_0, 8)));
    xResultV0 = _mm_adds_epi16(xResultV0, _mm_adds_epi16(xY1_0,                                 _mm_alignr_epi8_simd(xY1_1, xY1_0, 8)));

    xResultV1 = xY4_0;
    xResultV1 = _mm_adds_epi16(xResultV1, _mm_adds_epi16(_mm_alignr_epi8_simd(xY4_1, xY4_0, 2), _mm_alignr_epi8_simd(xY4_1, xY4_0, 4)));
    xResultV1 = _mm_adds_epi16(xResultV1, _mm_adds_epi16(_mm_alignr_epi8_simd(xY4_1, xY4_0, 6), _mm_alignr_epi8_simd(xY4_1, xY4_0, 8)));
    xResultV1 = _mm_adds_epi16(xResultV1, _mm_adds_epi16(xY3_0,                                 _mm_alignr_epi8_simd(xY3_1, xY3_0, 8)));
    xResultV0 = _mm_subs_epi16(xResultV0, xResultV1);

    xResultV2 = _mm_alignr_epi8_simd(xY1_1, xY1_0, 2);
    xResultV3 = _mm_alignr_epi8_simd(xY3_1, xY3_0, 2);
    xResultV2 = _mm_adds_epi16(xResultV2, _mm_adds_epi16(_mm_alignr_epi8_simd(xY1_1, xY1_0, 4), _mm_alignr_epi8_simd(xY1_1, xY1_0, 6)));
    xResultV3 = _mm_adds_epi16(xResultV3, _mm_adds_epi16(_mm_alignr_epi8_simd(xY3_1, xY3_0, 4), _mm_alignr_epi8_simd(xY3_1, xY3_0, 6)));
    xResultV2 = _mm_subs_epi16(xResultV2, xResultV3);

    xResultH0 = prewitt_filter_5x5_add_simd(xResultH0, xResultH2);
    xResultV0 = prewitt_filter_5x5_add_simd(xResultV0, xResultV2);

    return _mm_alpha_max_plus_beta_min_epi16(xResultH0, xResultV0);
}

#else

template<class type>
static __forceinline int get_logo_y(const type *ptr_lgp, int logo_width, int x_offset, int y_offset) {
    ptr_lgp += x_offset;
    ptr_lgp += y_offset * logo_width;
    return *ptr_lgp;
}

template<>
static __forceinline int get_logo_y<LOGO_PIXEL>(const LOGO_PIXEL *ptr_lgp, int logo_width, int x_offset, int y_offset) {
    ptr_lgp += x_offset;
    ptr_lgp += y_offset * logo_width;
    return ptr_lgp->y * ptr_lgp->dp_y / LOGO_FADE_MAX;
}

template<class type>
static __forceinline USHORT prewitt_filter(const type *ptr_lgp, int logo_width) {
    int y_sum_h = 0, y_sum_v = 0;
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -1, -1);
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -1,  0);
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -1,  1);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  1, -1);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  1,  0);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  1,  1);

    y_sum_v += get_logo_y(ptr_lgp, logo_width, -1,  1);
    y_sum_v += get_logo_y(ptr_lgp, logo_width,  0,  1);
    y_sum_v += get_logo_y(ptr_lgp, logo_width,  1,  1);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width, -1, -1);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width,  0, -1);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width,  1, -1);

    int absA = std::abs(y_sum_h);
    int absB = std::abs(y_sum_v);
    int maxY = std::max(absA, absB);
    int minY = std::min(absA, absB);
    return (USHORT)((maxY * 123 + minY * 51) >> 7);
    //return (USHORT)sqrt(((double)y_sum_h * (double)y_sum_h + (double)y_sum_v * (double)y_sum_v));
}

static __forceinline USHORT weighted_prewitt_filter(const short *ptr_lgp, int logo_pitch) {
    int y_sum_h = 0, y_sum_v = 0;
#if 0
    y_sum_h -= ptr_lgp[-1 + logo_pitch * -1];
    y_sum_h -= ptr_lgp[-1             ];
    y_sum_h -= ptr_lgp[-1 + logo_pitch *  1];
    y_sum_h += ptr_lgp[ 1 + logo_pitch * -1];
    y_sum_h += ptr_lgp[ 1             ];
    y_sum_h += ptr_lgp[ 1 + logo_pitch *  1];
    y_sum_v -= ptr_lgp[-1 + logo_pitch * -1];
    y_sum_v -= ptr_lgp[ 0 + logo_pitch * -1];
    y_sum_v -= ptr_lgp[ 1 + logo_pitch * -1];
    y_sum_v += ptr_lgp[-1 + logo_pitch *  1];
    y_sum_v += ptr_lgp[ 0 + logo_pitch *  1];
    y_sum_v += ptr_lgp[ 1 + logo_pitch *  1];
#else
    y_sum_h -= get_logo_y(ptr_lgp, logo_pitch, -1, -1);
    y_sum_h -= get_logo_y(ptr_lgp, logo_pitch, -1,  0);
    y_sum_h -= get_logo_y(ptr_lgp, logo_pitch, -1,  1);
    y_sum_h += get_logo_y(ptr_lgp, logo_pitch,  1, -1);
    y_sum_h += get_logo_y(ptr_lgp, logo_pitch,  1,  0);
    y_sum_h += get_logo_y(ptr_lgp, logo_pitch,  1,  1);

    y_sum_v -= get_logo_y(ptr_lgp, logo_pitch, -1, -1);
    y_sum_v -= get_logo_y(ptr_lgp, logo_pitch,  0, -1);
    y_sum_v -= get_logo_y(ptr_lgp, logo_pitch,  1, -1);
    y_sum_v += get_logo_y(ptr_lgp, logo_pitch, -1,  1);
    y_sum_v += get_logo_y(ptr_lgp, logo_pitch,  0,  1);
    y_sum_v += get_logo_y(ptr_lgp, logo_pitch,  1,  1);
#endif
    float n_val = std::sqrt(((float)y_sum_h * (float)y_sum_h + (float)y_sum_v * (float)y_sum_v));
    int n_coef = std::min(4096 , (int)*ptr_lgp);
    return (short)(n_val / (1.0f - (n_coef * 4.0f / (float)(9 * 2200))) + 0.5f);
    //return n_val * 2200 / (2200- (n_coef * 4 / 9));
}

template<class type>
static __forceinline USHORT prewitt_filter_5x5(const type *ptr_lgp, int logo_width) {
    // 5x5 Prewitt filter
    // +----------------+  +----------------+
    // | -1 -1 -1 -1 -1 |  | -1 -1  0  1  1 |
    // | -1 -1 -1 -1 -1 |  | -1 -1  0  1  1 |
    // |  0  0  0  0  0 |  | -1 -1  0  1  1 |
    // |  1  1  1  1  1 |  | -1 -1  0  1  1 |
    // |  1  1  1  1  1 |  | -1 -1  0  1  1 |
    // +----------------+  +----------------+
    int y_sum_h = 0, y_sum_v = 0;
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -2, -2);
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -2, -1);
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -2,  0);
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -2,  1);
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -2,  2);
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -1, -2);
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -1, -1);
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -1,  0);
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -1,  1);
    y_sum_h -= get_logo_y(ptr_lgp, logo_width, -1,  2);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  1, -2);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  1, -1);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  1,  0);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  1,  1);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  2,  2);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  2, -2);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  2, -1);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  2,  0);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  2,  1);
    y_sum_h += get_logo_y(ptr_lgp, logo_width,  2,  2);

    y_sum_v += get_logo_y(ptr_lgp, logo_width, -2,  2);
    y_sum_v += get_logo_y(ptr_lgp, logo_width, -1,  2);
    y_sum_v += get_logo_y(ptr_lgp, logo_width,  0,  2);
    y_sum_v += get_logo_y(ptr_lgp, logo_width,  1,  2);
    y_sum_v += get_logo_y(ptr_lgp, logo_width,  2,  2);
    y_sum_v += get_logo_y(ptr_lgp, logo_width, -2,  1);
    y_sum_v += get_logo_y(ptr_lgp, logo_width, -1,  1);
    y_sum_v += get_logo_y(ptr_lgp, logo_width,  0,  1);
    y_sum_v += get_logo_y(ptr_lgp, logo_width,  1,  1);
    y_sum_v += get_logo_y(ptr_lgp, logo_width,  2,  1);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width, -2, -1);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width, -1, -1);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width,  0, -1);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width,  1, -1);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width,  2, -1);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width, -2, -2);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width, -1, -2);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width,  0, -2);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width,  1, -2);
    y_sum_v -= get_logo_y(ptr_lgp, logo_width,  2, -2);

    y_sum_h >>= 2;
    y_sum_v >>= 2;

    int absA = std::abs(y_sum_h);
    int absB = std::abs(y_sum_v);
    int maxY = std::max(absA, absB);
    int minY = std::min(absA, absB);
    return (USHORT)((maxY * 123 + minY * 51) >> 7);
}

#endif

void __stdcall func_proc_prewitt_filter_c(short *logo_mask, const LOGO_HEADER *lgh);
void __stdcall func_proc_weighted_prewitt_filter_c(short *dst, const short *src, int logo_width, int logo_src_pitch, int height);

static __forceinline void func_proc_prewitt_filter_simd(short *logo_mask, const LOGO_HEADER *lgh) {
    const int logo_width = lgh->w;
    if (logo_width < (USE_AVX2 ? 32 : 16) + 2)
        return func_proc_prewitt_filter_c(logo_mask, lgh);

#if USE_AVX2
    __m256i yY0High, yY1High, yY0Middle, yY1Middle, yY0Lower, yY1Lower, yResult0, yResult1;
#elif USE_SSE2
    __m128i xY0High, xY1High, xY0Middle, xY1Middle, xY0Lower, xY1Lower, xResult0, xResult1;
#endif
    const int logo_src_pitch = logo_width;
    const int logo_dst_pitch = PITCH(logo_width);
    const LOGO_PIXEL *lgp_line = (const LOGO_PIXEL *)(lgh + 1);
    lgp_line += logo_src_pitch;
    short *logo_mask_line = logo_mask + logo_dst_pitch;
    const int y_fin = lgh->h - 1;
    int x_count = logo_width - 2;
    for (int y = 1; y < y_fin; y++, lgp_line += logo_src_pitch, logo_mask_line += logo_dst_pitch) {
        const LOGO_PIXEL *ptr_lgp = lgp_line;
        short *logo_mask_ptr = logo_mask_line;
        //SIMD版では、範囲外アクセス防止の為、最後の部分のみ特別に処理する
#if USE_SSE2 || USE_AVX2
        x_count -= (y == y_fin-1) * (USE_AVX2 ? 16 : 8);
#endif
#if USE_AVX2
        yResult0  = _mm256_setzero_si256();
        yY0High   = get_logo_y(ptr_lgp, logo_src_pitch, 0, -1);
        yY0Middle = get_logo_y(ptr_lgp, logo_src_pitch, 0,  0);
        yY0Lower  = get_logo_y(ptr_lgp, logo_src_pitch, 0,  1);
        for (int x = 0; x < x_count; x += 16, ptr_lgp += 16, logo_mask_ptr += 16) {
            yY1High   = get_logo_y(ptr_lgp, logo_src_pitch, 16, -1);
            yY1Middle = get_logo_y(ptr_lgp, logo_src_pitch, 16,  0);
            yY1Lower  = get_logo_y(ptr_lgp, logo_src_pitch, 16,  1);
            yResult1  = prewitt_filter_simd(yY0High, yY1High, yY0Middle, yY1Middle, yY0Lower, yY1Lower);
            //計算結果は1ピクセルずれた位置の値なので、alignrでずらして格納する
            _mm256_storeu_si256((__m256i *)logo_mask_ptr, _mm256_alignr256_epi8(yResult1, yResult0, 30));

            yY0High   = yY1High;
            yY0Middle = yY1Middle;
            yY0Lower  = yY1Lower;
            yResult0  = yResult1;
        }
#elif USE_SSE2
        xResult0  = _mm_setzero_si128();
        xY0High   = get_logo_y(ptr_lgp, logo_src_pitch, 0, -1);
        xY0Middle = get_logo_y(ptr_lgp, logo_src_pitch, 0,  0);
        xY0Lower  = get_logo_y(ptr_lgp, logo_src_pitch, 0,  1);
        for (int x = 0; x < x_count; x += 8, ptr_lgp += 8, logo_mask_ptr += 8) {
            xY1High   = get_logo_y(ptr_lgp, logo_src_pitch, 8, -1);
            xY1Middle = get_logo_y(ptr_lgp, logo_src_pitch, 8,  0);
            xY1Lower  = get_logo_y(ptr_lgp, logo_src_pitch, 8,  1);
            xResult1  = prewitt_filter_simd(xY0High, xY1High, xY0Middle, xY1Middle, xY0Lower, xY1Lower);
            //計算結果は1ピクセルずれた位置の値なので、alignrでずらして格納する
            _mm_storeu_si128((__m128i *)logo_mask_ptr, _mm_alignr_epi8_simd(xResult1, xResult0, 14));

            xY0High   = xY1High;
            xY0Middle = xY1Middle;
            xY0Lower  = xY1Lower;
            xResult0  = xResult1;
        }
#else
        *logo_mask_ptr = 0;
        ptr_lgp++;
        logo_mask_ptr++;
        for (int x = 0; x < x_count; x++, ptr_lgp++, logo_mask_ptr++) {
            *logo_mask_ptr = prewitt_filter(ptr_lgp, logo_width);
        }
#endif
        for (int x = logo_width-1; x < logo_dst_pitch; x++)
            logo_mask_line[x] = 0;
    }
    //SIMD版では、範囲外アクセス防止の為、最後の部分のみ特別に処理する
#if USE_SSE2
    //ロゴの最後のピクセル(のひとつ次)の位置
    short *logo_mask_fin = logo_mask_line - logo_dst_pitch + logo_width;
    const LOGO_PIXEL *lgp_fin = lgp_line - logo_src_pitch + logo_width;
#if USE_AVX2
    yY0High   = get_logo_y(lgp_fin, logo_src_pitch, -18, -1);
    yY0Middle = get_logo_y(lgp_fin, logo_src_pitch, -18,  0);
    yY0Lower  = get_logo_y(lgp_fin, logo_src_pitch, -18,  1);
    //最後が範囲外例外にならないようにロードしてからシフトで位置を合わせる
    yY1High   = get_logo_y(lgp_fin, logo_src_pitch, -16, -1);
    yY1Middle = get_logo_y(lgp_fin, logo_src_pitch, -16,  0);
    yY1Lower  = get_logo_y(lgp_fin, logo_src_pitch, -16,  1);
    yY1High   = _mm256_srli256_si256(yY1High,   28);
    yY1Middle = _mm256_srli256_si256(yY1Middle, 28);
    yY1Lower  = _mm256_srli256_si256(yY1Lower,  28);
    _mm256_storeu_si256((__m256i *)(logo_mask_fin - 17), prewitt_filter_simd(yY0High, yY1High, yY0Middle, yY1Middle, yY0Lower, yY1Lower));
#elif USE_SSE2
    xY0High   = get_logo_y(lgp_fin, logo_src_pitch, -10, -1);
    xY0Middle = get_logo_y(lgp_fin, logo_src_pitch, -10,  0);
    xY0Lower  = get_logo_y(lgp_fin, logo_src_pitch, -10,  1);
    //最後が範囲外例外にならないようにロードしてからシフトで位置を合わせる
    xY1High   = get_logo_y(lgp_fin, logo_src_pitch,  -8, -1);
    xY1Middle = get_logo_y(lgp_fin, logo_src_pitch,  -8,  0);
    xY1Lower  = get_logo_y(lgp_fin, logo_src_pitch,  -8,  1);
    xY1High   = _mm_srli_si128(xY1High,   12);
    xY1Middle = _mm_srli_si128(xY1Middle, 12);
    xY1Lower  = _mm_srli_si128(xY1Lower,  12);
    _mm_storeu_si128((__m128i *)(logo_mask_fin - 9), prewitt_filter_simd(xY0High, xY1High, xY0Middle, xY1Middle, xY0Lower, xY1Lower));
#endif
#endif
}

static __forceinline void func_proc_weighted_prewitt_filter_simd(short *dst, const short *src, int logo_width, int logo_src_pitch, int height) {
    if (logo_width < (USE_AVX2 ? 32 : 16) + 2)
        return func_proc_weighted_prewitt_filter_c(dst, src, logo_width, logo_src_pitch, height);

    memset(dst, 0, sizeof(short) * logo_src_pitch);
#if USE_AVX2
    __m256i yY0High, yY1High, yY0Middle, yY1Middle, yY0Lower, yY1Lower, yResult0, yResult1;
#elif USE_SSE2
    __m128i xY0High, xY1High, xY0Middle, xY1Middle, xY0Lower, xY1Lower, xResult0, xResult1;
#endif
    const short *src_line = src + logo_src_pitch;
    short *dst_line = dst + logo_src_pitch;
    const int y_fin = height - 1;
    int x_count = logo_width - 2;
    for (int y = 1; y < y_fin; y++, src_line += logo_src_pitch, dst_line += logo_src_pitch) {
        const short *ptr_src = src_line;
        short *ptr_dst = dst_line;
        //SIMD版では、範囲外アクセス防止の為、最後の部分のみ特別に処理する
#if USE_SSE2 || USE_AVX2
        x_count -= (y == y_fin-1) * (USE_AVX2 ? 16 : 8);
#endif
#if USE_AVX2
        yResult0  = _mm256_setzero_si256();
        yY0High   = get_logo_y(ptr_src, logo_src_pitch, 0, -1);
        yY0Middle = get_logo_y(ptr_src, logo_src_pitch, 0,  0);
        yY0Lower  = get_logo_y(ptr_src, logo_src_pitch, 0,  1);
        for (int x = 0; x < x_count; x += 16, ptr_src += 16, ptr_dst += 16) {
            yY1High   = get_logo_y(ptr_src, logo_src_pitch, 16, -1);
            yY1Middle = get_logo_y(ptr_src, logo_src_pitch, 16,  0);
            yY1Lower  = get_logo_y(ptr_src, logo_src_pitch, 16,  1);
            yResult1  = prewitt_filter_simd_with_weight(_mm256_alignr256_epi8(yY1Middle, yY0Middle, 2), yY0High, yY1High, yY0Middle, yY1Middle, yY0Lower, yY1Lower);
            //計算結果は1ピクセルずれた位置の値なので、alignrでずらして格納する
            _mm256_storeu_si256((__m256i *)ptr_dst, _mm256_alignr256_epi8(yResult1, yResult0, 30));

            yY0High   = yY1High;
            yY0Middle = yY1Middle;
            yY0Lower  = yY1Lower;
            yResult0  = yResult1;
        }
#elif USE_SSE2
        xResult0  = _mm_setzero_si128();
        xY0High   = get_logo_y(ptr_src, logo_src_pitch, 0, -1);
        xY0Middle = get_logo_y(ptr_src, logo_src_pitch, 0,  0);
        xY0Lower  = get_logo_y(ptr_src, logo_src_pitch, 0,  1);
        for (int x = 0; x < x_count; x += 8, ptr_src += 8, ptr_dst += 8) {
            xY1High   = get_logo_y(ptr_src, logo_src_pitch, 8, -1);
            xY1Middle = get_logo_y(ptr_src, logo_src_pitch, 8,  0);
            xY1Lower  = get_logo_y(ptr_src, logo_src_pitch, 8,  1);
            xResult1  = prewitt_filter_simd_with_weight(_mm_alignr_epi8_simd(xY1Middle, xY0Middle, 2), xY0High, xY1High, xY0Middle, xY1Middle, xY0Lower, xY1Lower);
            //計算結果は1ピクセルずれた位置の値なので、alignrでずらして格納する
            _mm_storeu_si128((__m128i *)ptr_dst, _mm_alignr_epi8_simd(xResult1, xResult0, 14));

            xY0High   = xY1High;
            xY0Middle = xY1Middle;
            xY0Lower  = xY1Lower;
            xResult0  = xResult1;
        }
#else
        *ptr_dst = 0;
        ptr_src++;
        ptr_dst++;
        for (int x = 0; x < x_count; x++, ptr_src++, ptr_dst++) {
            *ptr_dst = weighted_prewitt_filter(ptr_src, logo_src_pitch);
        }
#endif
        for (int x = logo_width-1; x < logo_src_pitch; x++)
            dst_line[x] = 0;
    }
    //SIMD版では、範囲外アクセス防止の為、最後の部分のみ特別に処理する
#if USE_SSE2
    //ロゴの最後のピクセル(のひとつ次)の位置
    short *logo_mask_fin = dst_line - logo_src_pitch + logo_width;
    const short *src_fin = src_line - logo_src_pitch + logo_width;
#if USE_AVX2
    yY0High   = get_logo_y(src_fin, logo_src_pitch, -18, -1);
    yY0Middle = get_logo_y(src_fin, logo_src_pitch, -18,  0);
    yY0Lower  = get_logo_y(src_fin, logo_src_pitch, -18,  1);
    //最後が範囲外例外にならないようにロードしてからシフトで位置を合わせる
    yY1High   = get_logo_y(src_fin, logo_src_pitch, -16, -1);
    yY1Middle = get_logo_y(src_fin, logo_src_pitch, -16,  0);
    yY1Lower  = get_logo_y(src_fin, logo_src_pitch, -16,  1);
    yY1High   = _mm256_srli256_si256(yY1High,   28);
    yY1Middle = _mm256_srli256_si256(yY1Middle, 28);
    yY1Lower  = _mm256_srli256_si256(yY1Lower,  28);
    _mm256_storeu_si256((__m256i *)(logo_mask_fin - 17), prewitt_filter_simd_with_weight(_mm256_alignr256_epi8(yY1Middle, yY0Middle, 2),yY0High, yY1High, yY0Middle, yY1Middle, yY0Lower, yY1Lower));
#elif USE_SSE2
    xY0High   = get_logo_y(src_fin, logo_src_pitch, -10, -1);
    xY0Middle = get_logo_y(src_fin, logo_src_pitch, -10,  0);
    xY0Lower  = get_logo_y(src_fin, logo_src_pitch, -10,  1);
    //最後が範囲外例外にならないようにロードしてからシフトで位置を合わせる
    xY1High   = get_logo_y(src_fin, logo_src_pitch,  -8, -1);
    xY1Middle = get_logo_y(src_fin, logo_src_pitch,  -8,  0);
    xY1Lower  = get_logo_y(src_fin, logo_src_pitch,  -8,  1);
    xY1High   = _mm_srli_si128(xY1High,   12);
    xY1Middle = _mm_srli_si128(xY1Middle, 12);
    xY1Lower  = _mm_srli_si128(xY1Lower,  12);
    _mm_storeu_si128((__m128i *)(logo_mask_fin - 9), prewitt_filter_simd_with_weight(_mm_alignr_epi8_simd(xY1Middle, xY0Middle, 2), xY0High, xY1High, xY0Middle, xY1Middle, xY0Lower, xY1Lower));
#endif
#endif
#if USE_AVX2
    _mm256_zeroupper();
#endif
    memset(dst_line, 0, sizeof(short) * logo_src_pitch);
}




unsigned int __stdcall func_proc_prewitt_evaluate_c(const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);

unsigned int __stdcall func_proc_prewitt_5x5_evaluate_c(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_5x5_evaluate_store_c(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);

static __forceinline unsigned int func_proc_prewitt_evaluate_simd(const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result) {
    const int logo_width = PITCH(lgh->w);
    if (logo_width < (USE_AVX2 ? 32 : 16) + 2)
        return func_proc_prewitt_evaluate_c(logo_mask, src, lgh, auto_result);

#if USE_AVX2
    __m256i yY0High, yY1High, yY0Middle, yY1Middle, yY0Lower, yY1Lower, yResult0, yResult1, yCount, yMask, yTemp;
#elif USE_SSE2
    __m128i xY0High, xY1High, xY0Middle, xY1Middle, xY0Lower, xY1Lower, xResult0, xResult1, xCount, xMask, xTemp;
#endif
    const short *src_line = src + logo_width;
    const short *logo_mask_line = logo_mask + logo_width;
    const int y_fin = lgh->h - 1;
    int x_count = logo_width;
#if USE_SSE2 || USE_AVX2
    //SIMD版では、範囲外アクセス防止の為、最後の部分のみ特別に処理する
    x_count -= (USE_AVX2 ? 16 : 8);
#endif
    DWORD result = 0;
    for (int y = 1; y < y_fin; y++, src_line += logo_width, logo_mask_line += logo_width) {
        const short *ptr_src = src_line;
        const short *logo_mask_ptr = logo_mask_line;
#if USE_AVX2
        yCount   = _mm256_setzero_si256();
        yResult0  = _mm256_setzero_si256();
        yY0High   = get_logo_y(ptr_src, logo_width, 0, -1);
        yY0Middle = get_logo_y(ptr_src, logo_width, 0,  0);
        yY0Lower  = get_logo_y(ptr_src, logo_width, 0,  1);
        for (int x = 0; x < x_count; x += 16, ptr_src += 16, logo_mask_ptr += 16) {
            yY1High   = get_logo_y(ptr_src, logo_width, 16, -1);
            yY1Middle = get_logo_y(ptr_src, logo_width, 16,  0);
            yY1Lower  = get_logo_y(ptr_src, logo_width, 16,  1);
            yResult1  = prewitt_filter_simd(yY0High, yY1High, yY0Middle, yY1Middle, yY0Lower, yY1Lower);
            yTemp = _mm256_alignr256_epi8(yResult1, yResult0, 30);

            yMask = _mm256_load_si256((__m256i *)logo_mask_ptr);
            yMask = _mm256_cmpgt_epi16(yMask, _mm256_set1_epi16(1024));

            yTemp = _mm256_and_si256(yTemp, yMask);

            yCount = _mm256_add_epi32(yCount, _mm256_unpacklo_epi16(yTemp, _mm256_setzero_si256()));
            yCount = _mm256_add_epi32(yCount, _mm256_unpackhi_epi16(yTemp, _mm256_setzero_si256()));

            yY0High   = yY1High;
            yY0Middle = yY1Middle;
            yY0Lower  = yY1Lower;
            yResult0  = yResult1;
        }
        //最後の切れ端
        //常にアライメントがとれているため、最後はゼロをロードしておけば良い
        yY1High   = _mm256_setzero_si256();
        yY1Middle = _mm256_setzero_si256();
        yY1Lower  = _mm256_setzero_si256();

        //あとは普通に計算
        yResult1  = prewitt_filter_simd(yY0High, yY1High, yY0Middle, yY1Middle, yY0Lower, yY1Lower);
        yTemp = _mm256_alignr256_epi8(yResult1, yResult0, 30);

        yMask = _mm256_load_si256((__m256i *)logo_mask_ptr);
        yMask = _mm256_cmpgt_epi16(yMask, _mm256_set1_epi16(1024));

        yTemp = _mm256_and_si256(yTemp, yMask);

        //32bitに拡張してから総和演算
        yCount = _mm256_add_epi32(yCount, _mm256_unpacklo_epi16(yTemp, _mm256_setzero_si256()));
        yCount = _mm256_add_epi32(yCount, _mm256_unpackhi_epi16(yTemp, _mm256_setzero_si256()));

        result += (yCount.m256i_u32[0] + yCount.m256i_u32[1]) + (yCount.m256i_u32[2] + yCount.m256i_u32[3]);
        result += (yCount.m256i_u32[4] + yCount.m256i_u32[5]) + (yCount.m256i_u32[6] + yCount.m256i_u32[7]);
#elif USE_SSE2
        xCount    = _mm_setzero_si128();
        xResult0  = _mm_setzero_si128();
        xY0High   = get_logo_y(ptr_src, logo_width, 0, -1);
        xY0Middle = get_logo_y(ptr_src, logo_width, 0,  0);
        xY0Lower  = get_logo_y(ptr_src, logo_width, 0,  1);
        for (int x = 0; x < x_count; x += 8, ptr_src += 8, logo_mask_ptr += 8) {
            xY1High   = get_logo_y(ptr_src, logo_width, 8, -1);
            xY1Middle = get_logo_y(ptr_src, logo_width, 8,  0);
            xY1Lower  = get_logo_y(ptr_src, logo_width, 8,  1);
            xResult1  = prewitt_filter_simd(xY0High, xY1High, xY0Middle, xY1Middle, xY0Lower, xY1Lower);
            xTemp = _mm_alignr_epi8_simd(xResult1, xResult0, 14);

            xMask = _mm_load_si128((__m128i *)logo_mask_ptr);
            xMask = _mm_cmpgt_epi16(xMask, _mm_set1_epi16(1024));

            xTemp = _mm_and_si128(xTemp, xMask);

            xCount = _mm_add_epi32(xCount, _mm_unpacklo_epi16(xTemp, _mm_setzero_si128()));
            xCount = _mm_add_epi32(xCount, _mm_unpackhi_epi16(xTemp, _mm_setzero_si128()));

            xY0High   = xY1High;
            xY0Middle = xY1Middle;
            xY0Lower  = xY1Lower;
            xResult0  = xResult1;
        }
        //最後の切れ端
        //常にアライメントがとれているため、最後はゼロをロードしておけば良い
        xY1High   = _mm_setzero_si128();
        xY1Middle = _mm_setzero_si128();
        xY1Lower  = _mm_setzero_si128();

        //あとは普通に計算
        xResult1  = prewitt_filter_simd(xY0High, xY1High, xY0Middle, xY1Middle, xY0Lower, xY1Lower);
        xTemp = _mm_alignr_epi8_simd(xResult1, xResult0, 14);

        xMask = _mm_load_si128((__m128i *)logo_mask_ptr);
        xMask = _mm_cmpgt_epi16(xMask, _mm_set1_epi16(1024));

        xTemp = _mm_and_si128(xTemp, xMask);

        //32bitに拡張してから総和演算
        xCount = _mm_add_epi32(xCount, _mm_unpacklo_epi16(xTemp, _mm_setzero_si128()));
        xCount = _mm_add_epi32(xCount, _mm_unpackhi_epi16(xTemp, _mm_setzero_si128()));

        //総和演算
        result += (xCount.m128i_u32[0] + xCount.m128i_u32[1]) + (xCount.m128i_u32[2] + xCount.m128i_u32[3]);
#else
        ptr_src++;
        logo_mask_ptr++;
        for (int x = 1; x < x_count - 1; x++, ptr_src++, logo_mask_ptr++) {
            if (*logo_mask_ptr > 1024) {
                result += prewitt_filter(ptr_src, logo_width);
            }
        }
#endif
        if (result > auto_result) {
            // これ以上調査しても意味がないのでやめる
            break;
        }
    }
#if USE_AVX2
    _mm256_zeroupper();
#endif
    return result;
}

#pragma warning(push)
#pragma warning(disable:4189) //ローカル変数が初期化されましたが、参照されていません
template<bool store_result>
static __forceinline unsigned int func_proc_prewitt_5x5_evaluate_simd(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result) {
    const int logo_width = PITCH(lgh->w);
    if (logo_width < (USE_AVX2 ? 32 : 16) + 2)
        return (store_result) ? func_proc_prewitt_5x5_evaluate_store_c(result_buf, logo_mask, src, lgh, auto_result)
                              : func_proc_prewitt_5x5_evaluate_c(result_buf, logo_mask, src, lgh, auto_result);

#define IF_STORE_RESULT(x) ((store_result) ? (x) : (0))
#if USE_AVX2
    __m256i yY0_0, yY0_1, yY1_0, yY1_1, yY2_0, yY2_1, yY3_0, yY3_1, yY4_0, yY4_1, yResult0, yResult1, yCount, yMask, yTemp;
#elif USE_SSE2
    __m128i xY0_0, xY0_1, xY1_0, xY1_1, xY2_0, xY2_1, xY3_0, xY3_1, xY4_0, xY4_1, xResult0, xResult1, xCount, xMask, xTemp;
#endif
    const short *src_line = src + logo_width * 2;
    const short *logo_mask_line = logo_mask + logo_width * 2;
    const short *result_buf_line = IF_STORE_RESULT(result_buf + logo_width * 2);
    const int y_fin = lgh->h - 2;
    int x_count = logo_width;
#if USE_SSE2 || USE_AVX2
    //SIMD版では、範囲外アクセス防止の為、最後の部分のみ特別に処理する
    x_count -= (USE_AVX2 ? 16 : 8);
#endif
    DWORD result = 0;
    for (int y = 2; y < y_fin; y++, src_line += logo_width, logo_mask_line += logo_width, result_buf_line += IF_STORE_RESULT(logo_width)) {
        const short *ptr_src = src_line;
        const short *logo_mask_ptr = logo_mask_line;
        const short *result_buf_ptr = IF_STORE_RESULT(result_buf_line);
#if USE_AVX2
        yCount   = _mm256_setzero_si256();
        yResult0 = _mm256_setzero_si256();
        yY0_0 = get_logo_y(ptr_src, logo_width, 0, -2);
        yY1_0 = get_logo_y(ptr_src, logo_width, 0, -1);
        yY2_0 = get_logo_y(ptr_src, logo_width, 0,  0);
        yY3_0 = get_logo_y(ptr_src, logo_width, 0,  1);
        yY4_0 = get_logo_y(ptr_src, logo_width, 0,  2);
        for (int x = 0; x < x_count; x += 16, ptr_src += 16, logo_mask_ptr += 16, result_buf_ptr += IF_STORE_RESULT(16)) {
            yY0_1 = get_logo_y(ptr_src, logo_width, 16, -2);
            yY1_1 = get_logo_y(ptr_src, logo_width, 16, -1);
            yY2_1 = get_logo_y(ptr_src, logo_width, 16,  0);
            yY3_1 = get_logo_y(ptr_src, logo_width, 16,  1);
            yY4_1 = get_logo_y(ptr_src, logo_width, 16,  2);
            yResult1  = prewitt_filter_5x5_simd(yY0_0, yY0_1, yY1_0, yY1_1, yY2_0, yY2_1, yY3_0, yY3_1, yY4_0, yY4_1);
            yTemp = _mm256_alignr256_epi8(yResult1, yResult0, (32-4));

            yMask = _mm256_load_si256((__m256i *)logo_mask_ptr);
            yMask = _mm256_cmpgt_epi16(yMask, _mm256_set1_epi16(1024));

            yTemp = _mm256_and_si256(yTemp, yMask);

            yCount = _mm256_add_epi32(yCount, _mm256_unpacklo_epi16(yTemp, _mm256_setzero_si256()));
            yCount = _mm256_add_epi32(yCount, _mm256_unpackhi_epi16(yTemp, _mm256_setzero_si256()));

            IF_STORE_RESULT(_mm256_store_si256((__m256i *)result_buf_ptr, yTemp));

            yY0_0 = yY0_1;
            yY1_0 = yY1_1;
            yY2_0 = yY2_1;
            yY3_0 = yY3_1;
            yY4_0 = yY4_1;
            yResult0  = yResult1;
        }
        //最後の切れ端
        //常にアライメントがとれているため、最後はゼロをロードしておけば良い
        yY0_1 = _mm256_setzero_si256();
        yY1_1 = _mm256_setzero_si256();
        yY2_1 = _mm256_setzero_si256();
        yY3_1 = _mm256_setzero_si256();
        yY4_1 = _mm256_setzero_si256();

        //あとは普通に計算
        yResult1  = prewitt_filter_5x5_simd(yY0_0, yY0_1, yY1_0, yY1_1, yY2_0, yY2_1, yY3_0, yY3_1, yY4_0, yY4_1);
        yTemp = _mm256_alignr256_epi8(yResult1, yResult0, (32-4));

        yMask = _mm256_load_si256((__m256i *)logo_mask_ptr);
        yMask = _mm256_cmpgt_epi16(yMask, _mm256_set1_epi16(1024));

        yTemp = _mm256_and_si256(yTemp, yMask);

        //32bitに拡張してから総和演算
        yCount = _mm256_add_epi32(yCount, _mm256_unpacklo_epi16(yTemp, _mm256_setzero_si256()));
        yCount = _mm256_add_epi32(yCount, _mm256_unpackhi_epi16(yTemp, _mm256_setzero_si256()));

        IF_STORE_RESULT(_mm256_store_si256((__m256i *)result_buf_ptr, yTemp));

        __m128i xCount = _mm256_extracti128_si256(yCount, 1);
        xCount = _mm_hadd_epi32(xCount, _mm256_castsi256_si128(yCount));
        xCount = _mm_hadd_epi32(xCount, xCount);
        result += (xCount.m128i_u32[0] + xCount.m128i_u32[1]);
#elif USE_SSE2
        xCount    = _mm_setzero_si128();
        xResult0  = _mm_setzero_si128();
        xY0_0 = get_logo_y(ptr_src, logo_width, 0, -2);
        xY1_0 = get_logo_y(ptr_src, logo_width, 0, -1);
        xY2_0 = get_logo_y(ptr_src, logo_width, 0,  0);
        xY3_0 = get_logo_y(ptr_src, logo_width, 0,  1);
        xY4_0 = get_logo_y(ptr_src, logo_width, 0,  2);
        for (int x = 0; x < x_count; x += 8, ptr_src += 8, logo_mask_ptr += 8, result_buf_ptr += IF_STORE_RESULT(8)) {
            xY0_1 = get_logo_y(ptr_src, logo_width, 8, -2);
            xY1_1 = get_logo_y(ptr_src, logo_width, 8, -1);
            xY2_1 = get_logo_y(ptr_src, logo_width, 8,  0);
            xY3_1 = get_logo_y(ptr_src, logo_width, 8,  1);
            xY4_1 = get_logo_y(ptr_src, logo_width, 8,  2);
            xResult1  = prewitt_filter_5x5_simd(xY0_0, xY0_1, xY1_0, xY1_1, xY2_0, xY2_1, xY3_0, xY3_1, xY4_0, xY4_1);
            xTemp = _mm_alignr_epi8_simd(xResult1, xResult0, (16-4));

            xMask = _mm_load_si128((__m128i *)logo_mask_ptr);
            xMask = _mm_cmpgt_epi16(xMask, _mm_set1_epi16(1024));

            xTemp = _mm_and_si128(xTemp, xMask);

            xCount = _mm_add_epi32(xCount, _mm_unpacklo_epi16(xTemp, _mm_setzero_si128()));
            xCount = _mm_add_epi32(xCount, _mm_unpackhi_epi16(xTemp, _mm_setzero_si128()));

            IF_STORE_RESULT(_mm_store_si128((__m128i *)result_buf_ptr, xTemp));

            xY0_0 = xY0_1;
            xY1_0 = xY1_1;
            xY2_0 = xY2_1;
            xY3_0 = xY3_1;
            xY4_0 = xY4_1;
            xResult0 = xResult1;
        }
        //最後の切れ端
        //常にアライメントがとれているため、最後はゼロをロードしておけば良い
        xY0_1 = _mm_setzero_si128();
        xY1_1 = _mm_setzero_si128();
        xY2_1 = _mm_setzero_si128();
        xY3_1 = _mm_setzero_si128();
        xY4_1 = _mm_setzero_si128();

        //あとは普通に計算
        xResult1  = prewitt_filter_5x5_simd(xY0_0, xY0_1, xY1_0, xY1_1, xY2_0, xY2_1, xY3_0, xY3_1, xY4_0, xY4_1);
        xTemp = _mm_alignr_epi8_simd(xResult1, xResult0, (16-4));

        xMask = _mm_load_si128((__m128i *)logo_mask_ptr);
        xMask = _mm_cmpgt_epi16(xMask, _mm_set1_epi16(1024));

        xTemp = _mm_and_si128(xTemp, xMask);

        //32bitに拡張してから総和演算
        xCount = _mm_add_epi32(xCount, _mm_unpacklo_epi16(xTemp, _mm_setzero_si128()));
        xCount = _mm_add_epi32(xCount, _mm_unpackhi_epi16(xTemp, _mm_setzero_si128()));

        IF_STORE_RESULT(_mm_store_si128((__m128i *)result_buf_ptr, xTemp));

        //総和演算
#if 0 //USE_SSSE3
        xCount = _mm_hadd_epi32(xCount, xCount);
        result += (xCount.m128i_u32[0] + xCount.m128i_u32[1]);
#else
        result += (xCount.m128i_u32[0] + xCount.m128i_u32[1]) + (xCount.m128i_u32[2] + xCount.m128i_u32[3]);
#endif
#else
        ptr_src += 2;
        logo_mask_ptr += 2;
        for (int x = 2; x < x_count - 2; x++, ptr_src++, logo_mask_ptr++) {
            if (*logo_mask_ptr > 1024) { // Mask値が1024を超える領域(=Logoの輪郭部)のみを評価する
                result += prewitt_filter_5x5(ptr_src, logo_width);
            }
        }
#endif
        if (result > auto_result) {
            // これ以上調査しても意味がないのでやめる
            break;
        }
    }
#if USE_AVX2
    _mm256_zeroupper();
#endif
    return result;
}
#pragma warning(pop)	/* 4189 */

#pragma warning(push)
#pragma warning(disable:4100) //引数は関数の本体部で 1 度も参照されません。
static __forceinline int count_logo_valid_pixels_simd(const short *mask, int width, int logo_pitch, int height) {
    int count = 0;
    const int y_fin = height - 1;
    mask += logo_pitch;
#if USE_AVX2
    const __m256i yThreshold = _mm256_set1_epi16(1024);
#elif USE_SSE2
    const __m128i xThreshold = _mm_set1_epi16(1024);
#endif
    for (int y = 1; y < y_fin; y++, mask += logo_pitch) {
        const short *ptr = mask;
#if USE_AVX2
        for (int x_count = logo_pitch >> 5; x_count; x_count--, ptr += 32) {
            __m256i y0 = _mm256_loadu_si256((__m256i *)(ptr +  0));
            __m256i y1 = _mm256_loadu_si256((__m256i *)(ptr + 16));
            y0 = _mm256_cmpgt_epi16(y0, yThreshold);
            y1 = _mm256_cmpgt_epi16(y1, yThreshold);
            y0 = _mm256_packs_epi16(y0, y1);
            count += _mm_popcnt_u32(_mm256_movemask_epi8(y0));
        }
        if (logo_pitch & 16) {
            __m256i y0 = _mm256_loadu_si256((__m256i *)(ptr +  0));
            y0 = _mm256_cmpgt_epi16(y0, yThreshold);
            y0 = _mm256_packs_epi16(y0, _mm256_setzero_si256());
            count += _mm_popcnt_u32(_mm256_movemask_epi8(y0));
        }
#elif USE_SSE2
        for (int x_count = logo_pitch >> 5; x_count; x_count--, ptr += 32) {
            __m128i x0 = _mm_loadu_si128((__m128i *)(ptr +  0));
            __m128i x1 = _mm_loadu_si128((__m128i *)(ptr +  8));
            __m128i x2 = _mm_loadu_si128((__m128i *)(ptr + 16));
            __m128i x3 = _mm_loadu_si128((__m128i *)(ptr + 24));
            x0 = _mm_cmpgt_epi16(x0, xThreshold);
            x1 = _mm_cmpgt_epi16(x1, xThreshold);
            x2 = _mm_cmpgt_epi16(x2, xThreshold);
            x3 = _mm_cmpgt_epi16(x3, xThreshold);
            x0 = _mm_packs_epi16(x0, x1);
            x2 = _mm_packs_epi16(x2, x3);
            DWORD count0 = _mm_movemask_epi8(x0);
            DWORD count1 = _mm_movemask_epi8(x2);
            count += popcnt32(count0 << 16 | count1);
        }
        if (logo_pitch & 16) {
            __m128i x0 = _mm_loadu_si128((__m128i *)(ptr +  0));
            __m128i x1 = _mm_loadu_si128((__m128i *)(ptr +  8));
            x0 = _mm_cmpgt_epi16(x0, xThreshold);
            x1 = _mm_cmpgt_epi16(x1, xThreshold);
            x0 = _mm_packs_epi16(x0, x1);
            count += popcnt32(_mm_movemask_epi8(x0));
        }
#else
        const int x_fin = width - 1;
        for (int x = 1; x < x_fin; x++) {
            count += (ptr[x] > 1024);
        }
#endif
    }
#if USE_AVX2
    _mm256_zeroupper();
#endif
    return count;
}
#pragma warning(pop)	/* 4100 */
#pragma warning(pop)	/* 4752 */
