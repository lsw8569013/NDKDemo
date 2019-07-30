//
// Created by lsw on 2019-07-26.
//

#ifndef COPYAPP_VAUDIO_H
#define COPYAPP_VAUDIO_H

#include <pthread.h>
#include "DZJNICall.h"
#include "DZConstDefine.h"
#include "PlayerStatus.h"
#include "PacketQueue.h"
#include "VMedia.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>


extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
};

class VAudio : public VMedia {
public:
    AVFormatContext *pFormatContext = NULL;
    SwrContext *pSwrContext;
    uint8_t *resampleOutBuffer = NULL;


public:
    VAudio(int audioStramIndex, DZJNICall *pJniCall, PlayerStatus *pPlayerStatus);

    ~VAudio();

    void play() ;

    void privateAnalysisStream(ThreadMode threadMode, AVFormatContext *pStream);

    void release();


    void initCreateOpenSLES();

    int resampleAudio();


};


#endif //COPYAPP_VAUDIO_H
