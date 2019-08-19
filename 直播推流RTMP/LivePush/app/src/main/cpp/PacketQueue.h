//
// Created by lsw on 2019-07-26.
//

#ifndef COPYAPP_PACKETQUEUE_H
#define COPYAPP_PACKETQUEUE_H

#include <queue>
#include <pthread.h>


extern "C"{
#include "librtmp/rtmp.h"
};


class PacketQueue {
public:
    std::queue<RTMPPacket *> *pPacketQueue;
    // 锁
    pthread_mutex_t packetMutex;
    // 条件变量
    pthread_cond_t packetCond;

public:
    PacketQueue();



    void push(RTMPPacket *pPacket);

    RTMPPacket* pop();

    void clear();

    ~PacketQueue();

    void notify();
};


#endif //COPYAPP_PACKETQUEUE_H
