package face.com.facedemo;

import android.graphics.Bitmap;

/**
 * author: Created by lsw on 2018/7/31 17:44
 * description:
 */
public class FaceDetation {

    static {
        System.loadLibrary("native-lib");
    }



    public native void  faceDetectionSaveInfo(Bitmap bitmap, String faceFileName);
    public native void  loadPath(String filePath);

}
