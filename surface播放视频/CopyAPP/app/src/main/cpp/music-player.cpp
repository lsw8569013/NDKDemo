#include <jni.h>
#include <malloc.h>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "VFFmpeg.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>


// 在 C++ 中兼容C
extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}


JavaVM *pJavaVM = NULL;
DZJNICall *pJniCall = NULL;
VFFmpeg *pVFFmpeg = NULL;




jobject initCreateAudioTrack(JNIEnv *env) {

    int steamType = 3;

    int sampleRateInHz = AUDIO_SAMPLE_RATE;

    int channelConfig = (0x04 | 0x08);

    int audioFormat = 2;


    int mode = 0;


    jclass jAudioTrakClass = env->FindClass("android/media/AudioTrack");

    jmethodID getMiniBufferSizeMid = env->GetStaticMethodID(jAudioTrakClass, "getMinBufferSize",
                                                            "(III)I");


    int bufferSizeInBytes = env->CallStaticIntMethod(jAudioTrakClass, getMiniBufferSizeMid,
                                                     sampleRateInHz, channelConfig, audioFormat);

    /*public AudioTrack(int streamType, int sampleRateInHz, int channelConfig,
            int audioFormat, int bufferSizeInBytes, int mode) */
//    jmethodID jAudioTrackCMid;
    jmethodID jAudioTrackCMid = env->GetMethodID(jAudioTrakClass, "<init>", "(IIIIII)V");
    jobject jAudioTrack = env->NewObject(jAudioTrakClass, jAudioTrackCMid,
                                         steamType, sampleRateInHz, channelConfig, audioFormat,
                                         bufferSizeInBytes, mode);


    jmethodID playMid = env->GetMethodID(jAudioTrakClass, "play", "()V");
    // 调用 play 方法
    env->CallVoidMethod(jAudioTrakClass, playMid);

    return jAudioTrack;

}


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
Java_music_VPlayer_nPlay(JNIEnv *env, jobject instance,jobject surface) {
    if (pVFFmpeg != NULL) {
        ANativeWindow *pNativeWindow =  ANativeWindow_fromSurface(env,surface);
        pVFFmpeg->play(pNativeWindow);
        LOGE("ndk player -- ");
    }
}



extern "C"
JNIEXPORT void JNICALL
Java_music_VPlayer_nPrepared(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);

    if (pVFFmpeg == NULL) {
        pJniCall = new DZJNICall(pJavaVM, env, instance);
        pVFFmpeg = new VFFmpeg(pJniCall, url);
        pVFFmpeg->prepare();
    }


    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_music_VPlayer_nPrepareAsync(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    if (pVFFmpeg == NULL) {
        pJniCall = new DZJNICall(pJavaVM, env, instance);
        pVFFmpeg = new VFFmpeg(pJniCall, url);
        pVFFmpeg->prepareAsync();
    }
    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_music_MainActivity_decodeVideo(JNIEnv *env, jobject instance, jstring url_,jobject surface) {
    const char *url = env->GetStringUTFChars(url_, 0);
    AVFormatContext *pFormatContext = NULL;
    AVPacket *pPacket = NULL;
    AVFrame *pFrame = NULL;
    AVCodecContext *pCodecContext = NULL;
    int index = -1;

    // 解码视频

    // 讲的理念的东西，千万要注意
    // z注册组件
    av_register_all();
    // 初始化网络
    avformat_network_init();

    int formatOpenInputRes = 0;
    int formatFindStreamInfoRes = 0;
    // 打开音频文件
    formatOpenInputRes = avformat_open_input(&pFormatContext, url, NULL, NULL);
    if (formatOpenInputRes != 0) {
        // 打开失败
        // 第一件事，需要回调给 Java 层(下次课讲)
        // 第二件事，需要释放资源
        LOGE("format open input error: %s", av_err2str(formatOpenInputRes));

        return;
    }

    formatFindStreamInfoRes = avformat_find_stream_info(pFormatContext, NULL);
    if (formatFindStreamInfoRes < 0) {
        LOGE("format find stream info error: %s", av_err2str(formatFindStreamInfoRes));
        // 查找视频流失败

        return;
    }

    // 查找视频流的 index
    int videoStramIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_VIDEO, -1,
                                              -1,
                                              NULL, 0);
    if (videoStramIndex < 0) {
        LOGE("format audio stream error.");
        // 这种方式一般不推荐这么写，但是的确方便

        return;
    }
    // 查找解码
    AVCodecParameters *pCodecParameters = pFormatContext->streams[videoStramIndex]->codecpar;
    AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (pCodec == NULL) {
        LOGE("codec find audio decoder error");

        return;
    }

    // 打开解码器
    pCodecContext = avcodec_alloc_context3(pCodec);
    if (pCodecContext == NULL) {
        LOGE("codec alloc context error");
        return;
    }
    int codecParametersToContextRes = avcodec_parameters_to_context(pCodecContext,
                                                                    pCodecParameters);
    if (codecParametersToContextRes < 0) {
        LOGE("codec parameters to context error: %s", av_err2str(codecParametersToContextRes));
        return;
    }

    int codecOpenRes = avcodec_open2(pCodecContext, pCodec, NULL);
    if (codecOpenRes != 0) {
        LOGE("codec audio open error: %s", av_err2str(codecOpenRes));
        return;
    }

    // 1.获取窗体
    ANativeWindow *pNativeWindow =  ANativeWindow_fromSurface(env,surface);
    // 2.设置缓冲区属性
    ANativeWindow_setBuffersGeometry(pNativeWindow,pCodecContext->width,
            pCodecContext->height,WINDOW_FORMAT_RGBA_8888);
    // window 缓冲区buffer
    ANativeWindow_Buffer outBuffer;



    pPacket = av_packet_alloc();
    pFrame = av_frame_alloc();

    while (av_read_frame(pFormatContext,pPacket) >=0){
        if(pPacket->stream_index == videoStramIndex){
            int codeSendPacketRes = avcodec_send_packet(pCodecContext,pPacket);
            if(codeSendPacketRes == 0){
                int codecReceiveFrameRes = avcodec_receive_frame(pCodecContext,pFrame);
                if(codecReceiveFrameRes == 0){
                    index++ ;
                    LOGE("解码视频 %d",index);
                    // 渲染 opgles 高效
                    // pFrame-> data  yuv420P  需要 RGBA8888
                    //
                   /* sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
                    const int srcStride[], int srcSliceY, int srcSliceH,
                    uint8_t *const dst[], const int dstStride[]);*/





                }
            }
        }
    }
    av_packet_unref(pPacket);
    av_frame_unref(pFrame);

    av_packet_free(&pPacket);
    av_frame_free(&pFrame);

    if(pCodecContext != NULL){
        avcodec_close(pCodecContext);
        avcodec_free_context(&pCodecContext);
        pCodecContext = NULL;
    }

    if(pFormatContext != NULL){
        avformat_close_input(&pFormatContext);
        avformat_free_context(pFormatContext);
        pFormatContext = NULL;
    }
    avformat_network_deinit();

    env->ReleaseStringUTFChars(url_, url);
}


extern "C"
JNIEXPORT void JNICALL
Java_music_VPlayer_setSurface(JNIEnv *env, jobject instance, jobject surface) {

    if(pVFFmpeg != NULL){
        pVFFmpeg->setSurface(surface);
    }

}