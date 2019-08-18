package com.darren.media;

import android.text.TextUtils;
import android.util.Log;

import com.darren.media.listener.MediaErrorListener;
import com.darren.media.listener.MediaInfoListener;
import com.darren.media.listener.MediaPreparedListener;

/**
 * Created by hcDarren on 2019/6/15.
 * 音频播放器的逻辑处理类
 */
public class DarrenPlayer {
    static {
        System.loadLibrary("music-player");
    }

    /**
     * url 可以是本地文件路径，也可以是 http 链接
     */
    private String url;

    private MediaErrorListener mErrorListener;

    private MediaPreparedListener mPreparedListener;
    private MediaInfoListener mMediaInfoListener;

    public void setOnErrorListener(MediaErrorListener errorListener) {
        this.mErrorListener = errorListener;
    }

    public void setOnPreparedListener(MediaPreparedListener preparedListener) {
        this.mPreparedListener = preparedListener;
    }

    public void setOnInfoListener(MediaInfoListener mMediaInfoListener) {
        this.mMediaInfoListener = mMediaInfoListener;
    }

    private void musicInfo(int samplezRate, int channels) {
        if (mMediaInfoListener != null) {
            mMediaInfoListener.musicInfo(samplezRate, channels);
        }
    }

    private void callbackPcm(byte[] pcmData, int size) {
        if (mMediaInfoListener != null) {
            mMediaInfoListener.callbackPcm(pcmData, size);
        }
    }



    // called from jni
    private void onError(int code, String msg) {
        if (mErrorListener != null) {
            mErrorListener.onError(code, msg);
        }
    }

    // called from jni
    private void onPrepared() {
        if (mPreparedListener != null) {
            mPreparedListener.onPrepared();
        }
    }

    public void setDataSource(String url) {
        this.url = url;
    }

    public void play() {
        if (TextUtils.isEmpty(url)) {
            throw new NullPointerException("url is null, please call method setDataSource");
        }
        nPlay();
    }

    private native void nPlay();

    public void prepare() {
        if (TextUtils.isEmpty(url)) {
            throw new NullPointerException("url is null, please call method setDataSource");
        }
        nPrepare(url);
    }

    public void stop() {
        nStop();
    }

    private native void nStop();

    /**
     * 异步准备
     */
    public void prepareAsync() {
        if (TextUtils.isEmpty(url)) {
            throw new NullPointerException("url is null, please call method setDataSource");
        }
        nPrepareAsync(url);
    }

    private native void nPrepareAsync(String url);

    private native void nTest(byte[] url);


    private native void nPrepare(String url);
}
