package com.demo.livepush;

import android.content.Context;


import com.demo.opengl.BaseVideoRecored;
import com.demo.opengl.RecordRender;

import javax.microedition.khronos.egl.EGLContext;

/**
 * @author liushengwei
 * @description: https://github.com/lsw8569013
 * @date :2019-08-17 13:56
 */
public class DefaultVideoPush extends BaseVideoPush {

    public DefaultVideoPush(Context context, EGLContext mEglContext, int textureId) {
        super(context, mEglContext);
        setRenderer(new RecordRender(context,textureId));
    }
}
