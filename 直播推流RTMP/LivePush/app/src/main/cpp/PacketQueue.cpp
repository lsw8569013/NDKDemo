//
// Created by lsw on 2019-07-26.
//

#include "PacketQueue.h"
#include "DZConstDefine.h"


PacketQueue::PacketQueue() {
    pPacketQueue = new std::queue<RTMPPacket *>();
    // 初始化 锁和条件变量
    pthread_mutex_init(&packetMutex, NULL);
    pthread_cond_init(&packetCond, NULL);
}


void PacketQueue::push(RTMPPacket *pPacket) {
    pthread_mutex_lock(&packetMutex);
    if (pPacketQueue == NULL) {
        LOGE("pPacketQueue error ---- ");
    }
    if (pPacket == NULL) {
        LOGE("pPacket error ---- ");
    }
    pPacketQueue->push(pPacket);
    pthread_cond_signal(&packetCond);
    pthread_mutex_unlock(&packetMutex);
}

RTMPPacket *PacketQueue::pop() {
    RTMPPacket *pPacket = NULL ;
    pthread_mutex_lock(&packetMutex);
    // 如果队列里面是空的
    if (pPacketQueue->empty()) {
        // 等待读取线程
        pthread_cond_wait(&packetCond, &packetMutex);
    }else{
        pPacket = pPacketQueue->front();
        pPacketQueue->pop();
    }

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
    pthread_mutex_lock(&packetMutex);
    while (!pPacketQueue->empty()) {
        RTMPPacket *pPacket = pPacketQueue->front();
        pPacketQueue->pop();
        RTMPPacket_Free(pPacket);
        free(pPacket);
    }
    pthread_mutex_unlock(&packetMutex);
}


void PacketQueue::notify() {
    pthread_mutex_lock(&packetMutex);
    pthread_cond_signal(&packetCond);
    pthread_mutex_unlock(&packetMutex);
}
