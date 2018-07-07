#define USE_SSE2  0
#define USE_SSSE3 0
#define USE_SSE41 0
#define USE_AVX   0
#define USE_AVX2  0
#define USE_FMA3  0
#include "delogo_proc_simd.h"
#include "smooth_simd.h"
#include "delogo_proc.h"

BOOL __stdcall func_proc_eraze_logo(FILTER *fp, FILTER_PROC_INFO *fpip, const LOGO_HEADER *lgh, int fade) {
    return func_proc_eraze_logo_simd(fp, fpip, lgh, fade);
}

void __stdcall func_proc_eraze_logo_y_c(short *buffer_dst, const short *buffer_src, FILTER *fp, const LOGO_HEADER *lgh, int fade) {
    func_proc_eraze_logo_y_simd(buffer_dst, buffer_src, fp, lgh, fade);
}

void __stdcall func_proc_prewitt_filter_c(short *logo_mask, const LOGO_HEADER *lgh) {
    func_proc_prewitt_filter_simd(logo_mask, lgh);
}

void __stdcall func_proc_weighted_prewitt_filter_c(short *dst, const short *src, int logo_width, int logo_src_pitch, int height) {
    func_proc_weighted_prewitt_filter_simd(dst, src, logo_width, logo_src_pitch, height);
}

unsigned int __stdcall func_proc_prewitt_evaluate_c(const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result) {
    return func_proc_prewitt_evaluate_simd(logo_mask, src, lgh, auto_result);
}

unsigned int __stdcall func_proc_prewitt_5x5_evaluate_c(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result) {
    return func_proc_prewitt_5x5_evaluate_simd<false>(result_buf, logo_mask, src, lgh, auto_result);
}

unsigned int __stdcall func_proc_prewitt_5x5_evaluate_store_c(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result) {
    return func_proc_prewitt_5x5_evaluate_simd<true>(result_buf, logo_mask, src, lgh, auto_result);
}

void __stdcall func_smooth_3x3_c(short *dst, short *src, const short *mask, int width, int pitch, int height) {
    smooth_simd<1>(dst, src, mask, width, pitch, height);
}

void __stdcall func_smooth_5x5_c(short *dst, short *src, const short *mask, int width, int pitch, int height) {
    smooth_simd<2>(dst, src, mask, width, pitch, height);
}

void __stdcall func_smooth_7x7_c(short *dst, short *src, const short *mask, int width, int pitch, int height) {
    smooth_simd<3>(dst, src, mask, width, pitch, height);
}

void __stdcall func_smooth_9x9_c(short *dst, short *src, const short *mask, int width, int pitch, int height) {
    smooth_simd<4>(dst, src, mask, width, pitch, height);
}

void __stdcall func_smooth_yc48_3x3_c(PIXEL_YC *src, const short *mask, int width, int pitch, int height) {
    smooth_simd<1>(nullptr, src, mask, width, pitch, height);
}

void __stdcall func_smooth_yc48_5x5_c(PIXEL_YC *src, const short *mask, int width, int pitch, int height) {
    smooth_simd<2>(nullptr, src, mask, width, pitch, height);
}

void __stdcall func_smooth_yc48_7x7_c(PIXEL_YC *src, const short *mask, int width, int pitch, int height) {
    smooth_simd<3>(nullptr, src, mask, width, pitch, height);
}

void __stdcall func_smooth_yc48_9x9_c(PIXEL_YC *src, const short *mask, int width, int pitch, int height) {
    smooth_simd<4>(nullptr, src, mask, width, pitch, height);
}

int __stdcall func_count_logo_valid_pixels_c(const short *mask, int width, int logo_pitch, int height) {
    return count_logo_valid_pixels_simd(mask, width, logo_pitch, height);
}

BOOL __stdcall func_proc_eraze_logo_sse2(FILTER *fp, FILTER_PROC_INFO *fpip, const LOGO_HEADER *lgh, int fade);
BOOL __stdcall func_proc_eraze_logo_sse41(FILTER *fp, FILTER_PROC_INFO *fpip, const LOGO_HEADER *lgh, int fade);
BOOL __stdcall func_proc_eraze_logo_avx(FILTER *fp, FILTER_PROC_INFO *fpip, const LOGO_HEADER *lgh, int fade);
BOOL __stdcall func_proc_eraze_logo_avx2(FILTER *fp, FILTER_PROC_INFO *fpip, const LOGO_HEADER *lgh, int fade);

