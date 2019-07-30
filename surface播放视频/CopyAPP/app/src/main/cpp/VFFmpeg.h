//
// Created by Administrator on 2019-07-26.
//

#ifndef COPYAPP_VFFMPEG_H
#define COPYAPP_VFFMPEG_H

#include "DZJNICall.h"
#include "DZConstDefine.h"
#include "VAudio.h"
#include "VVideo.h"

extern "C"{
#include "libswresample/swresample.h"
#include "libavformat/avformat.h"
};

class VFFmpeg {
public:

    DZJNICall *pJniCall = NULL;
    AVFormatContext *pFormatContext = NULL;
    char *url = NULL;
    VAudio *pAudio = NULL;
    PlayerStatus *pPlayerStatus = NULL;
    VVideo *pVideo = NULL;


public:
    VFFmpeg(DZJNICall *pJniCall, const char* url);


    void prepare();

    void release();

    void prepareMode(ThreadMode mode);

    void callPlayerJniError(ThreadMode mode, int code, char *msg);

    void prepareAsync();

    void preparedAsync(ThreadMode mode);

    void play(ANativeWindow *surface);

    ~VFFmpeg();

    void setSurface(jobject pJobject);


};


#endif //COPYAPP_VFFMPEG_H
