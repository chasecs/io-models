// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <errno.h>
// int main(int argc, char *argv[])
// {
//     int num;
//     FILE *fptr;
//     int flags = fcntl(fptr, F_GETFL, 0);
//     fcntl(fptr, F_SETFL, flags | O_NONBLOCK);

//     // if ((fptr = fopen(argv[1], "r")) == NULL)
//     // {
//     //     printf("Error! opening file");
//     //     // Program exits if the file pointer returns NULL.
//     //     exit(1);
//     // }

//     while (1)
//     {

//         if ((fptr = fopen(argv[1], "r")) < 0)
//         {
//             if (errno != EWOULDBLOCK)
//             {
//                 perror("read/fd1");
//                 exit(1);
//             }
//         }
//         else
//         {
//             fscanf(fptr, "%d", &num);

//             printf("Value of n=%d", num);
//             // fclose(fptr);

//             // return 0;
//         }
//         printf("wating...%d\n", fptr);
//     }
//     fclose(fptr);

//     return 0;
// }

#include "apue.h"
#include <errno.h>
#include <fcntl.h>
#include <time.h>

char buf[500000];

int main(void)
{
    int ntowrite, nwrite;
    char *ptr;

    ntowrite = read(STDIN_FILENO, buf, sizeof(buf));
    fprintf(stderr, "read %d bytes\n", ntowrite);

    set_fl(STDOUT_FILENO, O_NONBLOCK); /* set nonblocking */

    ptr = buf;
    while (ntowrite > 0)
    {
        errno = 0;
        clock_t begin = clock();
        /* here, do your time-consuming job */
        nwrite = write(STDOUT_FILENO, ptr, ntowrite);
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        fprintf(stderr, "nwrite = %d, errno = %d, time = %f\n", nwrite, errno, time_spent);

        if (nwrite > 0)
        {
            ptr += nwrite;
            ntowrite -= nwrite;
        }
    }

    clr_fl(STDOUT_FILENO, O_NONBLOCK); /* clear nonblocking */

    exit(0);
}

void set_fl(int fd, int flags) /* flags are file status flags to turn on */
{
    int val;

    if ((val = fcntl(fd, F_GETFL, 0)) < 0)
        printf("fcntl F_GETFL error");

    val |= flags; /* turn on flags */

    if (fcntl(fd, F_SETFL, val) < 0)
        printf("fcntl F_SETFL error");
}

void clr_fl(int fd, int flags) /* flags are file status flags to turn off */
{
    int val;

    if ((val = fcntl(fd, F_GETFL, 0)) < 0)
        printf("fcntl F_GETFL error");

    val &= ~flags; /* turn flags off */

    if (fcntl(fd, F_SETFL, val) < 0)
        printf("fcntl F_SETFL error");
}