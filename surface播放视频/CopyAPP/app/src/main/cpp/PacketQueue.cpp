//
// Created by lsw on 2019-07-26.
//

#include "PacketQueue.h"
#include "DZConstDefine.h"

PacketQueue::PacketQueue() {
    pPacketQueue = new std::queue<AVPacket *>;
    // 初始化 锁和条件变量
    pthread_mutex_init(&packetMutex, NULL);
    pthread_cond_init(&packetCond, NULL);
}


void PacketQueue::push(AVPacket *pPacket) {
    pthread_mutex_lock(&packetMutex);
    if(pPacketQueue == NULL ){
        LOGE("pPacketQueue error ---- ");
    }
    if(pPacket == NULL ){
        LOGE("pPacket error ---- ");
    }
    pPacketQueue->push(pPacket);
    pthread_cond_signal(&packetCond);
    pthread_mutex_unlock(&packetMutex);
}

AVPacket *PacketQueue::pop() {
    AVPacket *pPacket;
    pthread_mutex_lock(&packetMutex);
    // 如果队列里面是空的
    while (pPacketQueue->empty()){
        // 等待读取线程
        pthread_cond_wait(&packetCond,&packetMutex);
    }
    pPacket = pPacketQueue->front();
    pPacketQueue->pop();
    pthread_mutex_unlock(&packetMutex);
    return pPacket;
}

PacketQueue::~PacketQueue() {

    if (pPacketQueue != NULL) {
        clear();
        delete (pPacketQueue);
        pPacketQueue = NULL;
        pthread_mutex_destroy(&packetMutex);
        pthread_cond_destroy(&packetCond);
    }

}

void PacketQueue::clear() {

    while(!pPacketQueue->empty()){
        pPacketQueue->pop();
    }

}
