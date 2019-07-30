//
// Created by lsw on 2019-07-26.
//

#ifndef COPYAPP_PACKETQUEUE_H
#define COPYAPP_PACKETQUEUE_H

#include <queue>
#include <pthread.h>
extern "C"{
#include "libavcodec/avcodec.h"
};


class PacketQueue {
public:
    std::queue<AVPacket *> *pPacketQueue;
    // 锁
    pthread_mutex_t packetMutex;
    // 条件变量
    pthread_cond_t packetCond;

public:
    PacketQueue();



    void push(AVPacket *pPacket);

    AVPacket* pop();

    void clear();

    ~PacketQueue();
};


#endif //COPYAPP_PACKETQUEUE_H
