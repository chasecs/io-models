#include "apue.h"
#include <errno.h>
#include <time.h>

char buf[500000];

int main(void)
{
    int ntowrite, nwrite;
    char *ptr;

    ntowrite = read(STDIN_FILENO, buf, sizeof(buf));
    fprintf(stderr, "read %d bytes\n", ntowrite);

    ptr = buf;
    while (ntowrite > 0)
    {
        errno = 0;
        clock_t begin = clock();
        /* here, do your time-consuming job */
        nwrite = write(STDOUT_FILENO, ptr, ntowrite);
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("\nnwrite = %d, errno = %d, time = %f\n", nwrite, errno, time_spent);

        if (nwrite > 0)
        {
            ptr += nwrite;
            ntowrite -= nwrite;
        }
    }

    exit(0);
}