package com.demo.opengl;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public interface IFboRender {
    void onBindFbo();

    void onUnbindFbo();

    void onSurfaceCreated(int viewWidth, int viewHeight);

    void onSurfaceChanged(int width, int height);

    void onDrawFrame();
}
