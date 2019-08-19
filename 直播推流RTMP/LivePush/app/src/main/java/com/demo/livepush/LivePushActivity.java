package com.demo.livepush;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import com.demo.camera.CameraView;
import com.demo.opengl.LogUtils;
import com.demo.opengl.Utils;

public class LivePushActivity extends AppCompatActivity {


    // Used to load the 'native-lib' library on application startup.
    private CameraView mCameraView;
    private Button livePush_btn;

    private DefaultVideoPush mVideoPush;
    private LivePushActivity context;


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
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera_livepush);
        context = this;
        mCameraView = findViewById(R.id.surface_view);
        livePush_btn = findViewById(R.id.livePush_btn);
        View livePushStop_btn = findViewById(R.id.livePushStop_btn);

        livePush_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startPush();
                LogUtils.e("startPush");
            }
        });

        livePushStop_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                if (mVideoPush != null) {
                    LogUtils.e("btn click stopPush");
                    mVideoPush.stopPush();
                }
            }
        });
    }

    private void startPush() {

            mVideoPush = new DefaultVideoPush(LivePushActivity.this
                    , mCameraView.getEglContext(), mCameraView.getTextureId());

//            mVideoPush.initVideo("rtmp://192.168.2.100/oflaDemo/test",
//                    720/2,1280/2);
        mVideoPush.initVideo("",
                720 / 2, 1280 / 2);

            mVideoPush.setmConnectListener(new BaseVideoPush.ConnectListener() {
                @Override
                public void connectError(int errorCode, String errorMsg) {
                    Log.e("TAG", errorMsg);

                }

                @Override
                public void connectSuccess() {
                    Log.e("TAG", "可以开始推流了");


                }
            });

        mVideoPush.startPush();
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        LogUtils.e("onDestroy");
        if (mVideoPush != null) {
            mVideoPush.stopPush();
        }

    }
}
