#include <jni.h>
#include <string>
#include "LivePush.h"

JavaVM *pJavaVM = NULL;
LivePush *livePush = NULL;;
DZJNICall *pJniCall = NULL;

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *javaVM, void *reserved) {
    pJavaVM = javaVM;
    JNIEnv *env;
    if (javaVM->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}




extern "C"
JNIEXPORT void JNICALL
Java_com_demo_livepush_LivePush_nInitConnect(JNIEnv *env, jobject instance, jstring mLiveurl_) {
    const char *mLiveurl = env->GetStringUTFChars(mLiveurl_, 0);

    pJniCall = new DZJNICall(pJavaVM, env, instance);
    livePush = new LivePush(mLiveurl, pJniCall);
    LOGE("连接之前 ");
    livePush->initConnect();

    env->ReleaseStringUTFChars(mLiveurl_, mLiveurl);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_demo_livepush_LivePush_nStop(JNIEnv *env, jobject instance) {

    if (livePush != NULL) {
        livePush->stop();
        delete (livePush);
        livePush = NULL;
    }

    if (pJniCall != NULL) {
        delete (pJniCall);
        pJniCall = NULL;
    }


}


extern "C"
JNIEXPORT void JNICALL
Java_com_demo_livepush_LivePush_pushSpsPPs(JNIEnv *env, jobject instance, jbyteArray spsData_,
                                           jint spsLen, jbyteArray ppsData_, jint ppsLen) {
    jbyte *spsData = env->GetByteArrayElements(spsData_, NULL);
    jbyte *ppsData = env->GetByteArrayElements(ppsData_, NULL);

    //

    if (livePush != NULL) {
        livePush->pushSpsPPs(spsData, spsLen, ppsData, ppsLen);
    }

    env->ReleaseByteArrayElements(spsData_, spsData, 0);
    env->ReleaseByteArrayElements(ppsData_, ppsData, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_demo_livepush_LivePush_pushVideo(JNIEnv *env, jobject instance, jbyteArray videoData_,
                                          jint dataLen, jboolean keyFrame) {
    jbyte *videoData = env->GetByteArrayElements(videoData_, NULL);

    if (livePush != NULL) {
        livePush->pushVideo(videoData,dataLen, keyFrame);
    }

    env->ReleaseByteArrayElements(videoData_, videoData, 0);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_demo_livepush_LivePush_pushAudio(JNIEnv *env, jobject instance, jbyteArray data_,
                                          jint dataLen) {
    jbyte *data = env->GetByteArrayElements(data_, NULL);


    if (livePush != NULL) {
        livePush->pushAudio(data,dataLen);
    }

    env->ReleaseByteArrayElements(data_, data, 0);
}