package com.demo.livepush;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {
    private LivePush livePush;

    // Used to load the 'native-lib' library on application startup.


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);

        livePush = new LivePush("rtmp://192.168.2.101/oflaDemo/test");

        livePush.setOnConnectListener(new LivePush.ConnectListener() {
            @Override
            public void connectError(int errorCode, String errorMsg) {
                Log.e("TAG",errorMsg);
                livePush.stop();
            }

            @Override
            public void connectSuccess() {
                Log.e("TAG","可以开始推流了");
            }
        });
        livePush.initConnect();
    }


}
