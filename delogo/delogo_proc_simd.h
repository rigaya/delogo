#pragma once
#include <Windows.h>
#include "filter.h"
#include "logo.h"
#include "logodef.h"
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


#if USE_AVX2
#define MEM_ALIGN 32
#else
#define MEM_ALIGN 16
#endif

#if USE_AVX2
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
#else
#define Clamp(n,l,h) ((n<l) ? l : (n>h) ? h : n)
#endif
/*--------------------------------------------------------------------
* 	func_proc_eraze_logo()	ロゴ除去モード
*-------------------------------------------------------------------*/
static __forceinline BOOL func_proc_eraze_logo_simd(FILTER *fp, FILTER_PROC_INFO *fpip, LOGO_HEADER *lgh, int fade) {
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
    const int y_fin = min(fpip->h - lgh->y, lgh->h);
    const int x_fin = min(fpip->w - lgh->x, lgh->w);
    static const int Y_MIN = -128, Y_MAX = 4096+128;
    static const int C_MIN = -128-2048, C_MAX = 128+2048;

    static const __declspec(align(MEM_ALIGN)) USHORT MASK_16BIT[] = {
        0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000,
#if USE_AVX2
        0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000
#endif
    };
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
