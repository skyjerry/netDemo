#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "ae.h"
#include "anet.c"
#include "ae_select.c"



void error(int errNo) {
    printf("%d\n", errNo);
    exit(0);
}

void serveLog(const char *fmt, ...) {
    va_list ap;
    char msg[100];

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
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

int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask, aeFileProc *proc) {
    aeFileEvent *fe = &eventLoop->events[fd];

    // 设置FD描述符
    aeApiAddEvent(eventLoop, fd, mask);

    // TODO ????
    fe->mask |= mask;

    // 设置读写处理事件
    if (mask & AE_READABLE) {
        fe->rFileProc = proc;
        fe->wFileProc = proc;
    }

    if (fd > eventLoop->maxfd) {
        eventLoop->maxfd = fd;
    }
    return 0;
}

void handler(aeEventLoop *eventLoop, int fd, void *data, int mask) {
    struct sockaddr_storage ss;
    socklen_t sLen = sizeof(ss);
    int clientFd = accept(fd, (struct sockaddr *) &ss, &sLen);
    printf("accepted.\n");
    close(clientFd);
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
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    // 设置为非阻塞
    fcntl(listen_fd, F_SETFL, O_NONBLOCK);

    // 设置server参数
    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    bind(listen_fd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    listen(listen_fd, 1024);

    return listen_fd;
}

int main() {
    printf("netDemo, Hello!\n");

    // 创建事件循环
    aeEventLoop *eventLoop = createEventLoop();
    printf("eventLoop Created.\n");

    // 启动tcp服务器
    int fd = anetTcpServer(52000);
    printf("tcp server created %d.\n", fd);

    // 绑定文件事件处理器
    aeCreateFileEvent(eventLoop, fd, AE_READABLE, handler);
    printf("file event created.\n");

    // 运行主循环
    printf("event loop start.\n");
    aeMain(eventLoop);
    printf("event loop end.\n");
    return 0;
}


