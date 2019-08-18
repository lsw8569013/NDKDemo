package com.demo.livepush;

import android.os.Handler;
import android.os.Looper;

/**
 * @author liushengwei
 * @description: https://github.com/lsw8569013
 * @date :2019-08-15 16:10
 */
public class LivePush {
    static {
        System.loadLibrary("live-push");
    }

    private String mLiveurl;

    private ConnectListener mConnectListener;

    private Handler mHandler = new Handler(Looper.getMainLooper());


    public void setOnConnectListener(ConnectListener mConnectListener) {
        this.mConnectListener = mConnectListener;
    }

    public LivePush(String url) {
        this.mLiveurl = url;
    }

    public void initConnect() {
        nInitConnect(mLiveurl);
    }


    // C层的回调方法定义
    public void onConnectError(int errorCode, String errorMsg) {
        if(mConnectListener != null){
            mConnectListener.connectError(errorCode,errorMsg);
        }
    }

    public void onConnectSuccess() {
        if(mConnectListener != null){
            mConnectListener.connectSuccess();
        }
    }

    public void stop() {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                nStop();
            }
        });

    }



    public interface ConnectListener {
        void connectError(int errorCode, String errorMsg);

        void connectSuccess();
    }

    private native void nStop() ;
    private native void nInitConnect(String mLiveurl);
}
