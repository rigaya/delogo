#pragma once
#include "delogo_proc.h"
#include "delogo_proc_simd.h"

#pragma warning (push)
#pragma warning (disable: 4752) // Intel(R) AVX 命令が見つかりました。/arch:AVX を使用することを検討してください

#define RANGE_DIV(range) ((float)((2*(range)+1)*(2*(range)+1)) / ((range >= 4) ? 2 : 1))
#define IS_YC48 (sizeof(TYPE) == sizeof(PIXEL_YC))

#if USE_AVX2
//スムーンジング用に水平方向の16bit加算を行う
static __forceinline __m256i smooth_3x3_horizontal(__m256i y0, __m256i y1, __m256i y2) {
    __m256i ySum = y1;
    ySum = _mm256_adds_epi16(ySum, _mm256_adds_epi16(_mm256_alignr256_epi8(y1, y0, (32-2)),
                                                     _mm256_alignr256_epi8(y2, y1, 2)));
    return ySum;
}
static __forceinline __m256i smooth_5x5_horizontal(__m256i y0, __m256i y1, __m256i y2) {
    __m256i ySum = y1;
    ySum = _mm256_adds_epi16(ySum, _mm256_adds_epi16(_mm256_alignr256_epi8(y1, y0, (32-4)),
                                                     _mm256_alignr256_epi8(y1, y0, (32-2))));
    ySum = _mm256_adds_epi16(ySum, _mm256_adds_epi16(_mm256_alignr256_epi8(y2, y1, 2),
                                                     _mm256_alignr256_epi8(y2, y1, 4)));
    return ySum;
}
static __forceinline __m256i smooth_7x7_horizontal(__m256i y0, __m256i y1, __m256i y2) {
    __m256i ySum = y1;
    ySum = _mm256_adds_epi16(ySum, _mm256_adds_epi16(_mm256_alignr256_epi8(y1, y0, (32-6)),
                                                     _mm256_alignr256_epi8(y1, y0, (32-4))));
    ySum = _mm256_adds_epi16(ySum, _mm256_adds_epi16(_mm256_alignr256_epi8(y1, y0, (32-2)),
                                                     _mm256_alignr256_epi8(y2, y1, 2)));
    ySum = _mm256_adds_epi16(ySum, _mm256_adds_epi16(_mm256_alignr256_epi8(y2, y1, 4),
                                                     _mm256_alignr256_epi8(y2, y1, 6)));
    return ySum;
}
//水平方向の加算を行うが、1/2した値を格納する
static __forceinline __m256i smooth_9x9_horizontal(__m256i y0, __m256i y1, __m256i y2) {
    __m256i ySum0 = y1, ySum1, ySumHi, ySumLo;
    ySum0 = _mm256_adds_epi16(ySum0, _mm256_adds_epi16(_mm256_alignr256_epi8(y1, y0, (32-8)),
                                                       _mm256_alignr256_epi8(y1, y0, (32-6))));
    ySum0 = _mm256_adds_epi16(ySum0, _mm256_adds_epi16(_mm256_alignr256_epi8(y1, y0, (32-4)),
                                                       _mm256_alignr256_epi8(y1, y0, (32-2))));
    ySum1 =                          _mm256_adds_epi16(_mm256_alignr256_epi8(y2, y1, 2),
                                                       _mm256_alignr256_epi8(y2, y1, 4));
    ySum1 = _mm256_adds_epi16(ySum1, _mm256_adds_epi16(_mm256_alignr256_epi8(y2, y1, 6),
                                                       _mm256_alignr256_epi8(y2, y1, 8)));

    ySumLo = cvtlo256_epi16_epi32(ySum0);
    ySumHi = cvthi256_epi16_epi32(ySum0);
    ySumLo = _mm256_add_epi32(ySumLo, cvtlo256_epi16_epi32(ySum1));
    ySumHi = _mm256_add_epi32(ySumHi, cvthi256_epi16_epi32(ySum1));
    ySumLo = _mm256_srai_epi32(ySumLo, 1);
    ySumHi = _mm256_srai_epi32(ySumHi, 1);

    return _mm256_packs_epi32(ySumLo, ySumHi);
}

//y0,y1の各32bit整数値に対し、rangeに応じて割り算を行い、結果を16bitに格納して返す
template<int range>
static __forceinline __m256i div_and_pack_epi32(__m256i y0, __m256i y1) {
    y0 = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_cvtepi32_ps(y0), _mm256_set1_ps(1.0f/RANGE_DIV(range))));
    y1 = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_cvtepi32_ps(y1), _mm256_set1_ps(1.0f/RANGE_DIV(range))));
    return _mm256_packs_epi32(y0, y1);
}

//rangeに応じてスムージング用の水平加算を行う
template<unsigned int range>
static __forceinline __m256i smooth_horizontal(__m256i y0, __m256i y1, __m256i y2) {
    switch (range) {
    case 4: return smooth_9x9_horizontal(y0, y1, y2);
    case 3: return smooth_7x7_horizontal(y0, y1, y2);
    case 2: return smooth_5x5_horizontal(y0, y1, y2);
    case 1:
    default:return smooth_3x3_horizontal(y0, y1, y2);
    }
}

