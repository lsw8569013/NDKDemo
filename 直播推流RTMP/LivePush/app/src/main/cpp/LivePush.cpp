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

    if (pPacketQueue != NULL) {
        delete (pPacketQueue);
        pPacketQueue = NULL;
    }

    if (pRtmp != NULL) {
        RTMP_Close(pRtmp);
        free(pRtmp);
        pRtmp = NULL;
    }
}

// 子线程
void *initConnectFun(void *context) {
    LivePush *pLivePush = static_cast<LivePush *>(context);
    // 1 创建RTMP
    pLivePush->pRtmp = RTMP_Alloc();
    // 2 初始化
    RTMP_Init(pLivePush->pRtmp);
    // 3 设置参数
    pLivePush->pRtmp->Link.timeout = 10;
    pLivePush->pRtmp->Link.lFlags |= RTMP_LF_LIVE;
    RTMP_SetupURL(pLivePush->pRtmp, pLivePush->mLiveUrl);
    RTMP_EnableWrite(pLivePush->pRtmp);
    LOGE("开始连接");
    // 开始连接
    if (!RTMP_Connect(pLivePush->pRtmp, NULL)) {
        //回调java层
        LOGE("RTMP_Connect error");
        pLivePush->pJniCall->callConnectError(THREAD_CHILD, RTMP_CONNECT_ERROR_CODE,
                                              "RTMP_Connect error");
        return reinterpret_cast<void *>(RTMP_CONNECT_ERROR_CODE);
    }

    if (!RTMP_ConnectStream(pLivePush->pRtmp, 0)) {
        //回调java层
        LOGE("RTMP_Connect stream error");
        pLivePush->pJniCall->callConnectError(THREAD_CHILD, RTMP_STREAM_CONNECT_ERROR_CODE,
                                              "RTMP_Connect stream error");
        return reinterpret_cast<void *>(RTMP_CONNECT_ERROR_CODE);
    }

    LOGE("RTMP_Connect success");
    pLivePush->pJniCall->callConnectSuccess(THREAD_CHILD);

    pLivePush->startTime = RTMP_GetTime();

    while (pLivePush->isPushing){
        // 不断往服务器推数据
        RTMPPacket *pPacket = pLivePush->pPacketQueue->pop();
        if(pPacket != NULL){
            int res = RTMP_SendPacket(pLivePush->pRtmp,pPacket,1);
            LOGE("推送res = %d ",res);
            RTMPPacket_Free(pPacket);
            free(pPacket);
        }
    }

    LOGE("isPushing false stop push ");


    return 0;
}

void LivePush::initConnect() {
    // 连接 服务器

    pthread_create(&initConnectTid, NULL, initConnectFun, this);

//    pthread_detach(initConnectTid);
}

void LivePush::pushSpsPPs(jbyte *spsData, jint spsLen, jbyte *ppsData, jint ppsLen) {
//    flv 格式
    // frame type 1 关键帧 2 非关键帧 4bit
    //CodeID : 7 表示AVC 4bit ,与frame type 结合起来刚好是1个字节 ，0x17
    // fixed : 0x00 0x00 0x00 0x00 4byte
    // configureationVersion 1byte ,0x01版本
    // AVCProfileIndication 1byte ,sps[1]profile
    // profile_compatibility 1byte ,sps[2] compatibility
    // AVCLevelIIndication 1byte ,sps[3] Profile level
    // lengthSizeMinusOne  1byte 0xff 包长数据所使用的字节数

    // sps+pps 的数据
    // sps number  1byte ，0xe1 sps个数
    // sps data length ，2byte sps 长度
    // sps data  , sps 内容
    // pps number ,1byte ，0x01 sps个数
    // pps data length ,2byt pps 长度
    // pps data ，  pps 内容

    // 数据的长度

    int bodysize = spsLen + ppsLen + 16;
    // 构建 RTMPPacket
    RTMPPacket *pPacket = static_cast<RTMPPacket *>(malloc(sizeof(RTMPPacket)));
    RTMPPacket_Alloc(pPacket,bodysize);
    RTMPPacket_Reset(pPacket);

    // 构建body
    char *body = pPacket->m_body;
    int index = 0;
    // frame type 1 关键帧 2 非关键帧 4bit
    //CodeID : 7 表示AVC 4bit ,与frame type 结合起来刚好是1个字节 ，0x17
    body[index++] = 0x17;
    // fixed : 0x00 0x00 0x00 0x00 4byte
    body[index++] = 0x00;
    body[index++] = 0x00;
    body[index++] = 0x00;
    body[index++] = 0x00;
    // configureationVersion 1byte ,0x01版本
    body[index++] = 0x01;
    // AVCProfileIndication 1byte ,sps[1]profile
    body[index++] = spsData[1];
    // profile_compatibility 1byte ,sps[2] compatibility
    body[index++] = spsData[2];
    // AVCLevelIIndication 1byte ,sps[3] Profile level
    body[index++] = spsData[3];
    // lengthSizeMinusOne  1byte 0xff 包长数据所使用的字节数
    body[index++] = 0xff;


    // sps+pps 的数据
    // sps number  1byte ，0xe1 sps个数
    body[index++] = 0xe1;
    // sps data length ，2byte sps 长度
    body[index++] = (spsLen >> 8) & 0xFF;
    body[index++] = spsLen & 0xFF;
    // sps data  , sps 内容
    memcpy(&body[index],spsData,spsLen);
    index+=spsLen;
    // pps number ,1byte ，0x01 sps个数
    body[index++] = 0x01;
    // pps data length ,2byt pps 长度
    body[index++] = (ppsLen >> 8) & 0xFF;
    body[index++] = ppsLen & 0xFF;
    // pps data ，  pps 内容
    memcpy(&body[index],ppsData,ppsLen);

    //RtmpPacket 设置一些信息
    pPacket->m_hasAbsTimestamp = 0;
    pPacket->m_nTimeStamp = 0;
    pPacket->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    pPacket->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    pPacket->m_nBodySize = bodysize;
    pPacket->m_nChannel = 0x04;
    pPacket->m_nInfoField2 = this->pRtmp->m_stream_id;

    pPacketQueue->push(pPacket);
}

