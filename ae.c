//
// Created by skyjerry on 2020/7/22.
//

#include "ae_select.c"

int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask, aeFileProc *proc) {
    aeFileEvent *fe = &eventLoop->events[fd];

    // 设置FD描述符
    aeApiAddEvent(eventLoop, fd, mask);

    // TODO ????
    fe->mask |= mask;

    // 设置读写处理事件
    if (mask & AE_READABLE) {
        fe->rFileProc = proc;
    }
    if (mask & AE_WRITABLE) {
        fe->wFileProc = proc;
    }

    if (fd > eventLoop->maxfd) {
        eventLoop->maxfd = fd;
    }
    return 0;
}

void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask)
{
    aeFileEvent *fe = &eventLoop->events[fd];
    if (fe->mask == AE_NONE) return;

    /* We want to always remove AE_BARRIER if set when AE_WRITABLE
     * is removed. */
    if (mask & AE_WRITABLE) mask |= AE_BARRIER;

    aeApiDelEvent(eventLoop, fd, mask);
    fe->mask = fe->mask & (~mask);
    if (fd == eventLoop->maxfd && fe->mask == AE_NONE) {
        /* Update the max fd */
        int j;

        for (j = eventLoop->maxfd-1; j >= 0; j--)
            if (eventLoop->events[j].mask != AE_NONE) break;
        eventLoop->maxfd = j;
    }
}