static const USHORT __declspec(align(32)) MASK[32] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
};


static const BYTE   __declspec(align(32)) Array_SUFFLE_YCP_Y[]      = {0, 1, 6, 7, 12, 13, 2, 3, 8, 9, 14, 15, 4, 5, 10, 11, 0, 1, 6, 7, 12, 13, 2, 3, 8, 9, 14, 15, 4, 5, 10, 11};
static const USHORT __declspec(align(32)) Array_MASK_YCP_SELECT_Y[] = {0xFFFF, 0x0000, 0x0000, 0xFFFF, 0x0000, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0x0000, 0xFFFF, 0x0000, 0x0000, 0xFFFF, 0x0000};
#define SUFFLE_YCP_Y       _mm256_load_si256((__m256i*)Array_SUFFLE_YCP_Y)
#define MASK_YCP_SELECT_Y  _mm256_load_si256((__m256i*)Array_MASK_YCP_SELECT_Y)

//YC48から輝度値を抽出してレジスタに格納する
static __forceinline __m256i get_y_from_pixelyc(BYTE *src) {
    __m256i y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(src + 48)), _mm_loadu_si128((__m128i *)(src +  0)));
    __m256i y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(src + 64)), _mm_loadu_si128((__m128i *)(src + 16)));
    __m256i y2 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(src + 80)), _mm_loadu_si128((__m128i *)(src + 32)));
    const int MASK_INT = 0x40 + 0x08 + 0x01;
    y2 = _mm256_blend_epi16(y2, y0, MASK_INT);
    y2 = _mm256_blend_epi16(y2, y1, MASK_INT<<1);
    y2 = _mm256_shuffle_epi8(y2, SUFFLE_YCP_Y);
    return y2;
}

//ptrで指定した対象YC48に輝度値を挿入する
static __forceinline void store_y_to_yc48(BYTE *ptr, __m256i yY) {
    __m256i y0, y1, y2;
    y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr + 48)), _mm_loadu_si128((__m128i *)(ptr +  0))); // 384,   0
    y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr + 64)), _mm_loadu_si128((__m128i *)(ptr + 16))); // 512, 128
    y2 = _mm256_set_m128i(_mm_loadu_si128((__m128i *)(ptr + 80)), _mm_loadu_si128((__m128i *)(ptr + 32))); // 768, 256

    const int MASK_INT = 0x40 + 0x08 + 0x01;
    yY = _mm256_shuffle_epi8(yY, SUFFLE_YCP_Y);
    y0 = _mm256_blend_epi16(y0, yY, MASK_INT);
    y1 = _mm256_blend_epi16(y1, yY, MASK_INT<<1);
    y2 = _mm256_blend_epi16(y2, yY, (MASK_INT<<2) & 0xFF);

    _mm256_storeu_si256((__m256i *)(ptr +  0), _mm256_permute2x128_si256(y0, y1, (0x02<<4)+0x00)); // 128,   0
    _mm256_storeu_si256((__m256i *)(ptr + 32), _mm256_blend_epi32(       y0, y2, (0x00<<4)+0x0f)); // 384, 256
    _mm256_storeu_si256((__m256i *)(ptr + 64), _mm256_permute2x128_si256(y1, y2, (0x03<<4)+0x01)); // 768, 512
}
#pragma warning (pop)	/* 4752 */

//YC48モードかどうかを見分けて適切に輝度値をロードする
#define LOAD_256BIT(ptr)      ((sizeof(TYPE) == sizeof(PIXEL_YC)) ? get_y_from_pixelyc((BYTE *)(ptr))    : _mm256_loadu_si256((__m256i *)(ptr)))

//YC48モードかどうかを見分けて適切に輝度値をptrの位置に格納する
#define STORE_256BIT(ptr, yY) ((sizeof(TYPE) == sizeof(PIXEL_YC)) ? store_y_to_yc48((BYTE *)(ptr), (yY)) : _mm256_storeu_si256((__m256i *)(ptr), (yY)))

//スムージングでは、まず水平方向の加算結果をバッファに格納していく
//この関数は1ラインぶんの水平方向の加算 + バッファへの格納のみを行う
template<unsigned int range, class TYPE>
static __forceinline void smooth_fill_buffer(short *buf_ptr, const TYPE *src_ptr, int width, int x_fin) {
    __m256i y0 = _mm256_set1_epi16(*(short *)src_ptr);
    __m256i y1 = LOAD_256BIT(src_ptr);
    __m256i y2;
    for (int x = x_fin; x; x -= 16, src_ptr += 16, buf_ptr += 16) {
        y2 = LOAD_256BIT(src_ptr + 16);
        _mm256_storeu_si256((__m256i *)buf_ptr, smooth_horizontal<range>(y0, y1, y2));

        y0 = y1;
        y1 = y2;
    }
    y2 = _mm256_set1_epi16(*(short *)(src_ptr - x_fin + width - 1));
    y1 = _mm256_blendv_epi8(y1, y2, _mm256_loadu_si256((__m256i *)&MASK[16-(width & 15)]));
    _mm256_storeu_si256((__m256i *)buf_ptr, smooth_horizontal<range>(y0, y1, y2));
}

