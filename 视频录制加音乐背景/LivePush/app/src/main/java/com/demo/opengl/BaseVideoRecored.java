package com.demo.opengl;

import android.content.Context;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.Surface;


import com.darren.media.DarrenPlayer;
import com.darren.media.listener.MediaInfoListener;
import com.darren.media.listener.MediaPreparedListener;

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
abstract class BaseVideoRecored {


    private WeakReference<BaseVideoRecored> mVideoRecorederWr = new WeakReference<>(this);

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
    private MediaMuxer mMediaMuxer;

    private VideoRecorderThread mRenderThread;
    private VideoEncoderThread mVideoThread;

    private AudioEncoderThread mAudioThread;
    private MediaCodec mVideoCodec;
    private MediaCodec mAudioCodec;

    private CyclicBarrier mStartCb = new CyclicBarrier(2);
    private CyclicBarrier mDestoryCb = new CyclicBarrier(2);

    private DarrenPlayer mMusicPlayer;


    public BaseVideoRecored(Context context, EGLContext mEglContext) {
        this.context = context;
        this.mEglContext = mEglContext;
        mMusicPlayer = new DarrenPlayer();
        mMusicPlayer.setOnPreparedListener(new MediaPreparedListener() {
            @Override
            public void onPrepared() {
                mMusicPlayer.play();
                start();
            }
        });
        mMusicPlayer.setOnInfoListener(mMediaInfoListener);

    }

    private void start() {
        mRenderThread.start();
        mVideoThread.start();
        mAudioThread.start();
    }

    private MediaInfoListener mMediaInfoListener = new MediaInfoListener() {
        private long mAudioPts = 0;
        private int mSamplezRate = 0;
        private int mChannels = 0;

        @Override
        public void musicInfo(int samplezRate, int channels) {
            try {
                initAudioCodec(samplezRate, channels);
            } catch (IOException e) {
                e.printStackTrace();
            }
            this.mSamplezRate = samplezRate;
            this.mChannels = channels;
        }

        @Override
        public void callbackPcm(byte[] pcmData, int size) {
            // 吧数据写入 inputBuffer
            int inputBufferIndex = mAudioCodec.dequeueInputBuffer(0);
            if (inputBufferIndex >= 0) {
                ByteBuffer inputBuffer = mAudioCodec.getInputBuffers()[inputBufferIndex];
                inputBuffer.clear();
                inputBuffer.put(pcmData);

                mAudioPts += size * 1000000 / mSamplezRate * mChannels * 2;

                mAudioCodec.queueInputBuffer(inputBufferIndex, 0, size, mAudioPts, 0);
            }

        }
    };

