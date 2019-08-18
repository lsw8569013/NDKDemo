package com.demo.opengl;

import android.content.Context;

import javax.microedition.khronos.egl.EGLContext;

/**
 * @author liushengwei
 * @description: https://github.com/lsw8569013
 * @date :2019-08-17 13:56
 */
public class DefaultVideoRecorder extends BaseVideoRecored {

    public DefaultVideoRecorder(Context context, EGLContext mEglContext,int textureId) {
        super(context, mEglContext);
        setRenderer(new RecordRender(context,textureId));
    }
}