//バッファからbuf_ptr + offsetで指定した16bit整数値を読み取り、xLo + xHi の32bit整数に分けて加算する
static __forceinline void add_to_sum_lo_hi(__m256i& xLo, __m256i& xHi, const short *buf_ptr, int offset) {
    __m256i xTemp  = _mm256_load_si256((__m256i *)(buf_ptr + offset));
    xLo = _mm256_add_epi32(xLo, cvtlo256_epi16_epi32(xTemp));
    xHi = _mm256_add_epi32(xHi, cvthi256_epi16_epi32(xTemp));
}

//バッファのライン数によるオフセットを計算する
#define BUF_LINE_OFFSET(x) (((x) & (buf_line - 1)) * line_size)

#pragma warning (push)
#pragma warning (disable:4127) //warning C4127: 条件式が定数です。
//yNewLineResultの最新のラインの水平加算結果と、バッファに格納済みの水平加算結果を用いて、
//縦方向の加算を行い、スムージング結果を16bit整数に格納して返す。
//yNewLineResultの値は、新たにバッファに格納される
template<unsigned int range, int buf_line, int line_size>
static __forceinline __m256i smooth_vertical(short *buf_ptr, __m256i yNewLineResult, int y) {
    __m256i ySum0  = cvtlo256_epi16_epi32(yNewLineResult);
    __m256i ySum1  = cvthi256_epi16_epi32(yNewLineResult);
    if (range >= 1) {
        add_to_sum_lo_hi(ySum0, ySum1, buf_ptr, BUF_LINE_OFFSET(y+0));
        add_to_sum_lo_hi(ySum0, ySum1, buf_ptr, BUF_LINE_OFFSET(y+1));
    }
    _mm256_storeu_si256((__m256i *)(buf_ptr + BUF_LINE_OFFSET(y+range*2)), yNewLineResult);
    if (range >= 2) {
        add_to_sum_lo_hi(ySum0, ySum1, buf_ptr, BUF_LINE_OFFSET(y+2));
        add_to_sum_lo_hi(ySum0, ySum1, buf_ptr, BUF_LINE_OFFSET(y+3));
    }
    if (range >= 3) {
        add_to_sum_lo_hi(ySum0, ySum1, buf_ptr, BUF_LINE_OFFSET(y+4));
        add_to_sum_lo_hi(ySum0, ySum1, buf_ptr, BUF_LINE_OFFSET(y+5));
    }
    if (range >= 4) {
        add_to_sum_lo_hi(ySum0, ySum1, buf_ptr, BUF_LINE_OFFSET(y+6));
        add_to_sum_lo_hi(ySum0, ySum1, buf_ptr, BUF_LINE_OFFSET(y+7));
    }
    return div_and_pack_epi32<range>(ySum0, ySum1);
}
#pragma warning (pop)

#elif USE_SSE2
//スムーンジング用に水平方向の16bit加算を行う
static __forceinline __m128i smooth_3x3_horizontal(__m128i x0, __m128i x1, __m128i x2) {
    __m128i xSum = x1;
    xSum = _mm_adds_epi16(xSum, _mm_adds_epi16(_mm_alignr_epi8_simd(x1, x0, (16-2)),
                                               _mm_alignr_epi8_simd(x2, x1, 2)));
    return xSum;
}
static __forceinline __m128i smooth_5x5_horizontal(__m128i x0, __m128i x1, __m128i x2) {
    __m128i xSum = x1;
    xSum = _mm_adds_epi16(xSum, _mm_adds_epi16(_mm_alignr_epi8_simd(x1, x0, (16-4)),
                                               _mm_alignr_epi8_simd(x1, x0, (16-2))));
    xSum = _mm_adds_epi16(xSum, _mm_adds_epi16(_mm_alignr_epi8_simd(x2, x1, 2),
                                               _mm_alignr_epi8_simd(x2, x1, 4)));
    return xSum;
}
static __forceinline __m128i smooth_7x7_horizontal(__m128i x0, __m128i x1, __m128i x2) {
    __m128i xSum = x1;
    xSum = _mm_adds_epi16(xSum, _mm_adds_epi16(_mm_alignr_epi8_simd(x1, x0, (16-6)),
                                               _mm_alignr_epi8_simd(x1, x0, (16-4))));
    xSum = _mm_adds_epi16(xSum, _mm_adds_epi16(_mm_alignr_epi8_simd(x1, x0, (16-2)),
                                               _mm_alignr_epi8_simd(x2, x1, 2)));
    xSum = _mm_adds_epi16(xSum, _mm_adds_epi16(_mm_alignr_epi8_simd(x2, x1, 4),
                                               _mm_alignr_epi8_simd(x2, x1, 6)));
    return xSum;
}
//水平方向の加算を行うが、1/2した値を格納する
static __forceinline __m128i smooth_9x9_horizontal(__m128i x0, __m128i x1, __m128i x2) {
    __m128i xSum0 = x1, xSum1, xSumHi, xSumLo;
    xSum0 = _mm_adds_epi16(xSum0, _mm_adds_epi16(_mm_alignr_epi8_simd(x1, x0, (16-8)),
                                                 _mm_alignr_epi8_simd(x1, x0, (16-6))));
    xSum0 = _mm_adds_epi16(xSum0, _mm_adds_epi16(_mm_alignr_epi8_simd(x1, x0, (16-4)),
                                                 _mm_alignr_epi8_simd(x1, x0, (16-2))));
    xSum1 =                       _mm_adds_epi16(_mm_alignr_epi8_simd(x2, x1, 2),
                                                 _mm_alignr_epi8_simd(x2, x1, 4));
    xSum1 = _mm_adds_epi16(xSum1, _mm_adds_epi16(_mm_alignr_epi8_simd(x2, x1, 6),
                                                 _mm_alignr_epi8_simd(x2, x1, 8)));

    xSumLo = cvtlo_epi16_epi32(xSum0);
    xSumHi = cvthi_epi16_epi32(xSum0);
    xSumLo = _mm_add_epi32(xSumLo, cvtlo_epi16_epi32(xSum1));
    xSumHi = _mm_add_epi32(xSumHi, cvthi_epi16_epi32(xSum1));
    xSumLo = _mm_srai_epi32(xSumLo, 1);
    xSumHi = _mm_srai_epi32(xSumHi, 1);

    return _mm_packs_epi32(xSumLo, xSumHi);
}

