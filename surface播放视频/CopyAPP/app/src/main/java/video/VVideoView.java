package video;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import music.VPlayer;
import music.listener.MediaPreparedListener;

public class VVideoView extends SurfaceView implements MediaPreparedListener {
    private VPlayer mPlayer;
    public VVideoView(Context context) {
        this(context,null);
    }

    public VVideoView(Context context, AttributeSet attrs) {
        this(context, attrs,0);
    }

    public VVideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);

        // 设置显示的像素格式
        SurfaceHolder holder = getHolder();
        holder.setFormat(PixelFormat.RGBA_8888);

        mPlayer = new VPlayer();
        mPlayer.setPreparedListener(this);
    }
    public void  play(String url){
        Log.e("lsw","videoView ---play");
        stop();
        mPlayer.setDataSource(url);

        mPlayer.prepareAsync();
    }

    /**
     * 停止上一个视频，释放资源
     */
    public void stop(){
        mPlayer.stop();
    }


    @Override
    public void onPrepared() {
        Log.e("lsw","videoView  onPrepared start---  ");
//        mPlayer.setSurface(getHolder().getSurface());
        mPlayer.play(getHolder().getSurface());
        Log.e("lsw","videoView  onPrepared ---  ");
    }
}
