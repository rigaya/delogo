#ifndef ___DELOGO_PROC_H
#define ___DELOGO_PROC_H

#include "filter.h"
#include "logo.h"

#define PITCH(x) (((x) + 15) & (~15))

typedef BOOL (__stdcall * FUNC_PROCESS_LOGO)(FILTER* fp, FILTER_PROC_INFO *fpip, const LOGO_HEADER *lgh, int fade);

typedef void (__stdcall * FUNC_PROCESS_LOGO_Y)(short *buffer_dst, const short *buffer_src, FILTER *fp, const LOGO_HEADER *lgh, int fade);

typedef void (__stdcall * FUNC_PREWITT_FILTER)(short *logo_mask, const LOGO_HEADER *lgh);

typedef void (__stdcall * FUNC_WEIGHTED_PREWITT_FILTER)(short *dst, const short *src, int logo_width, int logo_src_pitch, int height);

typedef unsigned int (__stdcall * FUNC_PREWITT_EVALUATE)(short *result_buf, const short *logo_mask, const short *src, const LOGO_HEADER *lgh, unsigned int auto_result);

typedef void (__stdcall * FUNC_SMOOTH)(short *dst, short *src, const short *mask, int width, int pitch, int height);

typedef void (__stdcall * FUNC_SMOOTH_YC48)(PIXEL_YC *src, const short *mask, int width, int pitch, int height);

typedef int (__stdcall * FUNC_COUNT_LOGO_VALID_PIXELS)(const short *mask, int width, int logo_pitch, int height);

typedef struct DELOGO_FUNC_LIST {
    FUNC_PROCESS_LOGO delogo;
    FUNC_PROCESS_LOGO_Y delogo_y;
    FUNC_PREWITT_FILTER prewitt_filter;
    FUNC_WEIGHTED_PREWITT_FILTER weighted_prewitt_filter;
    FUNC_PREWITT_EVALUATE prewitt_evaluate[2];
    FUNC_SMOOTH smooth[4];
    FUNC_SMOOTH_YC48 smooth_yc48[4];
    FUNC_COUNT_LOGO_VALID_PIXELS count_valid_pixels;
    DWORD simd;
} DELOGO_FUNC_LIST;

const DELOGO_FUNC_LIST * get_delogo_func();

#endif //___DELOGO_PROC_H
