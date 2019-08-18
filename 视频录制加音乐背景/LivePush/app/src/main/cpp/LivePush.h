//
// Created by lsw on 2019-08-15.
//

#ifndef LIVEPUSH_LIVEPUSH_H
#define LIVEPUSH_LIVEPUSH_H


#include "DZJNICall.h"
#include "PacketQueue.h"

#include <cstring>
#include <malloc.h>

class LivePush {

public:
    DZJNICall *pJniCall;
    char* mLiveUrl;
    PacketQueue *pPacketQueue  = NULL;
    RTMP *pRtmp = NULL;

public:
    LivePush(const char *mLiveUrl,DZJNICall *pJniCall);
    ~LivePush();

    void initConnect();
};


#endif //LIVEPUSH_LIVEPUSH_H
