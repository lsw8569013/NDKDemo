//
// Created by lsw on 2019-07-28.
//

#ifndef COPYAPP_VMEDIA_H
#define COPYAPP_VMEDIA_H


#include "DZJNICall.h"
#include "PlayerStatus.h"
#include "PacketQueue.h"


extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
};


class VMedia {
public:
    int streamIndex = -1;
    AVCodecContext *pCodecContext = NULL;
    DZJNICall *pJniCall = NULL;
    PlayerStatus *pPlayerStatus = NULL;
    PacketQueue  *pPacketQueue = NULL;

    AVFormatContext *pFormatCtx = NULL;

    int duration = 0;

    /**
     * 当前播放的时间
     */
    double  currentTime = 0;
    /**
     * 上次更新的时间
     */
    double  lastUpdateTime = 0;

    /**
     * 时间基
     */
    AVRational timeBase ;
public:
    VMedia(int streamIndex,DZJNICall *pJniCall,PlayerStatus *pPlayerStatus);
    ~VMedia();

public:
    // 纯虚函数 必须继承
    virtual void play() = 0;

    void analysisStream(ThreadMode mode, AVFormatContext *pFormatCtx);

    virtual void release();

    void callPlayerJniError(ThreadMode mode, int code, char *msg) ;

    //
    virtual void privateAnalysisStream (ThreadMode mode, AVFormatContext *pFormatCtx)=0;


private:
    void publicAnalysisStream(ThreadMode mode, AVFormatContext *pFormatCtx);


};


#endif //COPYAPP_VMEDIA_H
