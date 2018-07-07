/*====================================================================
* 				delogo_util.h
*===================================================================*/
#ifndef __DELOGO_UTIL_H__
#define __DELOGO_UTIL_H__

#include <malloc.h>

struct aligned_malloc_deleter {
    void operator()(void* ptr) const {
        _aligned_free(ptr);
    }
};

struct malloc_deleter {
    void operator()(void* ptr) const {
        free(ptr);
    }
};


#endif //#ifdef __DELOGO_UTIL_H__
