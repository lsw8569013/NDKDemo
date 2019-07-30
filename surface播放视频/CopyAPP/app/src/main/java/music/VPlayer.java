package music;

import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

import music.listener.MediaErrorListener;
import music.listener.MediaPreparedListener;

public class VPlayer {

    static {
        System.loadLibrary("music-player");
    }

    private String url;


    public void setDataSource(String url){
        this.url = url;
    }

    public  void play(Surface surface){
        if(TextUtils.isEmpty(url)){
            throw new NullPointerException("url is null ,");
        }
        nPlay(surface);
    }



    public void prepared(){
        if(TextUtils.isEmpty(url)){
            throw new NullPointerException("url is null ,");
        }
        nPrepared(url);
    }


    public void prepareAsync() {
        if(TextUtils.isEmpty(url)){
            throw new NullPointerException("url is null ,");
        }
        nPrepareAsync(url);
    }



    private MediaErrorListener errorListener;
    private MediaPreparedListener preparedListener;


    public MediaErrorListener getErrorListener() {
        return errorListener;
    }

    public void setErrorListener(MediaErrorListener errorListener) {
        this.errorListener = errorListener;
    }

    public MediaPreparedListener getPreparedListener() {
        return preparedListener;
    }

    public void setPreparedListener(MediaPreparedListener preparedListener) {
        this.preparedListener = preparedListener;
    }


    /**
     *  jni  error call
     * @param code
     * @param msg
     */
    private void onError(int code ,int msg){
        if(errorListener != null)
            errorListener.onError(code,msg);
    }

    /**
     *  jni onPrepared call
     */
    private void onPrepared(){
        Log.e("lsw"," ndk  onPrepared ok vPlayer 86" );
        if(preparedListener != null){
            preparedListener.onPrepared();
        }
    }

    public void stop() {

    }


    private native void nPlay(Surface surface) ;

    private native void nPrepared(String url) ;

    private native  void nPrepareAsync(String url) ;

    public native void setSurface(Surface surface) ;
}
