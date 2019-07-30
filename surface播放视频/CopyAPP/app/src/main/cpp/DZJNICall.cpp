//
// Created by 曾辉 on 2019-07-03.
//

#include "DZJNICall.h"

/**
 *
 * @param javaVM
 * @param jniEnv
 * @param jCallObj jni的java调用类
 */
DZJNICall::DZJNICall(JavaVM *javaVM, JNIEnv *jniEnv, jobject jCallObj) {
    this->javaVM = javaVM;
    this->jniEnv = jniEnv;
    this->jCallObj = jniEnv->NewGlobalRef(jCallObj);

    jclass jPlayerClass = jniEnv->GetObjectClass(jCallObj);
//    jConnectErrorMid = jniEnv->GetMethodID(jPlayerClass, "onConnectError",
//                                           "(ILjava/lang/String;)V");
//    jConnectSuccessMid = jniEnv->GetMethodID(jPlayerClass, "onConnectSuccess", "()V");
    jPlayerPrepardMid = jniEnv->GetMethodID(jPlayerClass, "onPrepared", "()V");

}

DZJNICall::~DZJNICall() {
    jniEnv->DeleteGlobalRef(jCallObj);
}

void DZJNICall::callConnectError(ThreadMode threadMode, int code, char *msg) {

    if (threadMode == THREAD_MAIN) {
        jstring jMsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jCallObj, jConnectErrorMid, code, jMsg);
        jniEnv->DeleteLocalRef(jMsg);
    } else if (threadMode == THREAD_CHILD) {
        // 子线程用不了主线程 jniEnv （native 线程）
        // 子线程是不共享 jniEnv ，他们有自己所独有的
        // 获取当前线程的 JNIEnv， 通过 JavaVM
        // 创建子线程 env
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }

        jstring jMsg = env->NewStringUTF(msg);
        env->CallVoidMethod(jCallObj, jConnectErrorMid, code, jMsg);
        env->DeleteLocalRef(jMsg);

        javaVM->DetachCurrentThread();
    }
}
/*

void DZJNICall::callConnectSuccess(ThreadMode threadMode) {
    // 子线程用不了主线程 jniEnv （native 线程）
    // 子线程是不共享 jniEnv ，他们有自己所独有的
    if (threadMode == THREAD_MAIN) {
        jniEnv->CallVoidMethod(jCallObj, jConnectSuccessMid);
    } else if (threadMode == THREAD_CHILD) {
        // 获取当前线程的 JNIEnv， 通过 JavaVM
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }
        env->CallVoidMethod(jCallObj, jConnectSuccessMid);
        javaVM->DetachCurrentThread();
    }
}
*/

void DZJNICall::callPlayerPrepared(ThreadMode mode) {
    // 子线程用不了主线程 jniEnv （native 线程）
    // 子线程是不共享 jniEnv ，他们有自己所独有的
    if (mode == THREAD_MAIN) {
        jniEnv->CallVoidMethod(jCallObj, jPlayerPrepardMid);
    } else if (mode == THREAD_CHILD) {
        // 获取当前线程的 JNIEnv， 通过 JavaVM
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }
        env->CallVoidMethod(jCallObj, jPlayerPrepardMid);
        javaVM->DetachCurrentThread();
    }
}