//x0,x1の各32bit整数値に対し、rangeに応じて割り算を行い、結果を16bitに格納して返す
template<int range>
static __forceinline __m128i div_and_pack_epi32(__m128i x0, __m128i x1) {
    x0  = _mm_cvtps_epi32(_mm_mul_ps(_mm_cvtepi32_ps(x0), _mm_set1_ps(1.0f/RANGE_DIV(range))));
    x1  = _mm_cvtps_epi32(_mm_mul_ps(_mm_cvtepi32_ps(x1), _mm_set1_ps(1.0f/RANGE_DIV(range))));
    return _mm_packs_epi32(x0, x1);
}

//rangeに応じてスムージング用の水平加算を行う
template<unsigned int range>
static __forceinline __m128i smooth_horizontal(__m128i x0, __m128i x1, __m128i x2) {
    switch (range) {
    case 4: return smooth_9x9_horizontal(x0, x1, x2);
    case 3: return smooth_7x7_horizontal(x0, x1, x2);
    case 2: return smooth_5x5_horizontal(x0, x1, x2);
    case 1:
    default:return smooth_3x3_horizontal(x0, x1, x2);
    }
}

static const USHORT __declspec(align(16)) MASK[16] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};

static inline __m128i shuffle_ycp_selected_y_sse2(__m128i x0) {
    x0 = _mm_shuffle_epi32(  x0, _MM_SHUFFLE(3,1,2,0));
    x0 = _mm_shufflehi_epi16(x0, _MM_SHUFFLE(3,0,2,1));
    x0 = _mm_shuffle_epi32(  x0, _MM_SHUFFLE(3,1,2,0));
    x0 = _mm_shufflehi_epi16(x0, _MM_SHUFFLE(1,2,3,0));
    x0 = _mm_shufflelo_epi16(x0, _MM_SHUFFLE(1,3,2,0));
    return x0;
}

static const _declspec(align(16)) BYTE   Array_SUFFLE_YCP_Y[]      = {0, 1, 6, 7, 12, 13, 2, 3, 8, 9, 14, 15, 4, 5, 10, 11};
static const _declspec(align(16)) USHORT Array_MASK_YCP_SELECT_Y[] = {0xFFFF, 0x0000, 0x0000, 0xFFFF, 0x0000, 0x0000, 0xFFFF, 0x0000};
#define SUFFLE_YCP_Y       _mm_load_si128((__m128i*)Array_SUFFLE_YCP_Y)
#define MASK_YCP_SELECT_Y  _mm_load_si128((__m128i*)Array_MASK_YCP_SELECT_Y)

//YC48から輝度値を抽出してレジスタに格納する
static __forceinline __m128i get_y_from_pixelyc(BYTE *src) {
    __m128i x0 = _mm_loadu_si128((__m128i *)(src +  0));
    __m128i x1 = _mm_loadu_si128((__m128i *)(src + 16));
    __m128i x2 = _mm_loadu_si128((__m128i *)(src + 32));
#undef _mm_switch_load_si128
#if USE_SSE41
    const int MASK_INT = 0x40 + 0x08 + 0x01;
    x2 = _mm_blend_epi16(x2, x0, MASK_INT);
    x2 = _mm_blend_epi16(x2, x1, MASK_INT<<1);
#else
    x2 = blendv_epi8_simd(x2, x0, MASK_YCP_SELECT_Y);
    x2 = blendv_epi8_simd(x2, x1, _mm_slli_si128(MASK_YCP_SELECT_Y, 2));
#endif
#if USE_SSSE3
    x2 = _mm_shuffle_epi8(x2, SUFFLE_YCP_Y);
#else
    x2 = shuffle_ycp_selected_y_sse2(x2);
#endif
    return x2;
}

