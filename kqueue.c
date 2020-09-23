#include <sys/socket.h>
#include <sys/un.h>
#include <sys/event.h>
#include <netdb.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#define MAXBUF 256
void child_process(void)
{
    sleep(2);
    char msg[MAXBUF];
    struct sockaddr_in addr = {0};
    int n, sockfd, num = 1;
    srandom(getpid());
    /* Create socket and connect to server */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    printf("child {%d} connected \n", getpid());
    while (1)
    {
        int sl = (random() % 10) + 1;

        sleep(sl);
        sprintf(msg, "Server: message %d from client %d", num, getpid());
        printf("Client: message %d of %d \n", num, getpid());
        n = write(sockfd, msg, strlen(msg)); /* Send message */
        num++;
    }
}

int main(int argc, const char *argv[])
{
    int i;
    for (i = 0; i < 5; i++)
    {
        if (fork() == 0)
        {
            child_process();
            exit(0);
        }
    }
    // Macos automatically binds both ipv4 and 6 when you do this.
    struct sockaddr_in6 addr = {};
    addr.sin6_len = sizeof(addr);
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_any; //(struct in6_addr){}; // 0.0.0.0 / ::
    addr.sin6_port = htons(2000);

    int localFd = socket(addr.sin6_family, SOCK_STREAM, 0);
    assert(localFd != -1);

    int on = 1;
    setsockopt(localFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (bind(localFd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        return 1;
    }
    assert(listen(localFd, 5) != -1);

    int kq = kqueue();

    struct kevent evSet;

    // add listener to localFd
    EV_SET(&evSet, localFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    assert(-1 != kevent(kq, &evSet, 1, NULL, 0, NULL));

    uint64_t bytes_written = 0;

    struct kevent evList[32];
    char buffer[MAXBUF];

    while (1)
    {

        // returns number of events
        int nev = kevent(kq, NULL, 0, evList, 32, NULL);
        printf("round again\n");

        if (nev < 1)
            err(1, "kevent");
        for (int i = 0; i < nev; i++)
        {

            int fd = (int)evList[i].ident;

            if (evList[i].flags & EV_EOF)
            {
                printf("Disconnect\n");
                close(fd);
                // Socket is automatically removed from the kq by the kernel.
            }
            else if (fd == localFd)
            {

                struct sockaddr_storage addr;
                socklen_t socklen = sizeof(addr);
                int connfd = accept(fd, (struct sockaddr *)&addr, &socklen);
                assert(connfd != -1);
                printf("Got connection!\n");

                // add listener to connfd's event
                EV_SET(&evSet, connfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                kevent(kq, &evSet, 1, NULL, 0, NULL);
            }
            else if (evList[i].filter == EVFILT_READ)
            {
                // Read from socket.
                char buf[1024];
                // size_t bytes_read = read(fd, buf, sizeof(buf), 0);
                // printf("read %zu bytes\n", bytes_read);
                read(fd, buffer, MAXBUF);
                puts(buffer);
            }
            // else if (evList[i].filter == EVFILT_WRITE)
            // {
            //     //                printf("Ok to write more!\n");

            //     off_t offset = (off_t)evList[i].udata;
            //     off_t len = 0; //evList[i].data;
            //     if (sendfile(junk, fd, offset, &len, NULL, 0) != 0)
            //     {
            //         //                    perror("sendfile");
            //         //                    printf("err %d\n", errno);

            //         if (errno == EAGAIN)
            //         {
            //             // schedule to send the rest of the file
            //             EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, (void *)(offset + len));
            //             kevent(kq, &evSet, 1, NULL, 0, NULL);
            //         }
            //     }
            //     bytes_written += len;
            //     printf("wrote %lld bytes, %lld total\n", len, bytes_written);
            // }
        }
    }

    return 0;
}