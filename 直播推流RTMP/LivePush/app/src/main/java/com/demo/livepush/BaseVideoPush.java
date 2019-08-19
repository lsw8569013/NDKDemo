package com.demo.livepush;

import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.media.MediaRecorder;
import android.opengl.GLSurfaceView;
import android.view.Surface;

import com.demo.opengl.EglHelper;
import com.demo.opengl.LogUtils;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;
import java.util.concurrent.CyclicBarrier;

import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.opengles.GL10;

/**
 * @author liushengwei
 * @description: https://github.com/lsw8569013
 * @date :2019-08-17 10:28
 */
public abstract class BaseVideoPush {

    private static final int AUDIO_SAMPLE_RATE = 44100;
    private static final int CHANNELS = 2;
    private LivePush mLivePush;

    private WeakReference<BaseVideoPush> mVideoRecorederWr = new WeakReference<>(this);

    /**
     * 相机 共享的eglContext
     */
    private EGLContext mEglContext;
    /**
     * 硬编码的 surface
     */
    private Surface mSurface;
    private Context context;

    private GLSurfaceView.Renderer mRenderer;


    private VideoRecorderThread mRenderThread;
    private VideoEncoderThread mVideoThread;

    private AudioEncoderThread mAudioEncoderThread;
    private AudioRecordThread mAudioRecordThread;
    private MediaCodec mVideoCodec;
    private MediaCodec mAudioCodec;





    public BaseVideoPush(Context context, EGLContext mEglContext) {
        this.context = context;
        this.mEglContext = mEglContext;
    }


