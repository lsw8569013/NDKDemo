package com.darren.music.player;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;

import com.darren.media.DarrenPlayer;
import com.darren.media.listener.MediaErrorListener;
import com.darren.media.listener.MediaInfoListener;
import com.darren.media.listener.MediaPreparedListener;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    File mMusicFile = new File(Environment.getExternalStorageDirectory(), "01.mp3");
    // Used to load the 'native-lib' library on application startup.
    private DarrenPlayer mPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Log.e("TAG", "file is exist: " + mMusicFile.exists());

        mPlayer = new DarrenPlayer();
        mPlayer.setDataSource(mMusicFile.getAbsolutePath());

        mPlayer.setOnErrorListener(new MediaErrorListener() {
            @Override
            public void onError(int code, String msg) {
                Log.e("TAG", "error code: " + code);
                Log.e("TAG", "error msg: " + msg);
                // Java 的逻辑代码
            }
        });

        mPlayer.setOnPreparedListener(new MediaPreparedListener() {
            @Override
            public void onPrepared() {
                Log.e("TAG", "准备完毕");
                mPlayer.play();
            }
        });

        mPlayer.setOnInfoListener(new MediaInfoListener() {
            @Override
            public void musicInfo(int samplezRate, int channels) {

            }

            @Override
            public void callbackPcm(byte[] pcmData, int size) {

//                Log.e("TAG","audio size = "+ size);
//                Log.e("TAG",pcmData.toString());
            }


        });

        findViewById(R.id.sample_text).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mPlayer.stop();
            }
        });
        findViewById(R.id.start_btn).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mPlayer.prepareAsync();
            }
        });



    }
}