void __stdcall func_proc_eraze_logo_y_sse2(short *buffer_dst, const short *buffer_src, FILTER *fp, const LOGO_HEADER *lgh, int fade);
void __stdcall func_proc_eraze_logo_y_sse41(short *buffer_dst, const short *buffer_src, FILTER *fp, const LOGO_HEADER *lgh, int fade);
void __stdcall func_proc_eraze_logo_y_avx(short *buffer_dst, const short *buffer_src, FILTER *fp, const LOGO_HEADER *lgh, int fade);
void __stdcall func_proc_eraze_logo_y_avx2(short *buffer_dst, const short *buffer_src, FILTER *fp, const LOGO_HEADER *lgh, int fade);

void __stdcall func_proc_prewitt_filter_sse2(short *logo_mask, const LOGO_HEADER *lgh);
void __stdcall func_proc_prewitt_filter_ssse3(short *logo_mask, const LOGO_HEADER *lgh);
void __stdcall func_proc_prewitt_filter_sse41(short *logo_mask, const LOGO_HEADER *lgh);
void __stdcall func_proc_prewitt_filter_avx(short *logo_mask, const LOGO_HEADER *lgh);
void __stdcall func_proc_prewitt_filter_avx2(short *logo_mask, const LOGO_HEADER *lgh);

void __stdcall func_proc_weighted_prewitt_filter_sse2(short *dst, const short *src, int logo_width, int logo_src_pitch, int height);
void __stdcall func_proc_weighted_prewitt_filter_ssse3(short *dst, const short *src, int logo_width, int logo_src_pitch, int height);
void __stdcall func_proc_weighted_prewitt_filter_sse41(short *dst, const short *src, int logo_width, int logo_src_pitch, int height);
void __stdcall func_proc_weighted_prewitt_filter_avx(short *dst, const short *src, int logo_width, int logo_src_pitch, int height);
void __stdcall func_proc_weighted_prewitt_filter_avx2(short *dst, const short *src, int logo_width, int logo_src_pitch, int height);

