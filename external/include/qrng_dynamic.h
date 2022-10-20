#ifndef __QRNG_H
#define __QRNG_H

#include <graal_isolate_dynamic.h>


#if defined(__cplusplus)
extern "C" {
#endif

typedef char* (*qrng_get_main_executable_fn_t)(graal_isolatethread_t*);

typedef void (*qrng_connect_fn_t)(graal_isolatethread_t*);

typedef char* (*qrng_get_random_bytes_fn_t)(graal_isolatethread_t*, char*, int);

typedef void (*qrng_free_result_fn_t)(graal_isolatethread_t*, char*);

#if defined(__cplusplus)
}
#endif
#endif