//ptrで指定した対象YC48に輝度値を挿入する
static __forceinline void store_y_to_yc48(BYTE *ptr, __m128i xY) {
    __m128i x0, x1, x2;

    x0 = _mm_loadu_si128((__m128i *)(ptr +  0));
    x1 = _mm_loadu_si128((__m128i *)(ptr + 16));
    x2 = _mm_loadu_si128((__m128i *)(ptr + 32));
#if USE_SSE41
    const int MASK_INT = 0x40 + 0x08 + 0x01;
    xY = _mm_shuffle_epi8(xY, SUFFLE_YCP_Y);
    x0 = _mm_blend_epi16(x0, xY, MASK_INT);
    x1 = _mm_blend_epi16(x1, xY, MASK_INT<<1);
    x2 = _mm_blend_epi16(x2, xY, (MASK_INT<<2) & 0xFF);
#elif USE_SSSE3
    xY = _mm_shuffle_epi8(xY, SUFFLE_YCP_Y);
    x0 = blendv_epi8_simd(x0, xY, MASK_YCP_SELECT_Y);
    x1 = blendv_epi8_simd(x1, xY, _mm_slli_si128(MASK_YCP_SELECT_Y, 2));
    x2 = blendv_epi8_simd(x2, xY, _mm_slli_si128(MASK_YCP_SELECT_Y, 4));
#else
    xY = shuffle_ycp_selected_y_sse2(xY);
    x0 = blendv_epi8_simd(x0, xY, MASK_YCP_SELECT_Y);
    x1 = blendv_epi8_simd(x1, xY, _mm_slli_si128(MASK_YCP_SELECT_Y, 2));
    x2 = blendv_epi8_simd(x2, xY, _mm_slli_si128(MASK_YCP_SELECT_Y, 4));
#endif
    _mm_storeu_si128((__m128i *)(ptr +  0), x0);
    _mm_storeu_si128((__m128i *)(ptr + 16), x1);
    _mm_storeu_si128((__m128i *)(ptr + 32), x2);
}

//YC48モードかどうかを見分けて適切に輝度値をロードする
#define LOAD_128BIT(ptr)      ((sizeof(TYPE) == sizeof(PIXEL_YC)) ? get_y_from_pixelyc((BYTE *)(ptr))    : _mm_loadu_si128((__m128i *)(ptr)))

//YC48モードかどうかを見分けて適切に輝度値をptrの位置に格納する
#define STORE_128BIT(ptr, xY) ((sizeof(TYPE) == sizeof(PIXEL_YC)) ? store_y_to_yc48((BYTE *)(ptr), (xY)) : _mm_storeu_si128((__m128i *)(ptr), (xY)))

//スムージングでは、まず水平方向の加算結果をバッファに格納していく
//この関数は1ラインぶんの水平方向の加算 + バッファへの格納のみを行う
template<unsigned int range, class TYPE>
static __forceinline void smooth_fill_buffer(short *buf_ptr, const TYPE *src_ptr, int width, int x_fin) {
    __m128i x1 = LOAD_128BIT(src_ptr);
    __m128i x0 = _mm_set1_epi16(*(short *)src_ptr);
    __m128i x2;
    for (int x = x_fin; x; x -= 8, src_ptr += 8, buf_ptr += 8) {
        x2 = LOAD_128BIT(src_ptr + 8);
        _mm_storeu_si128((__m128i *)buf_ptr, smooth_horizontal<range>(x0, x1, x2));

        x0 = x1;
        x1 = x2;
    }
    x2 = _mm_set1_epi16(*(short *)(src_ptr - x_fin + width - 1));
    x1 = _mm_blendv_epi8_simd(x1, x2, _mm_loadu_si128((__m128i *)&MASK[8-(width & 7)]));
    _mm_storeu_si128((__m128i *)buf_ptr, smooth_horizontal<range>(x0, x1, x2));
}

//バッファからbuf_ptr + offsetで指定した16bit整数値を読み取り、xLo + xHi の32bit整数に分けて加算する
static __forceinline void add_to_sum_lo_hi(__m128i& xLo, __m128i& xHi, const short *buf_ptr, int offset) {
    __m128i xTemp  = _mm_load_si128((__m128i *)(buf_ptr + offset));
    xLo = _mm_add_epi32(xLo, cvtlo_epi16_epi32(xTemp));
    xHi = _mm_add_epi32(xHi, cvthi_epi16_epi32(xTemp));
}

#define BUF_LINE_OFFSET(x) (((x) & (buf_line - 1)) * line_size)