unsigned int __stdcall func_proc_prewitt_evaluate_sse2(const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_evaluate_ssse3(const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_evaluate_sse41(const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_evaluate_avx(const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_evaluate_avx2(const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);

unsigned int __stdcall func_proc_prewitt_5x5_evaluate_sse2(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_5x5_evaluate_ssse3(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_5x5_evaluate_sse41(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_5x5_evaluate_avx(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_5x5_evaluate_avx2(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);

unsigned int __stdcall func_proc_prewitt_5x5_evaluate_store_sse2(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_5x5_evaluate_store_ssse3(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_5x5_evaluate_store_sse41(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_5x5_evaluate_store_avx(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);
unsigned int __stdcall func_proc_prewitt_5x5_evaluate_store_avx2(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);

void __stdcall func_smooth_3x3_sse2(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_3x3_ssse3(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_3x3_sse41(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_3x3_avx(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_3x3_avx2(short *dst, short *src, const short *mask, int width, int pitch, int height);

void __stdcall func_smooth_5x5_sse2(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_5x5_ssse3(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_5x5_sse41(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_5x5_avx(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_5x5_avx2(short *dst, short *src, const short *mask, int width, int pitch, int height);

void __stdcall func_smooth_7x7_sse2(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_7x7_ssse3(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_7x7_sse41(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_7x7_avx(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_7x7_avx2(short *dst, short *src, const short *mask, int width, int pitch, int height);

void __stdcall func_smooth_9x9_sse2(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_9x9_ssse3(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_9x9_sse41(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_9x9_avx(short *dst, short *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_9x9_avx2(short *dst, short *src, const short *mask, int width, int pitch, int height);




void __stdcall func_smooth_yc48_3x3_sse2(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_3x3_ssse3(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_3x3_sse41(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_3x3_avx(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_3x3_avx2(PIXEL_YC *src, const short *mask, int width, int pitch, int height);

void __stdcall func_smooth_yc48_5x5_sse2(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_5x5_ssse3(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_5x5_sse41(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_5x5_avx(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_5x5_avx2(PIXEL_YC *src, const short *mask, int width, int pitch, int height);

void __stdcall func_smooth_yc48_7x7_sse2(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_7x7_ssse3(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_7x7_sse41(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_7x7_avx(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_7x7_avx2(PIXEL_YC *src, const short *mask, int width, int pitch, int height);

void __stdcall func_smooth_yc48_9x9_sse2(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_9x9_ssse3(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_9x9_sse41(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_9x9_avx(PIXEL_YC *src, const short *mask, int width, int pitch, int height);
void __stdcall func_smooth_yc48_9x9_avx2(PIXEL_YC *src, const short *mask, int width, int pitch, int height);

int __stdcall func_count_logo_valid_pixels_sse2(const short *mask, int width, int logo_pitch, int height);
int __stdcall func_count_logo_valid_pixels_avx(const short *mask, int width, int logo_pitch, int height);
int __stdcall func_count_logo_valid_pixels_avx2(const short *mask, int width, int logo_pitch, int height);


#include <intrin.h>

enum {
    NONE   = 0x0000,
    SSE2   = 0x0001,
    SSE3   = 0x0002,
    SSSE3  = 0x0004,
    SSE41  = 0x0008,
    SSE42  = 0x0010,
    POPCNT = 0x0020,
    AVX    = 0x0040,
    AVX2   = 0x0080,
    FMA3   = 0x0100,
};

static DWORD get_availableSIMD() {
    int CPUInfo[4];
    __cpuid(CPUInfo, 1);
    DWORD simd = NONE;
    if (CPUInfo[3] & 0x04000000) simd |= SSE2;
    if (CPUInfo[2] & 0x00000001) simd |= SSE3;
    if (CPUInfo[2] & 0x00000200) simd |= SSSE3;
    if (CPUInfo[2] & 0x00080000) simd |= SSE41;
    if (CPUInfo[2] & 0x00100000) simd |= SSE42;
    if (CPUInfo[2] & 0x00800000) simd |= POPCNT;
#if (_MSC_VER >= 1600)
    UINT64 xgetbv = 0;
    if ((CPUInfo[2] & 0x18000000) == 0x18000000) {
        xgetbv = _xgetbv(0);
        if ((xgetbv & 0x06) == 0x06)
            simd |= AVX;
#if (_MSC_VER >= 1700)
        if(CPUInfo[2] & 0x00001000 )
            simd |= FMA3;
#endif //(_MSC_VER >= 1700)
    }
#endif
#if (_MSC_VER >= 1700)
    __cpuid(CPUInfo, 7);
    if ((simd & AVX) && (CPUInfo[1] & 0x00000020))
        simd |= AVX2;
#endif
    return simd;
}

#define EVALUATE_5x5_AVX2  { func_proc_prewitt_5x5_evaluate_avx2,  func_proc_prewitt_5x5_evaluate_store_avx2  }
#define EVALUATE_5x5_AVX   { func_proc_prewitt_5x5_evaluate_avx,   func_proc_prewitt_5x5_evaluate_store_avx   }
#define EVALUATE_5x5_SSE41 { func_proc_prewitt_5x5_evaluate_sse41, func_proc_prewitt_5x5_evaluate_store_sse41 }
#define EVALUATE_5x5_SSSE3 { func_proc_prewitt_5x5_evaluate_ssse3, func_proc_prewitt_5x5_evaluate_store_ssse3 }
#define EVALUATE_5x5_SSE2  { func_proc_prewitt_5x5_evaluate_sse2,  func_proc_prewitt_5x5_evaluate_store_sse2  }
#define EVALUATE_5x5_C     { func_proc_prewitt_5x5_evaluate_c,     func_proc_prewitt_5x5_evaluate_store_c     }

#define SMOOTH_AVX2  { func_smooth_3x3_avx2,  func_smooth_5x5_avx2,  func_smooth_7x7_avx2,  func_smooth_9x9_avx2  }
#define SMOOTH_AVX   { func_smooth_3x3_avx,   func_smooth_5x5_avx,   func_smooth_7x7_avx,   func_smooth_9x9_avx   }
#define SMOOTH_SSE41 { func_smooth_3x3_sse41, func_smooth_5x5_sse41, func_smooth_7x7_sse41, func_smooth_9x9_sse41 }
#define SMOOTH_SSSE3 { func_smooth_3x3_ssse3, func_smooth_5x5_ssse3, func_smooth_7x7_ssse3, func_smooth_9x9_ssse3 }
#define SMOOTH_SSE2  { func_smooth_3x3_sse2,  func_smooth_5x5_sse2,  func_smooth_7x7_sse2,  func_smooth_9x9_sse2  }
#define SMOOTH_C     { func_smooth_3x3_c,     func_smooth_5x5_c,     func_smooth_7x7_c,     func_smooth_9x9_c     }

#define SMOOTH_YC48_AVX2  { func_smooth_yc48_3x3_avx2,  func_smooth_yc48_5x5_avx2,  func_smooth_yc48_7x7_avx2,  func_smooth_yc48_9x9_avx2  }
#define SMOOTH_YC48_AVX   { func_smooth_yc48_3x3_avx,   func_smooth_yc48_5x5_avx,   func_smooth_yc48_7x7_avx,   func_smooth_yc48_9x9_avx   }
#define SMOOTH_YC48_SSE41 { func_smooth_yc48_3x3_sse41, func_smooth_yc48_5x5_sse41, func_smooth_yc48_7x7_sse41, func_smooth_yc48_9x9_sse41 }
#define SMOOTH_YC48_SSSE3 { func_smooth_yc48_3x3_ssse3, func_smooth_yc48_5x5_ssse3, func_smooth_yc48_7x7_ssse3, func_smooth_yc48_9x9_ssse3 }
#define SMOOTH_YC48_SSE2  { func_smooth_yc48_3x3_sse2,  func_smooth_yc48_5x5_sse2,  func_smooth_yc48_7x7_sse2,  func_smooth_yc48_9x9_sse2  }
#define SMOOTH_YC48_C     { func_smooth_yc48_3x3_c,     func_smooth_yc48_5x5_c,     func_smooth_yc48_7x7_c,     func_smooth_yc48_9x9_c     }

const FUNC_SMOOTH smooth_c[] = SMOOTH_C;
const FUNC_SMOOTH_YC48 smooth_yc48_c[] = SMOOTH_YC48_C;

static const DELOGO_FUNC_LIST FUNC_LIST[] = {
    { func_proc_eraze_logo_avx2,  func_proc_eraze_logo_y_avx2,  func_proc_prewitt_filter_avx2,  func_proc_weighted_prewitt_filter_avx2,  EVALUATE_5x5_AVX2,  SMOOTH_AVX2,  SMOOTH_YC48_AVX2,  func_count_logo_valid_pixels_avx2, FMA3|AVX2|AVX        },
    { func_proc_eraze_logo_avx,   func_proc_eraze_logo_y_avx,   func_proc_prewitt_filter_avx,   func_proc_weighted_prewitt_filter_avx,   EVALUATE_5x5_AVX,   SMOOTH_AVX,   SMOOTH_YC48_AVX,   func_count_logo_valid_pixels_avx,  AVX|SSE41|SSSE3|SSE2 },
    { func_proc_eraze_logo_sse41, func_proc_eraze_logo_y_sse41, func_proc_prewitt_filter_sse41, func_proc_weighted_prewitt_filter_sse41, EVALUATE_5x5_SSE41, SMOOTH_SSE41, SMOOTH_YC48_SSE41, func_count_logo_valid_pixels_sse2, SSE41|SSSE3|SSE2     },
    { func_proc_eraze_logo_sse2,  func_proc_eraze_logo_y_sse2,  func_proc_prewitt_filter_ssse3, func_proc_weighted_prewitt_filter_ssse3, EVALUATE_5x5_SSSE3, SMOOTH_SSSE3, SMOOTH_YC48_SSSE3, func_count_logo_valid_pixels_sse2, SSSE3|SSE2           },
    { func_proc_eraze_logo_sse2,  func_proc_eraze_logo_y_sse2,  func_proc_prewitt_filter_sse2,  func_proc_weighted_prewitt_filter_sse2,  EVALUATE_5x5_SSE2,  SMOOTH_SSE2,  SMOOTH_YC48_SSE2,  func_count_logo_valid_pixels_sse2, SSE2                 },
    { func_proc_eraze_logo,       func_proc_eraze_logo_y_c,     func_proc_prewitt_filter_c,     func_proc_weighted_prewitt_filter_c,     EVALUATE_5x5_C,     SMOOTH_C,     SMOOTH_YC48_C,     func_count_logo_valid_pixels_c,    NONE                 },
};

const DELOGO_FUNC_LIST *get_delogo_func() {
    const DWORD simd_avail = get_availableSIMD();
    for (int i = 0; i < _countof(FUNC_LIST); i++) {
        if ((FUNC_LIST[i].simd & simd_avail) == FUNC_LIST[i].simd) {
            return &FUNC_LIST[i];
        }
    }
    return NULL;
};
