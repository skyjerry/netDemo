//
// Created by skyjerry on 2020/7/7.
//

#ifndef NETDEMO_AE_H
#define NETDEMO_AE_H

#define AE_NONE 0       /* No events registered. */
#define AE_READABLE 1   /* Fire when descriptor is readable. */
#define AE_WRITABLE 2   /* Fire when descriptor is writable. */
#define AE_BARRIER 4    /* With WRITABLE, never fire the event if the
                           READABLE event already fired in the same event
                           loop iteration. Useful when you want to persist
                           things to disk before sending replies, and want
                           to do that in a group fashion. */

#define MAX_CLIENT_NUM 100

struct aeEventLoop;


typedef void aeFileProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);

/* File event structure */
typedef struct aeFileEvent {
    int mask; /* one of AE_(READABLE|WRITABLE|BARRIER) */
    aeFileProc *rFileProc;
    aeFileProc *wFileProc;
    void *clientData;
} aeFileEvent;

/* A fired event */
typedef struct aeFiredEvent {
    int fd;
    int mask;
} aeFiredEvent;

typedef struct aeEventLoop {
    int maxfd;
    aeFileEvent *events; /* Registered events */
    aeFiredEvent *fired; /* Fired events */
    int stop;
    void *apiData; /* This is used for polling API specific data */
} aeEventLoop;

#endif //NETDEMO_AE_H
