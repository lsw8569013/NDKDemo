//
// Created by lsw on 2019-07-28.
//

#include "VMedia.h"

VMedia::VMedia(int streamIndex, DZJNICall *pJniCall, PlayerStatus *pPlayerStatus) {
    this->streamIndex = streamIndex;
    this->pJniCall = pJniCall;
    this->pPlayerStatus = pPlayerStatus;
    pPacketQueue = new PacketQueue();
}

void VMedia::analysisStream(ThreadMode mode, AVFormatContext *pFormatCtx) {
    publicAnalysisStream(mode, pFormatCtx);
    privateAnalysisStream(mode, pFormatCtx);
}

void VMedia::release() {

    if (pPacketQueue) {
        delete (pPacketQueue);
        pPacketQueue = NULL;
    }


    if (pCodecContext != NULL) {
        avcodec_close(pCodecContext);
        avcodec_free_context(&pCodecContext);
        pCodecContext = NULL;
    }
}

VMedia::~VMedia() {
    release();
}



void VMedia::publicAnalysisStream(ThreadMode threadMode, AVFormatContext *pFormatCtx) {
    this->pFormatCtx = pFormatCtx;
    // 查找解码
    AVCodecParameters *pCodecParameters = pFormatCtx->streams[streamIndex]->codecpar;
    AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (pCodec == NULL) {
        LOGE("codec find audio decoder error");
        callPlayerJniError(threadMode, CODEC_FIND_DECODER_ERROR_CODE,
                           "codec find audio decoder error");
        return;
    }
    // 打开解码器
    pCodecContext = avcodec_alloc_context3(pCodec);
    if (pCodecContext == NULL) {
        LOGE("codec alloc context error");
        callPlayerJniError(threadMode, CODEC_ALLOC_CONTEXT_ERROR_CODE, "codec alloc context error");
        return;
    }
    int codecParametersToContextRes = avcodec_parameters_to_context(pCodecContext,
                                                                    pCodecParameters);
    if (codecParametersToContextRes < 0) {
        LOGE("codec parameters to context error: %s", av_err2str(codecParametersToContextRes));
        callPlayerJniError(threadMode, codecParametersToContextRes,
                           av_err2str(codecParametersToContextRes));
        return;
    }

    int codecOpenRes = avcodec_open2(pCodecContext, pCodec, NULL);
    if (codecOpenRes != 0) {
        LOGE("codec audio open error: %s", av_err2str(codecOpenRes));
        callPlayerJniError(threadMode, codecOpenRes, av_err2str(codecOpenRes));
        return;
    }

    duration = pFormatCtx->duration;
    timeBase = pFormatCtx->streams[streamIndex]->time_base;

}
void VMedia::callPlayerJniError(ThreadMode mode, int code, char *msg) {
    release();
    pJniCall->callConnectError(mode, code, msg);
}


