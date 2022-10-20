// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
// #include <poll.h>
// #define TIMEOUT 500 // poll timeout in ms

// int main() {
//     char name[255];
//     int counter = 0;

//     pollfd qrngp = {};
//     qrngp.fd = 0;
//     qrngp.events = POLLIN;

//     printf("Type in your name: \n");

//     while(1) {
//         r = poll(&qrngp,1,TIMEOUT)
//         if(r<0)
//         {
            
//         }
//         else if(r==0)
//         {
//             printf("Poll timed out\n");
//         }
//     }
//     printf("%d %d %d\n", fp.events, fp.fd, fp.revents);


// }