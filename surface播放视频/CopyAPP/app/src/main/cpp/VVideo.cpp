//
// Created by lsw on 2019-07-27.
//



#include "VVideo.h"


VVideo::VVideo(int streamIndex, DZJNICall *pJniCall, PlayerStatus *pPlayerStatus, VAudio *pAudio)
        : VMedia(
        streamIndex, pJniCall, pPlayerStatus) {
    this->pAudio = pAudio;
}


// 播放线程的方法
void *threadVideoPlay(void *context) {
    VVideo *pVideo = (VVideo *) (context);

//    JNIEnv *env;
//    if (pVideo->pJniCall->javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
//        LOGE("get child thread jniEnv error!");
//        return 0;
//    }

    LOGE("获取窗体 -- start ");
    int dataSize = 0;
    // 1.获取窗体
//    ANativeWindow *pNativeWindow =  ANativeWindow_fromSurface(env,pVideo->surface);

//    pVideo->pJniCall->javaVM->DetachCurrentThread();
    LOGE("获取窗体 OK ");
    // 2.设置缓冲区属性
    ANativeWindow_setBuffersGeometry(pVideo->pNativeWindow, pVideo->pCodecContext->width,
                                     pVideo->pCodecContext->height, WINDOW_FORMAT_RGBA_8888);
    // window 缓冲区buffer
    AVPacket *pPacket = NULL;
    AVFrame *pFrame = av_frame_alloc();

    // window 缓冲区buffer
    ANativeWindow_Buffer outBuffer;

    while (pVideo->pPlayerStatus != NULL && !pVideo->pPlayerStatus->isExit) {
        pPacket = pVideo->pPacketQueue->pop();
        // Packet 包，压缩的数据，解码成 pcm 数据
        int codecSendPacketRes = avcodec_send_packet(pVideo->pCodecContext, pPacket);
        if (codecSendPacketRes == 0) {
            int codecReceiveFrameRes = avcodec_receive_frame(pVideo->pCodecContext, pFrame);
            if (codecReceiveFrameRes == 0) {
                // AVPacket -> AVFrame
                // 渲染 opgles 高效
                // pFrame-> data  yuv420P  需要 RGBA8888
                //
                /* sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
                 const int srcStride[], int srcSliceY, int srcSliceH,
                 uint8_t *const dst[], const int dstStride[]);*/
                sws_scale(pVideo->pSwsContext, (const uint8_t *const *) pFrame->data,
                          pFrame->linesize,
                          0, pVideo->pCodecContext->height,
                          pVideo->pRgbaFrame->data, pVideo->pRgbaFrame->linesize);
//                LOGE("数据推到缓冲区 OK ");

                // 在播放之前，需要判断一下需要休眠多久
                double frameSleepTime = pVideo->getFrameSleepTime(pFrame);
                LOGE("frameSleepTime = %lf", frameSleepTime);
                av_usleep(frameSleepTime * 1000000);




                // 数据推到缓冲区
                ANativeWindow_lock(pVideo->pNativeWindow, &outBuffer, NULL);
                memcpy(outBuffer.bits, pVideo->pFrameBuffer, pVideo->frameSize);
                ANativeWindow_unlockAndPost(pVideo->pNativeWindow);

            }
        }
        // 解引用
        av_packet_unref(pPacket);
        av_frame_unref(pFrame);
    }
    // 1. 解引用数据 data ， 2. 销毁 pPacket 结构体内存  3. pPacket = NULL
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);


    return 0;
}

void VVideo::play() {
    // 一个线程去解码播放

    pthread_t playThreadT;
    pthread_create(&playThreadT, NULL, threadVideoPlay, this);
    pthread_detach(playThreadT);
}

void VVideo::play(ANativeWindow *surface) {

    // 一个线程去解码播放
    pthread_t playThreadT;
    pthread_create(&playThreadT, NULL, threadVideoPlay, this);
    pthread_detach(playThreadT);
}

