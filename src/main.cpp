#include <stdlib.h>
#include <stdio.h>

#include "qrng.h"

int main(int argc, char **argv) {
    graal_isolatethread_t *thread = NULL;

    if (graal_create_isolate(NULL, NULL/* &isolate */, &thread) != 0) {
        fprintf(stderr, "graal_create_isolate error\n");
        return 1;
    }

    qrng_connect(thread);
    
    char* s = qrng_get_main_executable(thread);
    qrng_free_result(thread, s);

    char buf[10];
    s = qrng_get_random_bytes(thread, buf, 10);
    if (s==NULL) {
        for (int i=0; i<10; i++)
            printf("%d ",buf[i]);
    }
    else {
        printf("error: [%s]\n", s);
    }
    qrng_free_result(thread, s);
    


    if (graal_detach_thread(thread) != 0) {
        fprintf(stderr, "graal_detach_thread error\n");
        return 1;
    }
    
    return 0;
}