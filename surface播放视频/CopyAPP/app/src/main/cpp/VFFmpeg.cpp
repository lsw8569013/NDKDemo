//
// Created by Administrator on 2019-07-26.
//

#include <pthread.h>
#include "VFFmpeg.h"


VFFmpeg::VFFmpeg(DZJNICall *pJniCall, const char *url) {
    this->pJniCall = pJniCall;
    this->url = (char *) malloc(strlen(url) + 1);
    // 赋值一份 url ，因为怕外面方法结束销毁了 url
    memcpy(this->url, url, strlen(url) + 1);
    pPlayerStatus = new PlayerStatus;
}

VFFmpeg::~VFFmpeg() {
    release();
}


void VFFmpeg::prepare() {
    prepareMode(THREAD_MAIN);
}

void VFFmpeg::prepareAsync() {
    preparedAsync(THREAD_CHILD);
}


void VFFmpeg::prepareMode(ThreadMode threadMode) {
    // 讲的理念的东西，千万要注意
    // z注册组件
    av_register_all();
    // 初始化网络
    avformat_network_init();

    int formatOpenInputRes = 0;
    int formatFindStreamInfoRes = 0;
    // 打开音频文件
    formatOpenInputRes = avformat_open_input(&pFormatContext, url, NULL, NULL);
    if (formatOpenInputRes != 0) {
        // 打开失败
        // 第一件事，需要回调给 Java 层(下次课讲)
        // 第二件事，需要释放资源
        LOGE("format open input error: %s", av_err2str(formatOpenInputRes));
        callPlayerJniError(threadMode, formatOpenInputRes, av_err2str(formatOpenInputRes));
        return;
    }

    formatFindStreamInfoRes = avformat_find_stream_info(pFormatContext, NULL);
    if (formatFindStreamInfoRes < 0) {
        LOGE("format find stream info error: %s", av_err2str(formatFindStreamInfoRes));
        // 查找视频流失败
        callPlayerJniError(threadMode, formatFindStreamInfoRes,
                           av_err2str(formatFindStreamInfoRes));
        return;
    }


    // 查找音频流的 index
    int audioStramIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_AUDIO, -1,
                                              -1,NULL, 0);
    LOGE("find2 the stream index %d", audioStramIndex);
    if (audioStramIndex < 0) {
        LOGE("format audio stream error.");
        // 这种方式一般不推荐这么写，但是的确方便
        callPlayerJniError(threadMode, FIND_STREAM_ERROR_CODE,
                           (char *) "format audio stream error");
        return;
    }
    pAudio = new VAudio(audioStramIndex, pJniCall, pPlayerStatus);
    LOGE(" new pAudio ok");
    pAudio->analysisStream(threadMode, pFormatContext);

    // -----------------------

    // 查找视频频流的 index
    int videoStramIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_VIDEO, -1,
                                              -1,NULL, 0);
    if (videoStramIndex < 0) {
        LOGE("format videoStramIndex  error.");
        return;
    }
    //
    pVideo = new VVideo(videoStramIndex, pJniCall, pPlayerStatus,pAudio);
//    LOGE(" new video ok");
    pVideo->analysisStream(threadMode, pFormatContext);

    // 回调到 Java 告诉他准备好了
    pJniCall->callPlayerPrepared(threadMode);
}

void VFFmpeg::callPlayerJniError(ThreadMode mode, int code, char *msg) {
    release();
    //回调给java层
    pJniCall->callConnectError(mode, code, msg);
}

void *prepareThreadMethod(void *context) {
    VFFmpeg *pFFmpeg = (VFFmpeg *) context;
    pFFmpeg->prepareMode(THREAD_CHILD);
    return 0;
}

void VFFmpeg::preparedAsync(ThreadMode mode) {
    pthread_t prepareThreadT;
    pthread_create(&prepareThreadT, NULL, prepareThreadMethod, this);
    pthread_detach(prepareThreadT);
}


// 读取packet 线程的方法
void *threadReadPacket(void *context) {
    VFFmpeg *pVFFmpeg = (VFFmpeg *) context;
    while (pVFFmpeg->pPlayerStatus != NULL && !pVFFmpeg->pPlayerStatus->isExit) {
        AVPacket *pPacket = av_packet_alloc();
        if (av_read_frame(pVFFmpeg->pFormatContext, pPacket) >= 0) {
            // 音频 数据
            if (pPacket->stream_index == pVFFmpeg->pAudio->streamIndex) {
                pVFFmpeg->pAudio->pPacketQueue->push(pPacket);
            } else if(pPacket->stream_index == pVFFmpeg->pVideo->streamIndex){
                pVFFmpeg->pVideo->pPacketQueue->push(pPacket);
                // 视频数据
            } else{
                // 1. 解引用数据 data ， 2. 销毁 pPacket 结构体内存  3. pPacket = NULL
                av_packet_free(&pPacket);
            }
        } else {
            // 1. 解引用数据 data ， 2. 销毁 pPacket 结构体内存  3. pPacket = NULL
            av_packet_free(&pPacket);
        }
    }
    return 0;

}

void VFFmpeg::play(ANativeWindow *surface) {
    // 一个线程去读取 Packet
    pthread_t readPacketThreadT;
    pthread_create(&readPacketThreadT, NULL, threadReadPacket, this);
    pthread_detach(readPacketThreadT);

    if (pAudio != NULL) {
        pAudio->play();
    }

    if(pVideo != NULL){
        pVideo->play();
        pVideo->pNativeWindow = surface;
    }
}

void VFFmpeg::release() {
    if (pFormatContext != NULL) {
        avformat_close_input(&pFormatContext);
        avformat_free_context(pFormatContext);
        pFormatContext = NULL;
    }
    avformat_network_deinit();

    if (url != NULL) {
        free(url);
        url = NULL;
    }

    if (pPlayerStatus) {
        delete (pPlayerStatus);
        pPlayerStatus = NULL;
    }
    if (pVideo != NULL) {
        delete (pVideo);
        pVideo = NULL;
    }
    if (pAudio != NULL) {
        delete (pAudio);
        pAudio = NULL;
    }
}

void VFFmpeg::setSurface(jobject surface) {
    if(surface == NULL){
        LOGE("setSurface = null ");
    }
    if (pVideo != NULL) {
        pVideo->setSurface(surface);
    }
}