//
// Created by lsw on 2019-08-15.
//



#include "LivePush.h"


LivePush::LivePush(const char *mLiveUrl, DZJNICall *pJniCall) {

    this->mLiveUrl = (char *) (malloc(strlen(mLiveUrl) + 1));
    strcpy(this->mLiveUrl, mLiveUrl);
    this->pJniCall = pJniCall;
    pPacketQueue = new PacketQueue();
}

LivePush::~LivePush() {



    if (mLiveUrl != NULL) {
        free(mLiveUrl);
        mLiveUrl = NULL;
    }

    if(pPacketQueue != NULL){
        delete(pPacketQueue);
        pPacketQueue = NULL;
    }

    if(pRtmp != NULL){
        RTMP_Close(pRtmp);
        free(pRtmp);
        pRtmp = NULL;
    }
}

// 子线程
void *initConnectFun(void *context){
    LivePush *pLivePush = static_cast<LivePush *>(context);
    // 1 创建RTMP
    pLivePush->pRtmp = RTMP_Alloc();
    // 2 初始化
    RTMP_Init(pLivePush->pRtmp);
    // 3 设置参数
    pLivePush->pRtmp ->Link.timeout = 10;
    pLivePush->pRtmp ->Link.lFlags |= RTMP_LF_LIVE;
    RTMP_SetupURL(pLivePush->pRtmp,pLivePush->mLiveUrl);
    RTMP_EnableWrite(pLivePush->pRtmp );
    LOGE("开始连接");
    // 开始连接
    if(!RTMP_Connect(pLivePush->pRtmp ,NULL)){
    //回调java层
        LOGE("RTMP_Connect error");
        pLivePush->pJniCall->callConnectError(THREAD_CHILD,RTMP_CONNECT_ERROR_CODE,"RTMP_Connect error");
        return reinterpret_cast<void *>(RTMP_CONNECT_ERROR_CODE);
    }

    if(!RTMP_ConnectStream(pLivePush->pRtmp ,0)){
        //回调java层
        LOGE("RTMP_Connect stream error");
        pLivePush->pJniCall->callConnectError(THREAD_CHILD,RTMP_STREAM_CONNECT_ERROR_CODE,"RTMP_Connect stream error");
        return reinterpret_cast<void *>(RTMP_CONNECT_ERROR_CODE);
    }

    LOGE("RTMP_Connect success");
    pLivePush->pJniCall->callConnectSuccess(THREAD_CHILD);

    return  0;
}

void LivePush::initConnect() {
    // 连接 服务器
    pthread_t initConnectTid;
    pthread_create(&initConnectTid, NULL, initConnectFun, this);

    pthread_detach(initConnectTid);
}
