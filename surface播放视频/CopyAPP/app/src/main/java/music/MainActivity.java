package music;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.EditText;

import com.ly.copyapp.R;

import java.io.File;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import video.VVideoView;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    private static final int REQUEST_CAMERA = 99;


    private VPlayer mPlayer;

    static {
        System.loadLibrary("music-player");
    }
//    File mMusicFile = new File(Environment.getExternalStorageDirectory().getAbsolutePath()+"/test3.mkv");
    File mMusicFile = null;
    private VVideoView videoView;
    private EditText fileName;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        videoView = findViewById(R.id.vvvideo);
        fileName = findViewById(R.id.btn_file_name);



        if(ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED){
//            // 权限未被授予
            requestFilePermission();
        }

        /*mPlayer = new VPlayer();
        if(!mMusicFile.exists()){
            Toast.makeText(this,"音乐找不到了",Toast.LENGTH_SHORT).show();
        }else{
            mPlayer.setDataSource(mMusicFile.getAbsolutePath());
            mPlayer.setErrorListener(new MediaErrorListener() {
                @Override
                public void onError(int code, int msg) {
                    Log.e("TAG", "error code: " + code);
                    Log.e("TAG", "error msg: " + msg);
                }
            });

            mPlayer.setPreparedListener(new MediaPreparedListener() {
                @Override
                public void onPrepared() {
                    mPlayer.play();
                }
            });
        }*/


        Log.i(TAG, "检查权限是否被受理！");
        // 检查是否想要的权限申请是否弹框。如果是第一次申请，用户不通过，
        // 那么第二次申请的话，就要给用户说明为什么需要申请这个权限
//        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
//                != PackageManager.PERMISSION_GRANTED) {
//            // 权限未被授予
//            requestCameraPermission();
//        } else {
//
//        }


    }




    /**
     * 申请相机权限
     */
    private void requestCameraPermission() {
        Log.i(TAG, "相机权限未被授予，需要申请！");
        // 相机权限未被授予，需要申请！
        if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                Manifest.permission.CAMERA)) {
            // 如果访问了，但是没有被授予权限，则需要告诉用户，使用此权限的好处
            Log.i(TAG, "申请权限说明！");

                            // 这里重新申请权限
                            ActivityCompat.requestPermissions(MainActivity.this,
                                    new String[]{Manifest.permission.CAMERA},
                                    REQUEST_CAMERA);

        } else {
            // 第一次申请，就直接申请
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA},
                    REQUEST_CAMERA);
        }
    }

    private void requestFilePermission() {
        Log.i(TAG, "相机权限未被授予，需要申请！");
        // 相机权限未被授予，需要申请！
        if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            // 如果访问了，但是没有被授予权限，则需要告诉用户，使用此权限的好处
            Log.i(TAG, "申请权限说明！");

                            // 这里重新申请权限
                            ActivityCompat.requestPermissions(MainActivity.this,
                                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                                    REQUEST_CAMERA);

        } else {
            // 第一次申请，就直接申请
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA},
                    REQUEST_CAMERA);
        }
    }

    // 播放视频
    public void playVideo(View view) {
        mMusicFile = new File(Environment.getExternalStorageDirectory().getAbsolutePath()+File.separator+fileName.getText().toString().trim());
        videoView.play(mMusicFile.getAbsolutePath());
//        decodeVideo(mMusicFile.getAbsolutePath(),videoView.getHolder().getSurface());
    }




    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        if (requestCode == REQUEST_CAMERA) {
            if (grantResults.length == 1 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {

            } else {

            }
        }
    }
}