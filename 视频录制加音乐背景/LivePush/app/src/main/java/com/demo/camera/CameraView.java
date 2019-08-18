package com.demo.camera;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;


import com.demo.opengl.DZGLSurfaceView;
import com.demo.opengl.FboRender;

import java.util.List;

public class CameraView extends DZGLSurfaceView implements CameraRender.RenderListener {
    private CameraHelper mCameraHelper;
    private CameraRender mCameraRender;
    private int mCameraId = Camera.CameraInfo.CAMERA_FACING_FRONT;

    public CameraView(Context context) {
        this(context, null);
    }

    public CameraView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        mCameraRender = new CameraRender(context);
        mCameraHelper = new CameraHelper(context);
        setRenderer(mCameraRender);
        mCameraRender.setOnRenderListener(this);
    }

    @Override
    public void onSurfaceCreated(SurfaceTexture surfaceTexture) {
        mCameraHelper.init(surfaceTexture);
        open(mCameraId);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        mCameraHelper.setViewHeight(getMeasuredHeight());
        mCameraHelper.setViewWidth(getMeasuredWidth());
    }

    public void open(int cameraId) {
        rotateCameraAngle();
        mCameraHelper.open(cameraId);
    }

    /**
     * 旋转相机的角度
     */
    private void rotateCameraAngle() {
        mCameraRender.resetMatrix();
        // 前置摄像头
        if (mCameraId == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            mCameraRender.rotateMatrix(0, 90, 0, 0, 1);
            mCameraRender.rotateMatrix(0, 180, 1, 0, 0);
        }
        // 后置摄像头
        else if (mCameraId == Camera.CameraInfo.CAMERA_FACING_BACK) {
            mCameraRender.rotateMatrix(0, 90, 0, 0, 1);
        }
    }

    public void onDestroy() {
        mCameraHelper.close();
    }

    public int getTextureId() {
        return mCameraRender.getTextureId();
    }
}
