#define USE_SSE2  0
#define USE_SSSE3 0
#define USE_SSE41 0
#define USE_AVX   0
#define USE_AVX2  0
#define USE_FMA3  0
#include "delogo_proc_simd.h"
#include "delogo_proc.h"

BOOL __stdcall func_proc_eraze_logo(FILTER *fp, FILTER_PROC_INFO *fpip, LOGO_HEADER *lgh, int fade) {
    return func_proc_eraze_logo_simd(fp, fpip, lgh, fade);
}

BOOL __stdcall func_proc_eraze_logo_sse2(FILTER *fp, FILTER_PROC_INFO *fpip, LOGO_HEADER *lgh, int fade);
BOOL __stdcall func_proc_eraze_logo_sse41(FILTER *fp, FILTER_PROC_INFO *fpip, LOGO_HEADER *lgh, int fade);
BOOL __stdcall func_proc_eraze_logo_avx2(FILTER *fp, FILTER_PROC_INFO *fpip, LOGO_HEADER *lgh, int fade);

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

typedef struct {
    FUNC_PROCESS_LOGO func;
    DWORD simd;
} DELOGO_FUNC_LIST;

static const DELOGO_FUNC_LIST FUNC_LIST[] = {
    { func_proc_eraze_logo_avx2,  FMA3|AVX2|AVX    },
    { func_proc_eraze_logo_sse41, SSE41|SSSE3|SSE2 },
    { func_proc_eraze_logo_sse2,  SSE2             },
    { func_proc_eraze_logo,       NONE             },
};

const FUNC_PROCESS_LOGO get_delogo_func() {
    const DWORD simd_avail = get_availableSIMD();
    for (int i = 0; i < _countof(FUNC_LIST); i++) {
        if ((FUNC_LIST[i].simd & simd_avail) == FUNC_LIST[i].simd) {
            return FUNC_LIST[i].func;
        }
    }
    return NULL;
};