void LivePush::pushVideo(jbyte *videoData, jint dataLen, jboolean keyFrame) {
     // frame type 1 关键帧 2 非关键帧 4bit

    //   CodecID 7 表示AVC 4bit ,与frame type 结合起来刚好是1个字节 ，0x17
    // fixed  0x01 0x00 0x00 0x00 4byte ,0x01 表示NALU单元
    // video data Length 4byte, video长度
    // video data

    // 数据的长度
    int bodysize = dataLen + 9;
    // 构建 RTMPPacket
    RTMPPacket *pPacket = static_cast<RTMPPacket *>(malloc(sizeof(RTMPPacket)));
    RTMPPacket_Alloc(pPacket,bodysize);
    RTMPPacket_Reset(pPacket);

    // 构建body
    char *body = pPacket->m_body;
    int index = 0;
    // frame type 1 关键帧 2 非关键帧 4bit
    //CodeID : 7 表示AVC 4bit ,与frame type 结合起来刚好是1个字节 ，0x17
    if(keyFrame){
        body[index++] = 0x17;
    }else{
        body[index++] = 0x27;
    }
    // fixed  0x01 0x00 0x00 0x00 4byte ,0x01 表示NALU单元
    body[index++] = 0x01;
    body[index++] = 0x00;
    body[index++] = 0x00;
    body[index++] = 0x00;

    // video data Length 4byte, video长度
    body[index++] = (dataLen >> 24) & 0xFF;
    body[index++] = (dataLen >> 16) & 0xFF;
    body[index++] = (dataLen >> 8) & 0xFF;
    body[index++] = dataLen & 0xFF;
    // sps data  , sps 内容
    memcpy(&body[index],videoData,dataLen);


    //RtmpPacket 设置一些信息
    pPacket->m_headerType = RTMP_PACKET_SIZE_LARGE;
    pPacket->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    pPacket->m_hasAbsTimestamp = 0;
    pPacket->m_nTimeStamp = RTMP_GetTime()-startTime;
    pPacket->m_nBodySize = bodysize;
    pPacket->m_nChannel = 0x04;
    pPacket->m_nInfoField2 = this->pRtmp->m_stream_id;

    pPacketQueue->push(pPacket);

}

void LivePush::pushAudio(jbyte *data, jint dataLen) {
    // 2字节头信息
    // 前四位表示音频数据格式 AAC 10(A)  1010 A
    // 五六位表示采样率 0 = 5.5K 1 = 11k,2 =22K,3(11) == 44K
    // 七为表示采样的精度  0 = 8bit ,1 = 16bits
    // 八位表示音频类型   0= mono ,1 = stereo
    // 我们这里算出来第一个字节 是0xAF
    // 1011 11


    // 数据的长度
    int bodysize = dataLen + 2;
    // 构建 RTMPPacket
    RTMPPacket *pPacket = static_cast<RTMPPacket *>(malloc(sizeof(RTMPPacket)));
    RTMPPacket_Alloc(pPacket,bodysize);
    RTMPPacket_Reset(pPacket);

    // 构建body
    char *body = pPacket->m_body;
    int index = 0;
    //我们这里算出来第一个字节 是0xAF
    body[index++] = 0xAF;
    // 0x01 代表aac 原始数据
    body[index++] = 0x01;


    // sps data  , sps 内容
    memcpy(&body[index],data,dataLen);


    //RtmpPacket 设置一些信息
    pPacket->m_headerType = RTMP_PACKET_SIZE_LARGE;
    pPacket->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    pPacket->m_hasAbsTimestamp = 0;
    pPacket->m_nTimeStamp = RTMP_GetTime()-startTime;
    pPacket->m_nBodySize = bodysize;
    pPacket->m_nChannel = 0x04;
    pPacket->m_nInfoField2 = this->pRtmp->m_stream_id;

    pPacketQueue->push(pPacket);

}

void LivePush::stop() {
    LOGE("等待停止 start ");
    isPushing  = false;
    LOGE("stop isPushing ");
    pPacketQueue->notify();
    pthread_join(  initConnectTid,0);
    LOGE("等待停止 ");
}
