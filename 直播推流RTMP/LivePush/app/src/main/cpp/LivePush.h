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
    bool isPushing = true;
    uint32_t startTime;
    pthread_t initConnectTid;

public:
    LivePush(const char *mLiveUrl,DZJNICall *pJniCall);
    ~LivePush();

    void initConnect();

    void pushSpsPPs(jbyte *string, jint i, jbyte *string1, jint i1);


    void pushVideo(jbyte *videoData, jint dataLen, jboolean keyFrame);


    void pushAudio(jbyte *data, jint dataLen);

    void stop();
};


#endif //LIVEPUSH_LIVEPUSH_H
