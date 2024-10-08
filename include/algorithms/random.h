#pragma once
#define __XOC_ALGORITHMS_RANDOM_H__

#include "../base.h"

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

/// U -> Upper case  L -> lower case  N -> Number  S -> Special
#define XOC_CHAR_SET_ULN "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#define XOC_CHAR_SET_ULNS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+{}[]|\\:;\"'<>,.?/`~"

#define XOC_CHAR_SET_ULN_LEN 63
#define XOC_CHAR_SET_ULNS_LEN 93

xoc_int32_t
XOC_INTERFACE(xoc_rand32)(
    xoc_int32_t min ,
    xoc_int32_t max
);

xoc_int64_t
XOC_INTERFACE(xoc_rand64)(
    xoc_int64_t min ,
    xoc_int64_t max
);

void
XOC_INTERFACE(xoc_rand_string)(
    xoc_cstring_t    target_str ,
    xoc_size_t       target_str_len ,
    xoc_ccstring_t   char_set ,
    xoc_size_t       char_set_len
);

void
XOC_INTERFACE(xoc_sys_rand_buf)(
    xoc_pointer_t    buf , 
    xoc_size_t       len
);

void
XOC_INTERFACE(xoc_rand_string_utf8)(
    xoc_cstring_t    target_str ,
    xoc_size_t       target_str_len ,
    xoc_ccstring_t   char_set ,
    xoc_size_t       char_set_len
);

#if defined(__cplusplus)
}
#endif // __cplusplus

#define xoc_rand32 XOC_INTERFACE(xoc_rand32)
#define xoc_rand64 XOC_INTERFACE(xoc_rand64)
#define xoc_rand_string XOC_INTERFACE(xoc_rand_string)
#define xoc_sys_rand_buf XOC_INTERFACE(xoc_sys_rand_buf)
