package com.darren.media.listener;

/**
 * @author liushengwei
 * @description: https://github.com/lsw8569013
 * @date :2019-08-18 11:28
 */
public interface MediaInfoListener {
    void musicInfo(int samplezRate, int channels);

    void callbackPcm(byte[] pcmData, int size);
}
