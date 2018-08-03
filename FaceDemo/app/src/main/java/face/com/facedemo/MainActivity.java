package face.com.facedemo;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity  implements PermissionUtil.PermissionCallBack{

//     Used to load the 'native-lib' library on application startup.


    private ImageView mImage_detation;
    private Bitmap mBitmap;
    private FaceDetation mMFaceDetection;
    private File mCascadeFile;
    private PermissionUtil mPermissionUtil;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);


        mBitmap = BitmapFactory.decodeResource(getResources(), R.mipmap.face);

        mImage_detation = findViewById(R.id.image_detation);

        mImage_detation.setImageBitmap(mBitmap);

        mMFaceDetection = new FaceDetation();

        copyCascadeClassifier();

        mMFaceDetection.loadPath(mCascadeFile.getAbsolutePath());




        tv.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mMFaceDetection.faceDetectionSaveInfo(mBitmap, Environment.getExternalStorageDirectory().getAbsolutePath()+"/face.jpg");
                mImage_detation.setImageBitmap(mBitmap);
            }
        });

        mPermissionUtil = PermissionUtil.getInstance();
        mPermissionUtil.requestPermissions(this, 999, this);

    }

    private void copyCascadeClassifier() {
        try {
            // load cascade file from application resources
            InputStream is = getResources().openRawResource(R.raw.lbpcascade_frontalface);
            File cascadeDir = getDir("cascade", Context.MODE_PRIVATE);
            mCascadeFile = new File(cascadeDir, "lbpcascade_frontalface.xml");
            if(mCascadeFile.exists()){
                return;
            }
            FileOutputStream os = new FileOutputStream(mCascadeFile);

            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = is.read(buffer)) != -1) {
                os.write(buffer, 0, bytesRead);
            }
            is.close();
            os.close();

            



        } catch (IOException e) {
            e.printStackTrace();
            Log.e("lsw", "Failed to load cascade. Exception thrown: " + e);
        }

    }


    @Override
    public void onPermissionSuccess() {

    }

    @Override
    public void onPermissionReject(String strMessage) {

    }

    @Override
    public void onPermissionFail() {

    }
}