    private void initAudioCodec(int samplezRate, int channels) throws IOException {
        MediaFormat audioFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, samplezRate, channels);
        //
        audioFormat.setInteger(MediaFormat.KEY_BIT_RATE, 96000);
        //
        audioFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
        audioFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, samplezRate * channels * 2);

        mAudioCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
        mAudioCodec.configure(audioFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

        mAudioRecordThread = new AudioRecordThread(mVideoRecorederWr);
        // 开启一个线程 来采集 音乐播放器上的数据 合成视频
        mAudioEncoderThread = new AudioEncoderThread(mVideoRecorederWr);

    }


    public void setRenderer(GLSurfaceView.Renderer renderer) {
        this.mRenderer = renderer;
        mRenderThread = new VideoRecorderThread(mVideoRecorederWr);
    }


    /**
     * 初始化参数
     *
     * @param width  宽度
     * @param height 高度
     */
    public void initVideo(String liveUrl, int width, int height) {
        try {
            mLivePush = new LivePush(liveUrl);

            mLivePush.setOnConnectListener(new LivePush.ConnectListener() {
                @Override
                public void connectError(int errorCode, String errorMsg) {
//                    Log.e("TAG",errorMsg);
                    if (mConnectListener != null) {
                        mConnectListener.connectError(errorCode, errorMsg);
                    }
                }

                @Override
                public void connectSuccess() {
//                    Log.e("TAG","可以开始推流了");
                    if (mConnectListener != null) {
                        mConnectListener.connectSuccess();
                    }
                    start();
                }
            });

            mRenderThread.setSize(width, height);
            initVideoCodec(width, height);
            initAudioCodec(AUDIO_SAMPLE_RATE,CHANNELS);
        } catch (IOException e) {
            LogUtils.e("-----initVideo error ");
            e.printStackTrace();
        }
    }

    private void start() {
        mRenderThread.start();
        mVideoThread.start();
        mAudioEncoderThread.start();
        mAudioRecordThread.start();
    }


    public void startPush() {
        mLivePush.initConnect();

    }

    public void stopPush() {

        mRenderThread.requestExit();
        mVideoThread.requestExit();
        mAudioEncoderThread.requestExit();
        mAudioRecordThread.requestExit();
        mLivePush.stop();

    }

    private void initVideoCodec(int width, int height) throws IOException {
//        LogUtils.e("----- initVideoCodec");
//        MIMETYPE_VIDEO_AVC = h264
        MediaFormat videoFormat = MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC, width, height);
        // 设置颜色格式
        videoFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        videoFormat.setInteger(MediaFormat.KEY_BIT_RATE, width * height * 4);
        // 设置帧率
        videoFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 24);
        videoFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);

        mVideoCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_VIDEO_AVC);
        mVideoCodec.configure(videoFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

        mSurface = mVideoCodec.createInputSurface();

        // 开启一个线程 来采集 surface 上的数据合成视频
        mVideoThread = new VideoEncoderThread(mVideoRecorederWr);

    }



    /**
     * 音频的编码线程
     */
    private static final class AudioEncoderThread extends Thread {


        private WeakReference<BaseVideoPush> mVideoRecorederWr;
        private volatile boolean mShouldExit = false;

        private MediaCodec mAudioCodec;
        private MediaCodec.BufferInfo mBufferInfo;

        private long mAudioPtes = 0;


        public AudioEncoderThread(WeakReference<BaseVideoPush> mBaseRecorederWr) {
            this.mVideoRecorederWr = mBaseRecorederWr;
            mAudioCodec = mVideoRecorederWr.get().mAudioCodec;
            mBufferInfo = new MediaCodec.BufferInfo();
        }


        @Override
        public void run() {

            try {
                // 开启音频编码
                mAudioCodec.start();

                while (true) {
                    if (mShouldExit) {
                        return;
                    }
                    BaseVideoPush mVideoRecoredRender = mVideoRecorederWr.get();
                    if (mVideoRecoredRender == null) {
                        return;
                    }
                    //
                    int outputBufferindex = mAudioCodec.dequeueOutputBuffer(mBufferInfo, 0);
//
                    while (outputBufferindex >= 0) {
                        // 获取数据 音频数据从哪里来
                        ByteBuffer outBuffer = mAudioCodec.getOutputBuffers()[outputBufferindex];
                        outBuffer.position(mBufferInfo.offset);
                        outBuffer.limit(mBufferInfo.offset + mBufferInfo.size);

                        // 修改pts
                        if (mAudioPtes == 0) {
                            mAudioPtes = mBufferInfo.presentationTimeUs;
                        }

                        mBufferInfo.presentationTimeUs -= mAudioPtes;

//                            Log.e("TAG", "mAudioPts - " + mBufferInfo.presentationTimeUs / 1000000);
                        // 打印一下音频的 AAC 数据
                        byte[] data = new byte[outBuffer.remaining()];
                        outBuffer.get(data, 0, data.length);
//                        LogUtils.e(" audio data   --  " + bytesToHexString(data));
                        mVideoRecorederWr.get().mLivePush.pushAudio(data,data.length);


                        mAudioCodec.releaseOutputBuffer(outputBufferindex, false);
                        outputBufferindex = mAudioCodec.dequeueOutputBuffer(mBufferInfo, 0);

                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                onDestroy();
            }


        }

        private void onDestroy() {
            try {
                mAudioCodec.stop();
                mAudioCodec.release();

            } catch (Exception e) {
                e.printStackTrace();
            }


        }

        private void requestExit() {
            mShouldExit = true;
        }


    }


    /**
     * 音频的采集录制线程
     */
    private static final class AudioRecordThread extends Thread {

        private WeakReference<BaseVideoPush> mVideoRecorederWr;

        private volatile boolean mShouldExit = false;

        private MediaCodec mAudioCodec;

        private long mAudioPts = 0;
        private AudioRecord mAudioRecord;
        private byte[] mAudioData;
        private int mMinibufferSize;


        public AudioRecordThread(WeakReference<BaseVideoPush> mBaseRecorederWr) {
            this.mVideoRecorederWr = mBaseRecorederWr;
            mAudioCodec = mVideoRecorederWr.get().mAudioCodec;


             mMinibufferSize = AudioRecord.getMinBufferSize(
                    BaseVideoPush.AUDIO_SAMPLE_RATE,
                    AudioFormat.CHANNEL_IN_STEREO,
                    AudioFormat.ENCODING_PCM_16BIT);

            mAudioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC,
                    BaseVideoPush.AUDIO_SAMPLE_RATE,
                    AudioFormat.CHANNEL_IN_STEREO,
                    AudioFormat.ENCODING_PCM_16BIT,
                    mMinibufferSize);

            mAudioData = new byte[mMinibufferSize];
        }


        @Override
        public void run() {

            try {
                // 开启录制
                mAudioRecord.startRecording();

                while (true) {
                    if (mShouldExit) {
                        return;
                    }

                    // 不断的去读取mic 上传了的pcm 数据
                    mAudioRecord.read(mAudioData, 0, mMinibufferSize);

                    // 吧数据写入 inputBuffer
                    int inputBufferIndex = mAudioCodec.dequeueInputBuffer(0);
                    if (inputBufferIndex >= 0) {
                        ByteBuffer inputBuffer = mAudioCodec.getInputBuffers()[inputBufferIndex];
                        inputBuffer.clear();
                        inputBuffer.put(mAudioData);

                        mAudioPts += mMinibufferSize * 1000000 / BaseVideoPush.AUDIO_SAMPLE_RATE * BaseVideoPush.CHANNELS * 2;

                        mAudioCodec.queueInputBuffer(inputBufferIndex, 0, mMinibufferSize, mAudioPts, 0);
                    }

                }
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                onDestroy();
            }


        }

        private void onDestroy() {
            try {
                mAudioCodec.stop();
                mAudioCodec.release();


            } catch (Exception e) {
                e.printStackTrace();
            }


        }

        private void requestExit() {
            mShouldExit = true;
        }


    }

    /**
     * 视频的采集线程
     */
    private static final class VideoEncoderThread extends Thread {


        private WeakReference<BaseVideoPush> mVideoRecorederWr;

        private volatile boolean mShouldExit = false;

        private MediaCodec mVideoCodec;
        private MediaCodec.BufferInfo mBufferInfo;
        private int mVideoTrackindex = -1;
        private long mVideoPtes = 0;

        private byte[] mVideoSps, mVideoPps;

        public VideoEncoderThread(WeakReference<BaseVideoPush> mBaseRecorederWr) {
            this.mVideoRecorederWr = mBaseRecorederWr;
            mVideoCodec = mVideoRecorederWr.get().mVideoCodec;

            mBufferInfo = new MediaCodec.BufferInfo();

        }


        @Override
        public void run() {


            try {
                mVideoCodec.start();
                while (true) {
                    if (mShouldExit) {
                        return;
                    }
                    BaseVideoPush mVideoRecoredRender = mVideoRecorederWr.get();
                    if (mVideoRecoredRender == null) {
                        return;
                    }
                    // 从 surface上获取数据 ，编码成h264 ,通过 MediaMuter 合成mp4
                    int outputBufferindex = mVideoCodec.dequeueOutputBuffer(mBufferInfo, 0);
                    if (outputBufferindex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {

                        // 获取 pps sps


                        LogUtils.e("获取 pps --  sps ");

                        ByteBuffer byteBuffer = mVideoCodec.getOutputFormat().getByteBuffer("csd-0");
                        mVideoSps = new byte[byteBuffer.remaining()];

                        byteBuffer.get(mVideoSps, 0, mVideoSps.length);

//                        LogUtils.e("sps --  "+bytesToHexString(mVideoSps));

                        byteBuffer = mVideoCodec.getOutputFormat().getByteBuffer("csd-1");
                        mVideoPps = new byte[byteBuffer.remaining()];

                        byteBuffer.get(mVideoPps, 0, mVideoPps.length);

//                        LogUtils.e("pps -- "+bytesToHexString(mVideoPps));


                    } else {
                        while (outputBufferindex >= 0) {
                            // 获取数据
                            ByteBuffer outBuffer = mVideoCodec.getOutputBuffers()[outputBufferindex];
                            outBuffer.position(mBufferInfo.offset);
                            outBuffer.limit(mBufferInfo.offset + mBufferInfo.size);



                            // 修改pts
                            if (mVideoPtes == 0) {
                                mVideoPtes = mBufferInfo.presentationTimeUs;
                            }
                            mBufferInfo.presentationTimeUs -= mVideoPtes;

                            // 在关键帧前 先把 sps 和pps 推上去
                            if(mBufferInfo.flags == MediaCodec.BUFFER_FLAG_KEY_FRAME){
                                mVideoRecorederWr.get().mLivePush.pushSpsPPs(mVideoSps,
                                        mVideoSps.length,mVideoPps,mVideoPps.length);
                            }



                            byte[] data = new byte[outBuffer.remaining()];
                            outBuffer.get(data, 0, data.length);
//                            LogUtils.e("视频的帧data   --  "+bytesToHexString(data));
                            mVideoRecorederWr.get().mLivePush.pushVideo(data,data.length,
                                    mBufferInfo.flags == MediaCodec.BUFFER_FLAG_KEY_FRAME);



                            //视频 回调时间
                            if (mVideoRecoredRender.mRecorderListener != null) {
                                mVideoRecoredRender.mRecorderListener.onTime(mBufferInfo.presentationTimeUs / 1000);
                            }


                            mVideoCodec.releaseOutputBuffer(outputBufferindex, false);
                            outputBufferindex = mVideoCodec.dequeueOutputBuffer(mBufferInfo, 0);
                        }
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                onDestroy();
            }
        }


        private void onDestroy() {

            try {
                mVideoCodec.stop();
                mVideoCodec.release();
            } catch (Exception e) {
                e.printStackTrace();
            }


        }

        private void requestExit() {
            mShouldExit = true;
        }


    }


    private RecorderListener mRecorderListener;

    public void setOnRecorderListener(RecorderListener recorderListener) {
        this.mRecorderListener = recorderListener;
    }

    public interface RecorderListener {
        void onTime(long times);
    }


    /**
     * 视频的渲染线程
     */
    private static final class VideoRecorderThread extends Thread {


        private WeakReference<BaseVideoPush> mVideoRecorederWr;
        private EglHelper mEglHelper;

        private volatile boolean mShouldExit = false;
        private boolean mHaveCreateEgl = false;
        private boolean mHaveSurfaceCreated = false;
        private boolean mHaveSurfaceChanged = false;
        private int mHeight;
        private int mWidth;


        public VideoRecorderThread(WeakReference<BaseVideoPush> mBaseRecorederWr) {
            this.mVideoRecorederWr = mBaseRecorederWr;
            mEglHelper = new EglHelper();
        }


        @Override
        public void run() {


            try {
                while (true) {
                    if (mShouldExit) {
                        return;
                    }
                    BaseVideoPush mVideoRecoredRender = mVideoRecorederWr.get();
                    if (mVideoRecoredRender == null) {
                        return;
                    }

                    if (!mHaveCreateEgl) {
                        mEglHelper.initCreateEgl(mVideoRecoredRender.mSurface, mVideoRecoredRender.mEglContext);
                        mHaveCreateEgl = true;
                    }

                    GL10 egl = (GL10) mEglHelper.getEglContext().getGL();

                    // 回调Render
                    if (!mHaveSurfaceCreated) {
                        mVideoRecoredRender.mRenderer.onSurfaceCreated(egl, mEglHelper.getEGLConfig());
                        mHaveSurfaceCreated = true;
                    }

                    if (!mHaveSurfaceChanged) {
                        mVideoRecoredRender.mRenderer.onSurfaceChanged(egl, mWidth, mHeight);
                        mHaveSurfaceChanged = true;
                    }

                    mVideoRecoredRender.mRenderer.onDrawFrame(egl);

                    mEglHelper.swapBuffers();

                    Thread.sleep(16);
                }
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                onDestroy();
            }


        }

        private void onDestroy() {
            mEglHelper.destroy();

        }

        private void requestExit() {
            mShouldExit = true;
        }

        public void setSize(int width, int height) {
            this.mWidth = width;
            this.mHeight = height;
        }
    }

    public void setmConnectListener(ConnectListener mConnectListener) {
        this.mConnectListener = mConnectListener;
    }

    private ConnectListener mConnectListener;


    public interface ConnectListener {
        void connectError(int errorCode, String errorMsg);

        void connectSuccess();
    }

    private static String bytesToHexString(byte[] bArr) {
        StringBuffer sb = new StringBuffer(bArr.length);
        String sTmp;

        for (int i = 0; i < bArr.length; i++) {
            sTmp = Integer.toHexString(bArr[i]);
            if (sTmp.length() < 2)
                sb.append(0);
            sb.append(sTmp.toUpperCase());
            if (i > 20) {
                break;
            }
        }

        return sb.toString();
    }

}