void VVideo::privateAnalysisStream(ThreadMode threadMode, AVFormatContext *pStream) {

    // 3.初始化 转换上下文
    /*int srcW, int srcH, enum AVPixelFormat srcFormat,
    int dstW, int dstH, enum AVPixelFormat dstFormat,
    int flags, SwsFilter *srcFilter,
            SwsFilter *dstFilter, const double *param*/
    pSwsContext = sws_getContext(pCodecContext->width, pCodecContext->height,
                                 pCodecContext->pix_fmt, pCodecContext->width,
                                 pCodecContext->height,
                                 AV_PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);
    pRgbaFrame = av_frame_alloc();
    frameSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA,
                                         pCodecContext->width, pCodecContext->height, 1);
    pFrameBuffer = (uint8_t *) malloc(frameSize * sizeof(uint8_t));
    av_image_fill_arrays(pRgbaFrame->data, pRgbaFrame->linesize, pFrameBuffer,
                         AV_PIX_FMT_RGBA, pCodecContext->width, pCodecContext->height, 1);

    int num = pFormatCtx->streams[streamIndex]->avg_frame_rate.num;
    int den = pFormatCtx->streams[streamIndex]->avg_frame_rate.den;
    if (den != 0 && num != 0) {
        defaultDelayTime = 1.0f * den / num;
    }
    LOGE("视频一共多少帧 -- %d %lf %d", num / den, defaultDelayTime, den);
}

void VVideo::release() {
    VMedia::release();

    if (pSwsContext != NULL) {
        sws_freeContext(pSwsContext);
        free(pSwsContext);
        pSwsContext = NULL;

    }
    if (pFrameBuffer != NULL) {
        free(pFrameBuffer);
        pFrameBuffer = NULL;
    }
    if (pRgbaFrame != NULL) {
        av_frame_free(&pRgbaFrame);
        pRgbaFrame = NULL;
    }
    // pjniCall 需要再VFFmpeg 之后销毁
    if (pJniCall != NULL) {
        pJniCall->jniEnv->DeleteGlobalRef(surface);
    }
}

VVideo::~VVideo() {
    release();

}

void VVideo::setSurface(jobject surface) {
    if (surface == NULL) {
        LOGE("surface == NULL");
    }
    LOGE("VVideo::setSurface start");
//    LOGE("NewGlobalRef OK ");
//    this->surface = pJniCall->jniEnv->NewGlobalRef(surface);
    this->surface = surface;
    LOGE("VVideo::setSurface OK ");
}

/**
 * 视频同步音频 计算获取需要休眠的时间
 * @param pFrame
 * @return 休眠时间 秒
 */
double VVideo::getFrameSleepTime(AVFrame *pFrame) {
    double times = av_frame_get_best_effort_timestamp(pFrame) * av_q2d(timeBase);

    if (times > currentTime) {
        currentTime = times;
    }
    // 视频 音频相差多少秒
    double diffTme = pAudio->currentTime - currentTime;
    // 视频快了慢一点，视频快了慢一点 ，时间控制在帧率范围左右
    // 1/24 0.016 - -0.016
    LOGE("diffTme =  %lf ", diffTme);
    if(diffTme < 0){
       diffTme =  diffTme * 0.9;
    }else{

    }
    LOGE("diffTme =  %lf ", diffTme);
//    if (diffTme > 0.016 || diffTme < -0.016) {
//        if (diffTme > 0.016) {
//            delayTime = delayTime * 2 / 3;
//        } else if (delayTime < 0.016) {
//            delayTime = delayTime * 3 / 2;
//        }
//
//        LOGE("delayTime =  %lf ", delayTime);
//        LOGE("defaultDelayTime =  %lf ", defaultDelayTime);
//
//        // 第二次控制
//        if (delayTime < defaultDelayTime / 2) {
//            delayTime = defaultDelayTime * 2 / 3;
//        } else if (delayTime > defaultDelayTime * 2) {
//            delayTime = defaultDelayTime * 3 / 2;
//        }
//    }

    //越界处理
//    if (diffTme >= defaultDelayTime * 0.7) {
//        delayTime = 0;
//    } else if (diffTme <= -defaultDelayTime * 0.7) {
//        delayTime = defaultDelayTime * 2;
//    }


    return -diffTme;
}