#pragma warning (push)
#pragma warning (disable:4127) //warning C4127: 条件式が定数です。
template<unsigned int range, int buf_line, int line_size>
//yNewLineResultの最新のラインの水平加算結果と、バッファに格納済みの水平加算結果を用いて、
//縦方向の加算を行い、スムージング結果を16bit整数に格納して返す。
//yNewLineResultの値は、新たにバッファに格納される
static __forceinline __m128i smooth_vertical(short *buf_ptr, __m128i xNewLineResult, int y) {
    __m128i xSum0  = cvtlo_epi16_epi32(xNewLineResult);
    __m128i xSum1  = cvthi_epi16_epi32(xNewLineResult);
    if (range >= 1) {
        add_to_sum_lo_hi(xSum0, xSum1, buf_ptr, BUF_LINE_OFFSET(y+0));
        add_to_sum_lo_hi(xSum0, xSum1, buf_ptr, BUF_LINE_OFFSET(y+1));
    }
    _mm_storeu_si128((__m128i *)(buf_ptr + BUF_LINE_OFFSET(y+range*2)), xNewLineResult);
    if (range >= 2) {
        add_to_sum_lo_hi(xSum0, xSum1, buf_ptr, BUF_LINE_OFFSET(y+2));
        add_to_sum_lo_hi(xSum0, xSum1, buf_ptr, BUF_LINE_OFFSET(y+3));
    }
    if (range >= 3) {
        add_to_sum_lo_hi(xSum0, xSum1, buf_ptr, BUF_LINE_OFFSET(y+4));
        add_to_sum_lo_hi(xSum0, xSum1, buf_ptr, BUF_LINE_OFFSET(y+5));
    }
    if (range >= 4) {
        add_to_sum_lo_hi(xSum0, xSum1, buf_ptr, BUF_LINE_OFFSET(y+6));
        add_to_sum_lo_hi(xSum0, xSum1, buf_ptr, BUF_LINE_OFFSET(y+7));
    }
    return div_and_pack_epi32<range>(xSum0, xSum1);
}
#pragma warning (pop)

#endif

extern const FUNC_SMOOTH      smooth_c[];
extern const FUNC_SMOOTH_YC48 smooth_yc48_c[];

