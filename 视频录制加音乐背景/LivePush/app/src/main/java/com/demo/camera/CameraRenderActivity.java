package com.demo.camera;

import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;


import com.demo.livepush.R;
import com.demo.opengl.DefaultVideoRecorder;


public class CameraRenderActivity extends AppCompatActivity {
    private CameraView mCameraView;
    private CameraButton cbtn;

    private DefaultVideoRecorder mVideoRecorder;

    @Override
    protected void onResume() {
        super.onResume();
        mCameraView.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mCameraView.onPause();
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera_render);


        mCameraView = findViewById(R.id.surface_view);
        cbtn = findViewById(R.id.recorder_btn);
        cbtn.setOnLongClickListener(new CameraButton.OnLongClickListener() {
            @Override
            public void onLongClickStart() {
                mVideoRecorder = new DefaultVideoRecorder(CameraRenderActivity.this,
                        mCameraView.getEglContext(),mCameraView.getTextureId());

                mVideoRecorder.initVideo(Environment.getExternalStorageDirectory().getAbsolutePath()+"/01.mp3"
                ,Environment.getExternalStorageDirectory().getAbsolutePath()+"/live_push.mp4"
                ,720,1280);
                mVideoRecorder.startRecord();
                Log.e("TAG","开始录制");
            }

            @Override
            public void onNoMinRecord(int currentTime) {

            }

            @Override
            public void onRecordFinishedListener() {
                //
                mVideoRecorder.stopRecord();
                Log.e("TAG","停止录制");
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mCameraView.onDestroy();
    }
}