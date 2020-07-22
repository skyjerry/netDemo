#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include "ae.c"



void error(int errNo) {
    printf("%d\n", errNo);
    exit(0);
}

aeEventLoop *createEventLoop() {
    int i;
    aeEventLoop *eventLoop;

    // 申请内存
    if ((eventLoop = malloc(sizeof(aeEventLoop))) == NULL) {
        error(1594133077);
    }

    // 申请内存
    eventLoop->events = malloc(sizeof(aeFileEvent) * MAX_CLIENT_NUM);
    eventLoop->fired = malloc(sizeof(aeFileEvent) * MAX_CLIENT_NUM);

    if (eventLoop->events == NULL || eventLoop->fired == NULL) {
        error(1594133202);
    }

    // 创建事件循环
    if (aeApiCreate(eventLoop) == -1) {
        error(1594133534);
    }

    for (i = 0; i < MAX_CLIENT_NUM; i++) {
        eventLoop->events[i].mask = AE_NONE;
    }
    return eventLoop;
}

void readFromClient(aeEventLoop *eventLoop, int fd, void *data, int mask) {
    int readLen = 1024;
    char buf[readLen];
    int n = read(fd, buf, readLen);
    if (n == -1) {
        if (errno == EAGAIN) {
            return;
        } else {
            printf("fd %d closed. errno: %d.\n", fd, errno);
            close(fd);
            return;
        }
    } else if (n == 0) {
        printf("fd %d close connection. errno: %d.\n", fd, errno);
        close(fd);
        return;
    } else {
        printf("%s", buf);

        if (buf[0] == '@') {
            aeDeleteFileEvent(eventLoop, fd, AE_READABLE);
            close(fd);
            return ;
        }

        if (write(fd, buf, n) < 0) {
            printf("fd %d error . errno: %d.\n", fd, errno);
        }
    }
}

void writeToClient(aeEventLoop *eventLoop, int fd, void *data, int mask) {

}

void handler(aeEventLoop *eventLoop, int fd, void *data, int mask) {
    struct sockaddr_storage ss;
    socklen_t sLen = sizeof(ss);
    int clientFd = accept(fd, (struct sockaddr *) &ss, &sLen);
    printf("fd: %d, accepted\n", clientFd);
    aeCreateFileEvent(eventLoop, clientFd, AE_READABLE, readFromClient);
}


int aeProcessEvents(aeEventLoop *eventLoop) {
    int processed = 0, eventsNum;
    int j;

    eventsNum = aeApiPoll(eventLoop, NULL);
    printf("eventsNum: %d\n", eventsNum);

    for (j = 0; j < eventsNum; j++) {
        aeFileEvent *fe = &eventLoop->events[eventLoop->fired[j].fd];
        int mask = eventLoop->fired[j].mask;
        int fd = eventLoop->fired[j].fd;
        int fired = 0;

        if (fe->mask & mask & AE_READABLE) {
            fe->rFileProc(eventLoop,fd,fe->clientData,mask);
            fired++;
        }
        if (fe->mask & mask & AE_WRITABLE) {
            fe->wFileProc(eventLoop,fd,fe->clientData,mask);
            fired++;
        }
    }

    return 0;
}

void aeMain(aeEventLoop *eventLoop) {
    eventLoop->stop = 0;
    while (!eventLoop->stop) {
        aeProcessEvents(eventLoop);
    }
}

int anetTcpServer(int port)
{
    // 创建一个监听的socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    // 设置为socket为非阻塞
    fcntl(listen_fd, F_SETFL, O_NONBLOCK);

    // 设置server参数
    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    // 把server信息绑定到socket
    bind(listen_fd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    // 监听socket
    listen(listen_fd, 1024);

    return listen_fd;
}

int main() {
    printf("netDemo, Hello!\n");

    // 创建事件循环
    aeEventLoop *eventLoop = createEventLoop();
    printf("eventLoop Created.\n");

    // 创建并启动tcp服务
    int fd = anetTcpServer(52000);
    printf("tcp server created %d.\n", fd);

    // 注册可读事件处理函数
    aeCreateFileEvent(eventLoop, fd, AE_READABLE, handler);
    printf("file event created.\n");

    // 事件循环
    printf("event loop start.\n");
    aeMain(eventLoop);
    printf("event loop end.\n");
    return 0;
}