#pragma warning (push)
#pragma warning (disable:4127) //warning C4127: 条件式が定数です。
//スムージング関数のベース
//rangeとsrcの型により、様々な種類の関数を生成できる
template<unsigned int range, class TYPE>
static __forceinline void smooth_simd(short *dst, TYPE *src, const short *mask, int width, int src_pitch, int height) {
    const int logo_pitch = PITCH(width); //logo_pitch
#if USE_AVX2 || USE_SSE2
    static_assert(1 <= range && range <= 4, "rangeは1～4のあいだまでしかサポートされません。");
    //バッファサイズの都合上、128画素までのスムージングをサポート
    //それ以上のサイズの場合は、C言語版コードに回す
    const int line_size = 256;
    if (logo_pitch > line_size) {
        (IS_YC48) ? smooth_yc48_c[range-1]((PIXEL_YC *)src, mask, width, src_pitch, height)
                  : smooth_c[range-1](dst, (short *)src, mask, width, src_pitch, height);
        return;
    }

    //横方向のループ数は、AVX2(256bit)か128bitかによって異なる (logo_pitchとは異なる)
    const int x_fin = (((USE_AVX2) ? (width + 15) & ~ 15 : (width + 7) & ~7)) - ((USE_AVX2) ? 16 : 8);
    //最低限必要なバッファのライン数の決定、計算上2の乗数を使用する
    //最後の1ラインは一時的に重複して保持させる(必要な物を読んだところに上書きしていく)ため、
    //range4 (9x9)なら8ラインあればよい
    const int buf_line = (range >= 3 ? 8 : (range >= 2 ? 4 : 2));
    //水平方向の加算結果を保持するバッファ
    short __declspec(align(32)) buffer[buf_line * line_size];
    memset(buffer, 0, sizeof(buffer));

    //まずはじめにバッファを必要なところまで準備する
    smooth_fill_buffer<range>(buffer, src, width, x_fin);
    //0行目と同じものでバッファの1行目～range行目まで埋める
    for (int i = 1; i <= range; i++)
        memcpy(buffer + i * line_size, buffer, logo_pitch * sizeof(buffer[0]));

    //バッファのrange+1行目～range*2-1行目までを埋める (range*2行目はメインループ内でロードする)
    for (int i = 0; i < range-1; i++)
        smooth_fill_buffer<range>(buffer + (i + 1 + range) * line_size, src + (i + 1) * ((IS_YC48) ? src_pitch : logo_pitch), width, x_fin);

    //メインループ
#if USE_AVX2
    __m256i y0, y1, y2, yResult, ySrc, yMask;
#else
    __m128i x0, x1, x2, xResult, xSrc, xMask;
#endif
    int y = 0; //バッファのライン数のもととなるため、y=0で始めることは重要
    const int y_fin = height - range; //水平加算用に先読みするため、rangeに配慮してループの終わりを決める
    for (; y < y_fin; y++, dst += (IS_YC48) ? 0 : logo_pitch, src += (IS_YC48) ? src_pitch : logo_pitch, mask += logo_pitch) {
        short *dst_ptr = dst;
        TYPE *src_ptr = src;
        short *buf_ptr = buffer;
        const short *mask_ptr = mask;
        const int range_offset = range * ((IS_YC48) ? src_pitch : logo_pitch); //水平加算用に先読みする位置のオフセット YC48モードではsrc_pitchを使用する
#if USE_AVX2
        y0 = _mm256_set1_epi16(*(short *)(src_ptr + range_offset));
        y1 = LOAD_256BIT(src_ptr + range_offset);
        for (int x = x_fin; x; x -= 16, src_ptr += 16, dst_ptr += (IS_YC48) ? 0 : 16, buf_ptr += 16, mask_ptr += 16) {
            y2 = LOAD_256BIT(src_ptr + range_offset + 16);
            //連続するデータy0, y1, y2を使って水平方向の加算を行う
            yResult = smooth_horizontal<range>(y0, y1, y2);
            //yResultとバッファに格納されている水平方向の加算結果を合わせて
            //垂直方向の加算を行い、スムージングを完成させる
            //このループで得た水平加算結果はバッファに新たに格納される (不要になったものを上書き)
            yResult = smooth_vertical<range, buf_line, line_size>(buf_ptr, yResult, y);

            yMask = _mm256_loadu_si256((__m256i *)mask_ptr);
            ySrc = LOAD_256BIT(src_ptr);
            //YC48モードではsrcに直接上書きする (計算用のデータはバッファに先読みしてあるため問題ない)
            STORE_256BIT((IS_YC48) ? (void*)src_ptr : (void*)dst_ptr, _mm256_blendv_epi8(ySrc, yResult, _mm256_cmpgt_epi16(yMask, _mm256_set1_epi16(1024))));

            y0 = y1;
            y1 = y2;
        }
        y2 = _mm256_set1_epi16(*(short *)(src_ptr + range_offset - x_fin + width - 1));
        y1 = _mm256_blendv_epi8(y1, y2, _mm256_loadu_si256((__m256i *)&MASK[16-(width & 15)]));
        yResult = smooth_horizontal<range>(y0, y1, y2);
        yResult = smooth_vertical<range, buf_line, line_size>(buf_ptr, yResult, y);

        yMask = _mm256_loadu_si256((__m256i *)mask_ptr);
        ySrc = LOAD_256BIT(src_ptr);
        STORE_256BIT((IS_YC48) ? (void*)src_ptr : (void*)dst_ptr, _mm256_blendv_epi8(ySrc, yResult, _mm256_cmpgt_epi16(yMask, _mm256_set1_epi16(1024))));
    }
    //先読みできる分が終了したら、あとはバッファから読み込んで処理する
    //yとiの値に注意する
    for (int i = 1; i <= range; y++, i++, dst += (IS_YC48) ? 0 : logo_pitch, src += (IS_YC48) ? src_pitch : logo_pitch) {
        short *dst_ptr = dst;
        TYPE *src_ptr = src;
        short *buf_ptr = buffer;
        const short *mask_ptr = mask;
        for (int x = x_fin; x; x -= 16, src_ptr += 16, dst_ptr += (IS_YC48) ? 0 : 16, buf_ptr += 16, mask_ptr += 16) {
            yResult = _mm256_load_si256((__m256i *)(buf_ptr + BUF_LINE_OFFSET(y-i+range*2)));
            yResult = smooth_vertical<range, buf_line, line_size>(buf_ptr, yResult, y);

            yMask = _mm256_loadu_si256((__m256i *)mask_ptr);
            ySrc = LOAD_256BIT(src_ptr);
            STORE_256BIT((IS_YC48) ? (void*)src_ptr : (void*)dst_ptr, _mm256_blendv_epi8(ySrc, yResult, _mm256_cmpgt_epi16(yMask, _mm256_set1_epi16(1024))));
        }
        yResult = _mm256_load_si256((__m256i *)(buf_ptr + BUF_LINE_OFFSET(y-i+range*2)));
        yResult = smooth_vertical<range, buf_line, line_size>(buf_ptr, yResult, y);

        yMask = _mm256_loadu_si256((__m256i *)mask_ptr);
        ySrc = LOAD_256BIT(src_ptr);
        STORE_256BIT((IS_YC48) ? (void*)src_ptr : (void*)dst_ptr, _mm256_blendv_epi8(ySrc, yResult, _mm256_cmpgt_epi16(yMask, _mm256_set1_epi16(1024))));
    }
#else
        x0 = _mm_set1_epi16(*(short *)(src_ptr + range_offset));
        x1 = LOAD_128BIT(src_ptr + range_offset);
        for (int x = x_fin; x; x -= 8, src_ptr += 8, dst_ptr += (IS_YC48) ? 0 : 8, buf_ptr += 8, mask_ptr += 8) {
            x2 = LOAD_128BIT(src_ptr + range_offset + 8);
            //連続するデータx0, x1, x2を使って水平方向の加算を行う
            xResult = smooth_horizontal<range>(x0, x1, x2);
            //xResultとバッファに格納されている水平方向の加算結果を合わせて
            //垂直方向の加算を行い、スムージングを完成させる
            //このループで得た水平加算結果はバッファに新たに格納される (不要になったものを上書き)
            xResult = smooth_vertical<range, buf_line, line_size>(buf_ptr, xResult, y);

            xMask = _mm_loadu_si128((__m128i *)mask_ptr);
            xSrc = LOAD_128BIT(src_ptr);
            //YC48モードではsrcに直接上書きする (計算用のデータはバッファに先読みしてあるため問題ない)
            STORE_128BIT((IS_YC48) ? (void*)src_ptr : (void*)dst_ptr, _mm_blendv_epi8_simd(xSrc, xResult, _mm_cmpgt_epi16(xMask, _mm_set1_epi16(1024))));

            x0 = x1;
            x1 = x2;
        }
        x2 = _mm_set1_epi16(*(short *)(src_ptr + range_offset - x_fin + width - 1));
        x1 = _mm_blendv_epi8_simd(x1, x2, _mm_loadu_si128((__m128i *)&MASK[8-(width & 7)]));
        xResult = smooth_horizontal<range>(x0, x1, x2);
        xResult = smooth_vertical<range, buf_line, line_size>(buf_ptr, xResult, y);

        xMask = _mm_loadu_si128((__m128i *)mask_ptr);
        xSrc = LOAD_128BIT(src_ptr);
        STORE_128BIT((IS_YC48) ? (void*)src_ptr : (void*)dst_ptr, _mm_blendv_epi8_simd(xSrc, xResult, _mm_cmpgt_epi16(xMask, _mm_set1_epi16(1024))));
    }
    //先読みできる分が終了したら、あとはバッファから読み込んで処理する
    //yとiの値に注意する
    for (int i = 1; i <= range; y++, i++, dst += (IS_YC48) ? 0 : logo_pitch, src += (IS_YC48) ? src_pitch : logo_pitch) {
        short *dst_ptr = dst;
        TYPE *src_ptr = src;
        short *buf_ptr = buffer;
        const short *mask_ptr = mask;
        for (int x = x_fin; x; x -= 8, src_ptr += 8, dst_ptr += (IS_YC48) ? 0 : 8, buf_ptr += 8, mask_ptr += 8) {
            xResult = _mm_load_si128((__m128i *)(buf_ptr + BUF_LINE_OFFSET(y-i+range*2)));
            xResult = smooth_vertical<range, buf_line, line_size>(buf_ptr, xResult, y);

            xMask = _mm_loadu_si128((__m128i *)mask_ptr);
            xSrc = LOAD_128BIT(src_ptr);
            STORE_128BIT((IS_YC48) ? (void*)src_ptr : (void*)dst_ptr, _mm_blendv_epi8_simd(xSrc, xResult, _mm_cmpgt_epi16(xMask, _mm_set1_epi16(1024))));
        }
        xResult = _mm_load_si128((__m128i *)(buf_ptr + BUF_LINE_OFFSET(y-i+range*2)));
        xResult = smooth_vertical<range, buf_line, line_size>(buf_ptr, xResult, y);

        xMask = _mm_loadu_si128((__m128i *)mask_ptr);
        xSrc = LOAD_128BIT(src_ptr);
        STORE_128BIT((IS_YC48) ? (void*)src_ptr : (void*)dst_ptr, _mm_blendv_epi8_simd(xSrc, xResult, _mm_cmpgt_epi16(xMask, _mm_set1_epi16(1024))));
    }
