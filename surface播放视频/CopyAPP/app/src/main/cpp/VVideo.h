//
// Created by lsw on 2019-07-27.
//

#ifndef COPYAPP_VVIDEO_H
#define COPYAPP_VVIDEO_H


#include "VMedia.h"
#include "VAudio.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>


extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}


class VVideo : public VMedia {
public:
    SwsContext *pSwsContext = NULL;
    AVFrame *pRgbaFrame = NULL;
    uint8_t *pFrameBuffer = NULL;
    int frameSize;
    jobject surface;
    ANativeWindow *pNativeWindow = NULL;
    VAudio *pAudio = NULL;

    /**
     * 视频延时时间
     */
    double delayTime = 0;
    /**
     * 默认情况下最合适的延迟时间,动态获取
     */
    double defaultDelayTime = 0.04;

public:
    VVideo(int streamIndex, DZJNICall *pJniCall, PlayerStatus *pPlayerStatus,VAudio *pAudio);

    ~VVideo();

    void play();



    void privateAnalysisStream(ThreadMode threadMode, AVFormatContext *pStream);

    void release();


    void setSurface(jobject surface);

    void play(ANativeWindow *surface);

    double getFrameSleepTime(AVFrame *pFrame);
};


#endif //COPYAPP_VVIDEO_H
