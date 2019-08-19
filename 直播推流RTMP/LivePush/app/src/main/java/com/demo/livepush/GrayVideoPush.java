package com.demo.livepush;

import android.content.Context;

import com.demo.opengl.RecordRender;

import javax.microedition.khronos.egl.EGLContext;

/**
 * @author liushengwei
 * @description: https://github.com/lsw8569013
 * @date :2019-08-17 13:56
 */
public class GrayVideoPush extends BaseVideoPush {


    private final PushRender pushRender;

    public GrayVideoPush(Context context, EGLContext mEglContext, int textureId) {
        super(context, mEglContext);
        pushRender = new PushRender(context, textureId);
        setRenderer(pushRender);
        pushRender.setFragmentRender(R.raw.filter_fragment_gray);
    }
}
