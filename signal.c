#include "apue.h"
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

char buf[500000];

static volatile sig_atomic_t gotSigio = 0;
/* Set nonzero on receipt of SIGIO */

static void
sigioHandler(int sig)
{
    gotSigio = 1;
}

int main(void)
{
    int ntowrite, nwrite;
    char *ptr;

    ntowrite = read(STDIN_FILENO, buf, sizeof(buf));
    fprintf(stderr, "read %d bytes\n", ntowrite);

    struct sigaction sa;
    /* Establish handler for "I/O possible" signal */

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sigioHandler;
    // On Linux platform
    if (sigaction(SIGIO, &sa, NULL) == -1)
        printf("sigaction");

    /* Set owner process that is to receive "I/O possible" signal */

    if (fcntl(STDOUT_FILENO, F_SETOWN, getpid()) == -1)
        printf("fcntl(F_SETOWN)");

    /* Enable "I/O possible" signaling and make I/O nonblocking
        for file descriptor */
    set_fl(STDOUT_FILENO, O_ASYNC | O_NONBLOCK);

    ptr = buf;
    while (ntowrite > 0)
    {
        errno = 0;
        if (gotSigio)
        { /* Is data available? */
            gotSigio = 0;
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