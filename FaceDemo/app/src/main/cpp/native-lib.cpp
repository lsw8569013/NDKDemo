#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>
#include <android/log.h>


#define TAG "JNI_TAG"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

using namespace std;
using namespace cv;

// -------------------------- 方法声明

extern "C"
{
void bitmap2Mat(JNIEnv *pEnv, Mat &mat, jobject pJobject);

JNIEXPORT void JNICALL
Java_face_com_facedemo_FaceDetation_faceDetectionSaveInfo(JNIEnv *env, jobject instance,jobject bitmap,jstring faceFileName_);

void mat2Bitmap(JNIEnv *env, Mat mat, jobject bitmap);


JNIEXPORT void JNICALL
Java_face_com_facedemo_FaceDetation_loadPath(JNIEnv *env, jobject instance, jstring filePath_);
}

// -------------------------- 方法声明 end --------------



CascadeClassifier cascadeClassifier;

extern "C"
JNIEXPORT void JNICALL
Java_face_com_facedemo_FaceDetation_loadPath(JNIEnv *env, jobject instance, jstring filePath_) {
    const char *filePath = env->GetStringUTFChars(filePath_, 0);

    //
    cascadeClassifier.load(filePath);
    LOGE("加载分类器成功");


    env->ReleaseStringUTFChars(filePath_, filePath);
}



extern "C"
JNIEXPORT void JNICALL
Java_face_com_facedemo_FaceDetation_faceDetectionSaveInfo(JNIEnv *env, jobject instance,jobject bitmap,jstring faceFileName_) {
    const char *faceFileName = env->GetStringUTFChars(faceFileName_, 0);
    //  检查人脸 ARGB888 =  8UC4  RGB565 = 8UC2
    Mat mat;
    bitmap2Mat(env,mat,bitmap);

    // 处理灰度图
    Mat gray;
    cvtColor(mat,gray,COLOR_BGRA2GRAY);

    //直方均衡补偿
    Mat equalize_mat;
    equalizeHist(gray,equalize_mat);

    //识别人脸 CascadeClassifier 人脸分类器文件
    std::vector<Rect> faces;

    cascadeClassifier.detectMultiScale(equalize_mat,faces,1.1,5);

    LOGE("face count %d",faces.size());

//    if(faces.size() == 1){
        Rect faceRect = faces[0];

        //人脸边框显示
        rectangle(equalize_mat,faceRect,Scalar(255,0,0),6);

        // 保存人脸信息 mat
        Mat face_info_mat(equalize_mat,faceRect);

        //imwrite( const String& filename, InputArray img, const std::vector<int>& params = std::vector<int>());

        vector<int> compression_params;
        compression_params.push_back(IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(9);

        try {
            imwrite(faceFileName, face_info_mat,compression_params);
        }
        catch (cv::Exception& ex) {
            LOGE( "Exception converting image to PNG format: %s\n", ex.what());

        }
//    }

    mat2Bitmap(env,equalize_mat,bitmap);
//    mat2Bitmap(env,mat,bitmap);

    //保存人脸信息

}



void mat2Bitmap(JNIEnv *env, Mat mat, jobject bitmap) {
    AndroidBitmapInfo bitmapInfo;

    AndroidBitmap_getInfo(env,bitmap,&bitmapInfo);

    void* pixels;
    //bitmap lock 画布
    AndroidBitmap_lockPixels(env,bitmap,&pixels);


    if(bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888){
        Mat temp(bitmapInfo.height,bitmapInfo.width,CV_8UC4,pixels);
        if(mat.type() == CV_8UC4){
            mat.copyTo(temp);
        }else
        if(mat.type() == CV_8UC2){
            cvtColor(mat,temp,COLOR_BGR5652RGBA);

        }else
        if(mat.type() == CV_8UC1){
            cvtColor(mat,temp,COLOR_GRAY2BGRA);
        }
        
    }else if(bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565){
        Mat temp(bitmapInfo.height,bitmapInfo.width,CV_8UC2,pixels);
        if(mat.type() == CV_8UC4){
            cvtColor(mat,temp,COLOR_RGBA2BGR565);
        }else
        if(mat.type() == CV_8UC2){
            mat.copyTo(temp);
        }else
        if(mat.type() == CV_8UC1){
            cvtColor(mat,temp,COLOR_GRAY2BGR565);
        }
    }

    //其他需要自己去转

    AndroidBitmap_unlockPixels(env,bitmap);

}

void bitmap2Mat(JNIEnv *env, Mat &mat, jobject bitmap) {
    //ARGB888 =  8UC4  RGB565 = 8UC2

    AndroidBitmapInfo bitmapInfo;

    AndroidBitmap_getInfo(env,bitmap,&bitmapInfo);

    void* pixels;
    //bitmap lock 画布
    AndroidBitmap_lockPixels(env,bitmap,&pixels);

    mat.create(bitmapInfo.height,bitmapInfo.width,CV_8UC4);


    if(bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888){
        Mat temp(bitmapInfo.height,bitmapInfo.width,CV_8UC4,pixels);
        //temp copy to mat
        temp.copyTo(mat);
    }else if(bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565){
        Mat temp(bitmapInfo.height,bitmapInfo.width,CV_8UC2,pixels);
        cvtColor(temp,mat,COLOR_BGR5552RGBA);
    }

    //其他需要自己去转

    AndroidBitmap_unlockPixels(env,bitmap);



}
