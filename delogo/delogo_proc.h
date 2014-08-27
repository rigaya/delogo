#ifndef ___DELOGO_PROC_H
#define ___DELOGO_PROC_H

#include "filter.h"
#include "logo.h"

typedef BOOL (__stdcall * FUNC_PROCESS_LOGO)(FILTER* fp, FILTER_PROC_INFO *fpip, LOGO_HEADER *lgh, int fade);

const FUNC_PROCESS_LOGO get_delogo_func();

#endif //___DELOGO_PROC_H
