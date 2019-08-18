//
// Created by  on 2019/6/16.
//

#ifndef MUSICPLAYER_DZJNICALL_H
#define MUSICPLAYER_DZJNICALL_H

#include <jni.h>

enum ThreadMode{
    THREAD_CHILD,THREAD_MAIN
};

class DZJNICall {
public:
    JavaVM *javaVM;
    JNIEnv *jniEnv;
    jmethodID jPlayerErrorMid;
    jmethodID jPlayerPreparedMid;
    jmethodID jPlayerMusicInfoMid;
    jmethodID jPlayercallBackPcmMid;
    jobject jPlayerObj;
public:
    DZJNICall(JavaVM *javaVM, JNIEnv *jniEnv, jobject jPlayerObj);
    ~DZJNICall();

public:
    void callPlayerError(ThreadMode threadMode,int code, char *msg);

    void callPlayerPrepared(ThreadMode mode);

    void callMusicInfo(ThreadMode mode, int sampleRate, int channels);


    void callCallBackPcm(ThreadMode mode, uint8_t *string, int i);



};


#endif //MUSICPLAYER_DZJNICALL_H
