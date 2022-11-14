#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

#define DEVICE_PATH "/dev/qrandom0"

// usage ./benchmark [-p(for printing)] [integer count]

int main(int argc, char* argv[]) {
    clock_t start, end;
    start = clock();

    bool print_x = false;
    int cnt = 100;
    for(int i=1;i<argc;i++){
        if(std::string(argv[i])=="-p") print_x = true;
        else cnt=atoi(argv[i]);
    }
    printf("Retrieving %d rnd 32 bit integers from %s.\n",cnt,DEVICE_PATH);
    int fd = open(DEVICE_PATH, O_RDONLY);
    if(fd==-1){
        printf("Opening %s failed with error %d.",DEVICE_PATH, errno);
        return 0;
    }
    for(int i=0;i<cnt;i++)
    {
        int x;
        ssize_t r = read(fd,&x,sizeof(x));
        if(r!=sizeof(x)) {
            printf("Reading x failed with error %d.", errno);
            return 0;
        }
        if(print_x)
            printf("%d ", x);
    }
    printf("\n");
    close(fd);

    end = clock();
    double time_taken = double(end-start)/double(CLOCKS_PER_SEC);
    printf("Time taken by program is: %5f sec", time_taken);
}
