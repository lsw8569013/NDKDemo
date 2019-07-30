package com.ly.copyapp;

import android.app.Service;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.IBinder;
import android.provider.Settings;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.RequiresApi;

public class FloatingServic
        extends Service {

    private WindowManager.LayoutParams params;
    private WindowManager manager;
    private View displayView;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }



    @RequiresApi(api = Build.VERSION_CODES.M)
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        showFloatingView();
        return super.onStartCommand(intent, flags, startId);
    }

    /**
     * 悬浮窗控件可以是任意的View的子类类型
     */
    @RequiresApi(api = Build.VERSION_CODES.M)
    private void showFloatingView() {
        if (Settings.canDrawOverlays(getApplicationContext())) {
            //WindowManager 对象
            manager = (WindowManager) getSystemService(WINDOW_SERVICE);

            //新建悬浮控件
            //视频播放
            displayView = View.inflate(getApplicationContext(), R.layout.view_display, null);

            TextView share_z = displayView.findViewById(R.id.share_z);


            //设置layoutParams
            params = new WindowManager.LayoutParams();
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                params.type = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
            } else {
                params.type = WindowManager.LayoutParams.TYPE_PHONE;
            }
            params.format = PixelFormat.RGBA_8888;
//            params.width = WindowManager.LayoutParams.WRAP_CONTENT;
//            params.height = WindowManager.LayoutParams.WRAP_CONTENT;
            params.width = 300;
            params.height = 400;
            //设置不阻挡其他view的触摸事件
            params.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
//            params.x = 500;
//            params.y = 300;
//            params.gravity = Gravity.CENTER_HORIZONTAL;

            //添加view到windowManager
            manager.addView(displayView, params);

            //触摸事件
            displayView.setOnTouchListener(new OnFloatingButtonTouchListener());
        }else{
            Toast.makeText(this,"没有权限！",Toast.LENGTH_SHORT).show();
        }
    }


    private class OnFloatingButtonTouchListener implements View.OnTouchListener {
        private int x;
        private int y;

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    x = (int) event.getRawX();
                    y = (int) event.getRawY();
                    break;
                case MotionEvent.ACTION_UP:

                    break;
                case MotionEvent.ACTION_MOVE:
                    int nowX = (int) event.getRawX();
                    int nowY = (int) event.getRawY();

                    int movedX = nowX - x;
                    int movedY = nowY - y;

                    x = nowX;
                    y = nowY;

                    params.x = params.x + movedX;
                    params.y = params.y + movedY;
                    manager.updateViewLayout(displayView, params);
                    break;

            }
            return false;
        }
    }


}


