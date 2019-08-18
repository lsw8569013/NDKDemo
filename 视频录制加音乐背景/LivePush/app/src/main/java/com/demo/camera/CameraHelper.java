package com.demo.camera;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.util.Log;

import java.io.IOException;
import java.util.List;

final class CameraHelper {
    private SurfaceTexture mSurfaceTexture;
    private Camera mCamera;
    private int mViewWidth;
    private int mViewHeight;
    private Context mContext;

    public CameraHelper(Context context) {

    }

    public void setViewHeight(int viewHeight) {
        this.mViewHeight = viewHeight;
    }

    public void setViewWidth(int viewWidth) {
        this.mViewWidth = viewWidth;
    }

    public void init(SurfaceTexture surfaceTexture) {
        this.mSurfaceTexture = surfaceTexture;
    }

    public void open(int cameraId) {
        try {
            close();
            mCamera = Camera.open(cameraId);
            mCamera.setPreviewTexture(mSurfaceTexture);

            Camera.Parameters parameters = mCamera.getParameters();

            parameters.setFlashMode("off");
            parameters.setPreviewFormat(ImageFormat.NV21);

            // 直接设置 View 的大小，会有一定的兼容性问题，怎么拿合适的需要根据具体场景来选择
            Camera.Size size = getFitSize(parameters.getSupportedPictureSizes());
            parameters.setPictureSize(size.width, size.height);
            size = getFitSize(parameters.getSupportedPreviewSizes());
            parameters.setPreviewSize(size.width, size.height);

            mCamera.setParameters(parameters);

            mCamera.startPreview();
            mCamera.autoFocus(null);

            Log.e("TAG", "开始预览相机：" + cameraId);
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    public void close() {
        if (mCamera != null) {
            mCamera.stopPreview();
            mCamera.release();
            Log.e("TAG", "停止预览相机");
        }
    }

    public void change(int cameraId) {
        close();
        open(cameraId);
    }

    private Camera.Size getFitSize(List<Camera.Size> sizes) {
        if (mViewWidth < mViewHeight) {
            int t = mViewHeight;
            mViewHeight = mViewWidth;
            mViewWidth = t;
        }

        for (Camera.Size size : sizes) {
            if (size.width >= mViewWidth && size.height >= mViewHeight) {
                return size;
            }
        }
        return sizes.get(0);
    }
}
