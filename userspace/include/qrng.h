#ifndef __QRNG_H
#define __QRNG_H

#include <graal_isolate.h>


#if defined(__cplusplus)
extern "C" {
#endif

char* qrng_get_main_executable(graal_isolatethread_t*);

void qrng_connect(graal_isolatethread_t*);

char* qrng_get_random_bytes(graal_isolatethread_t*, char*, int);

void qrng_free_result(graal_isolatethread_t*, char*);

#if defined(__cplusplus)
}
#endif
#endif
