//
// Created by 曾辉 on 2019-07-03.
//

#ifndef NDK_DAY03_DZCONSTDEFINE_H
#define NDK_DAY03_DZCONSTDEFINE_H
#include <android/log.h>

#define TAG "JNI_MUSIC_LSW"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

// ---------- 错误码 start ----------
#define RTMP_CONNECT_ERROR_CODE -0x10
#define RTMP_STREAM_CONNECT_ERROR_CODE -0x11
// ---------- 错误码 end ----------


// ---------- 播放错误码 start ----------
#define FIND_STREAM_ERROR_CODE -0x10
#define CODEC_FIND_DECODER_ERROR_CODE -0x11
#define CODEC_ALLOC_CONTEXT_ERROR_CODE -0x12
#define SWR_ALLOC_SET_OPTS_ERROR_CODE -0x13
#define SWR_CONTEXT_INIT_ERROR_CODE -0x14
// ---------- 播放错误码 end ----------

#define AUDIO_SAMPLE_RATE  44100;

#endif //NDK_DAY03_DZCONSTDEFINE_