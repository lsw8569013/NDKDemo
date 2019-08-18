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

    pJniCall = new DZJNICall(pJavaVM,env,instance);
    livePush = new LivePush(mLiveurl,pJniCall);
    LOGE("连接之前 ");
    livePush->initConnect();

    env->ReleaseStringUTFChars(mLiveurl_, mLiveurl);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_demo_livepush_LivePush_nStop(JNIEnv *env, jobject instance) {

    if(livePush != NULL){
        delete(livePush);
        livePush = NULL;
    }

    if(pJniCall != NULL){
        delete (pJniCall);
        pJniCall = NULL;
    }


}