    private void initAudioCodec(int samplezRate, int channels) throws IOException {
        MediaFormat audioFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, samplezRate, channels);
        //
        audioFormat.setInteger(MediaFormat.KEY_BIT_RATE, 96000);
        //
        audioFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
        audioFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, samplezRate * channels * 2);

        mAudioCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
        mAudioCodec.configure(audioFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);


        // 开启一个线程 来采集 音乐播放器上的数据 合成视频
        mAudioThread = new AudioEncoderThread(mVideoRecorederWr);
        mAudioCodec.start();
    }


    public void setRenderer(GLSurfaceView.Renderer renderer) {
        this.mRenderer = renderer;
        mRenderThread = new VideoRecorderThread(mVideoRecorederWr);
    }


    /**
     * 初始化参数
     *
     * @param audioPaht 音频文件路径
     * @param outPath   输出文件路径
     * @param width     宽度
     * @param height    高度
     */
    public void initVideo(String audioPaht, String outPath, int width, int height) {
        try {
            mRenderThread.setSize(width, height);
            mMediaMuxer = new MediaMuxer(outPath, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
            initVideoCodec(width, height);
            mMusicPlayer.setDataSource(audioPaht);
        } catch (IOException e) {
            LogUtils.e("-----initVideo error ");
            e.printStackTrace();
        }
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

    public void startRecord() {
        mMusicPlayer.prepareAsync();
    }

    public void stopRecord() {
        mMusicPlayer.stop();
        mRenderThread.requestExit();
        mVideoThread.requestExit();
        mAudioThread.requestExit();


    }


    /**
     * 音频的采集线程
     */
    private static final class AudioEncoderThread extends Thread {


        private MediaMuxer mMediaMuxer;
        private WeakReference<BaseVideoRecored> mVideoRecorederWr;

        private volatile boolean mShouldExit = false;

        private MediaCodec mAudioCodec;
        private MediaCodec.BufferInfo mBufferInfo;
        private int mAudeoTrackindex = -1;
        private long mAudioPtes = 0;
        private final CyclicBarrier mStartCb, mDestoryCb;


        public AudioEncoderThread(WeakReference<BaseVideoRecored> mBaseRecorederWr) {
            this.mVideoRecorederWr = mBaseRecorederWr;
            mAudioCodec = mVideoRecorederWr.get().mAudioCodec;
            mMediaMuxer = mVideoRecorederWr.get().mMediaMuxer;
            mStartCb = mVideoRecorederWr.get().mStartCb;
            mDestoryCb = mVideoRecorederWr.get().mDestoryCb;
            mBufferInfo = new MediaCodec.BufferInfo();
        }


        @Override
        public void run() {


            try {

                while (true) {
                    if (mShouldExit) {
                        return;
                    }
                    BaseVideoRecored mVideoRecoredRender = mVideoRecorederWr.get();
                    if (mVideoRecoredRender == null) {
                        return;
                    }
                    //
                    int outputBufferindex = mAudioCodec.dequeueOutputBuffer(mBufferInfo, 0);
                    if (outputBufferindex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                        mAudeoTrackindex = mMediaMuxer.addTrack(mAudioCodec.getOutputFormat());
                        mStartCb.await();
                        // mMediaMuxer.start();
                    } else {
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

                            Log.e("TAG", "mAudioPts - " + mBufferInfo.presentationTimeUs / 1000000);

                            // 写入数据
                            // 要等待 视频线程
                            mMediaMuxer.writeSampleData(mAudeoTrackindex, outBuffer, mBufferInfo);

//


                            mAudioCodec.releaseOutputBuffer(outputBufferindex, false);
                            outputBufferindex = mAudioCodec.dequeueOutputBuffer(mBufferInfo, 0);
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
                mAudioCodec.stop();
                mAudioCodec.release();
                mDestoryCb.await();

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


        private MediaMuxer mMediaMuxer;
        private WeakReference<BaseVideoRecored> mVideoRecorederWr;

        private volatile boolean mShouldExit = false;

        private MediaCodec mVideoCodec;
        private MediaCodec.BufferInfo mBufferInfo;
        private int mVideoTrackindex = -1;
        private long mVideoPtes = 0;
        private final CyclicBarrier mStartCb;
        private final CyclicBarrier mDestoryCb;

        public VideoEncoderThread(WeakReference<BaseVideoRecored> mBaseRecorederWr) {
            this.mVideoRecorederWr = mBaseRecorederWr;
            mVideoCodec = mVideoRecorederWr.get().mVideoCodec;
            mMediaMuxer = mVideoRecorederWr.get().mMediaMuxer;
            mBufferInfo = new MediaCodec.BufferInfo();
            mStartCb = mVideoRecorederWr.get().mStartCb;
            mDestoryCb = mVideoRecorederWr.get().mDestoryCb;
        }


        @Override
        public void run() {


            try {
                mVideoCodec.start();
                while (true) {
                    if (mShouldExit) {
                        return;
                    }
                    BaseVideoRecored mVideoRecoredRender = mVideoRecorederWr.get();
                    if (mVideoRecoredRender == null) {
                        return;
                    }
                    // 从 surface上获取数据 ，编码成h264 ,通过 MediaMuter 合成mp4
                    int outputBufferindex = mVideoCodec.dequeueOutputBuffer(mBufferInfo, 0);
                    if (outputBufferindex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                        mVideoTrackindex = mMediaMuxer.addTrack(mVideoCodec.getOutputFormat());
                        mMediaMuxer.start();
                        mStartCb.await();
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
                            // 写入数据

                            mMediaMuxer.writeSampleData(mVideoTrackindex, outBuffer, mBufferInfo);

                            // 回调时间
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
                mDestoryCb.await();
                mMediaMuxer.stop();
                mMediaMuxer.release();
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


        private WeakReference<BaseVideoRecored> mVideoRecorederWr;
        private EglHelper mEglHelper;

        private volatile boolean mShouldExit = false;
        private boolean mHaveCreateEgl = false;
        private boolean mHaveSurfaceCreated = false;
        private boolean mHaveSurfaceChanged = false;
        private int mHeight;
        private int mWidth;


        public VideoRecorderThread(WeakReference<BaseVideoRecored> mBaseRecorederWr) {
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
                    BaseVideoRecored mVideoRecoredRender = mVideoRecorederWr.get();
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


}