#endif
#else
    short *tmp;
    if (IS_YC48) {
        tmp = (short *)calloc(width * logo_pitch, sizeof(short));
        dst = tmp;
    }
    const float inv_div = 1.0f / (float)((2*range+1) * (2*range+1));
    for (int y = 0; y < height; y++, dst += logo_pitch, mask += logo_pitch) {
        short *dst_ptr = dst;
        const short *mask_ptr = mask;
        for (int x = 0; x < width; x++, dst_ptr++, mask_ptr++) {
            short result = 0;
            if (*mask_ptr > 1024) {
                int temp = 0;
                for (int j = -1 * (int)range; j <= (int)range; j++) {
                    for (int i = -1 * (int)range; i <= (int)range; i++) {
#define clamp(x, low, high) (((x) <= (high)) ? (((x) >= (low)) ? (x) : (low)) : (high))
                        int ix = clamp(x+i, 0, width-1);
                        int jy = clamp(y+j, 0, height-1);
#undef clamp
                        temp += *((short *)&src[ix + jy * src_pitch]);
                    }
                }
                result = (short)((float)temp * inv_div + 0.5f);
            } else {
                result = *((short *)&src[x + y * src_pitch]);
            }
            *((short *)dst_ptr) = result;
        }
    }

    if (IS_YC48) {
        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                *((short *)&src[y * src_pitch + x]) = tmp[y * logo_pitch + x];
            }
        }
        free(tmp);
    }
#endif
}
#pragma warning (pop)
#undef BUF_LINE_OFFSET
