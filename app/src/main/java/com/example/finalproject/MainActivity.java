package com.example.finalproject;

import static java.lang.Math.abs;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.hardware.Camera;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Mat;
import org.opencv.core.Point;
import org.opencv.core.Scalar;
import org.opencv.imgproc.Imgproc;

import java.io.File;
import java.io.FileOutputStream;

public class MainActivity extends AppCompatActivity implements GPIOListener{

    private static final String TAG = "FinalProject";

    // Used to load the 'finalproject' library on application startup.
//    static {
//        System.loadLibrary("finalproject");
//    }

    static {
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "OpenCV is not loaded.");
        } else {
            Log.d(TAG, "OpenCV is loaded successfully.");
        }
        System.loadLibrary("JNIDriver");
        System.loadLibrary("OpenCLDriver");
    }


    private Camera mCamera;
    private CameraPreview mPreview;
    private ImageView capturedImageHolder;
    private Bitmap bitmap;
    //private Bitmap convertedBitmap;
    private TextView text_left;
    private TextView text_right;
    GPIODriver gpioDriver;
    private native static int openLEDDriver(String path);
    private native static void closeLEDDriver();
    private native static void writeLEDDriver(byte[] data, int length);
    private native static int openSegmentDriver(String path);
    private native static void closeSegmentDriver();
    private native static void writeSegmentDriver(byte[] data, int length);
    private native static Bitmap makeGrayscale(Bitmap input);
    byte[] led = {0, 0, 0, 0, 0, 0, 0, 0};

    int data_int = 30;
    boolean mThreadRun, mStart;
    SegmentThread mSegThread;

    private class SegmentThread extends Thread {
        @Override
        public void run() {
            super.run();
            while(mThreadRun) {
                byte[] n = {0, 0, 0, 0, 0, 0, 0};

                if(mStart==false) {writeSegmentDriver(n, n.length);}
                else {
                    for(int i=0; i<100; i++) {
                        n[0] = (byte) (data_int % 1000000 / 100000);
                        n[1] = (byte) (data_int % 100000 / 10000);
                        n[2] = (byte) (data_int % 10000 / 1000);
                        n[3] = (byte) (data_int % 1000 / 100);
                        n[4] = (byte) (data_int % 100 / 10);
                        n[5] = (byte) (data_int % 10 );
                        writeSegmentDriver(n, n.length);
                    }
                }
            }
        }
    }

    private final int REQ_CODE_SELECT_IMAGE = 100;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
//          버튼 삭제 -> GPIO 버튼으로 컨트롤
//        Button btn = (Button)findViewById(R.id.button_capture);
        capturedImageHolder = (ImageView)findViewById(R.id.captured_image);

        mCamera = getCameraInstance();
        mCamera.setDisplayOrientation(180);
        mPreview = new CameraPreview(this,mCamera);
        FrameLayout preview = (FrameLayout) findViewById(R.id.camera_preview);
        preview.addView(mPreview);
        text_left = (TextView) findViewById(R.id.text_left);
        text_right = (TextView) findViewById(R.id.text_right);
//          버튼 삭제 -> GPIO 버튼으로 컨트롤
//        btn.setOnClickListener(new View.OnClickListener(){
//            @Override
//            public void onClick(View v) {
//                mCamera.takePicture(null,null,pictureCallback);
//            }
//        });

        gpioDriver = new GPIODriver();
        gpioDriver.setListener(this);

        if(gpioDriver.open("/dev/sm9s5422_interrupt") < 0) {
            Toast.makeText(MainActivity.this, "Driver Open Failed",
                    Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    protected void onResume() {
        if(openLEDDriver("/dev/sm9s5422_led") < 0) {
            Toast.makeText(MainActivity.this, "Driver Open Failed",
                    Toast.LENGTH_SHORT).show();
        }
        if(openSegmentDriver("/dev/sm9s5422_segment") < 0) {
            Toast.makeText(MainActivity.this, "Driver Open Failed",
                    Toast.LENGTH_SHORT).show();
        }
        mThreadRun=true;
        mSegThread = new SegmentThread();
        mSegThread.start();
        super.onResume();
    }

    public static Camera getCameraInstance(){
        Camera c = null;
        try{
            c = Camera.open();
        } catch (Exception e) {

        }
        return c;
    }

    Camera.PictureCallback pictureCallback = new Camera.PictureCallback() {
        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
//            Bitmap bitmap = BitmapFactory.decodeByteArray(data,0,data.length);
            bitmap = BitmapFactory.decodeByteArray(data,0,data.length);
            int w = bitmap.getWidth();
            int h = bitmap.getHeight();

            Matrix mtx = new Matrix();
            mtx.postRotate(180);
            mtx.postScale(0.25f, 0.25f);
            bitmap = Bitmap.createBitmap(bitmap,0,0,w,h,mtx,true);
            //convertedBitmap = Bitmap.createBitmap(bitmap,0,0,w,h,mtx,true);
            //Bitmap rotatedBitmap = Bitmap.createScaledBitmap(bitmap,450,300,false);
            //rotatedBitmap = Bitmap.createBitmap(rotatedBitmap, 0, 0, 450, 300, mtx, true);

            if(bitmap==null) {
                Toast.makeText(MainActivity.this, "Capture image is empty",
                        Toast.LENGTH_LONG).show();
                return;
            }

            capturedImageHolder.setImageBitmap(scaleDownBitmapImage(bitmap, 450,
                    300));
//            bitmap.recycle();
//            rotatedBitmap.recycle();
//            System.gc();
        }
    };
    private Bitmap scaleDownBitmapImage(Bitmap bitmap, int newWidth, int newHeight) {
        Bitmap resizedBitmap = Bitmap.createScaledBitmap(bitmap, newWidth, newHeight,true);
        return resizedBitmap;
    }

    @Override
    protected void onPause() {
        gpioDriver.close();
        super.onPause();
        releaseMediaRecorder();
        releaseCamera();
        closeLEDDriver();
        closeSegmentDriver();
        mThreadRun=false;
        mSegThread=null;
    }

    public Handler handler = new Handler(Looper.getMainLooper()){
        public void handleMessage(Message msg) {
            switch (msg.arg1) {
                case 1:
//                    tv.setText("Up");
                    break;
                case 2:
//                    tv.setText("Down");
                    break;
                case 3:
//                    tv.setText("Left");
                    if (bitmap != null) {
                        detectCircleCPU();
                        capturedImageHolder.setImageBitmap(bitmap);
                    }
                    break;
                case 4:
//                    tv.setText("Right");
                    if (bitmap != null) {
                        detectCircleGPU();
                        capturedImageHolder.setImageBitmap(bitmap);
                    }
                    break;
                case 5:
//                    tv.setText("Center");
                    mCamera.takePicture(null,null,pictureCallback);
                    break;
            }
        }
    };

    public void onReceive(int val) {
        Message text = Message.obtain();
        text.arg1 = val;
        handler.sendMessage(text);
    }

    private void releaseMediaRecorder() { mCamera.lock(); }

    private void releaseCamera() {
        if (mCamera != null) {
            mCamera.release();
            mCamera = null;
        }
    }

    protected void onDestroy() {
//        bitmap1.recycle();
//        bitmap1 = null;
        bitmap.recycle();
        bitmap = null;
        super.onDestroy();
    }

    //onButtonRunClicked 안써서 지워도 됨 <- 버튼을 GPIO버튼으로 대체함
    public void onButtonRunClicked(View view) {
        if (bitmap != null) {
            detectCircleGPU();
            capturedImageHolder.setImageBitmap(bitmap);
        }
    }
    //detectEdge 안써서 지워도 됨
    public void detectEdge() {
        Mat src = new Mat();
        Utils.bitmapToMat(bitmap,src);
        Mat edge = new Mat();
        Imgproc.Canny(src, edge, 50, 150);
        Utils.matToBitmap(edge, bitmap);
        src.release();
        edge.release();
    }

    public void saveBitmap(Bitmap bitmap, String name) {
        File tempFile = new File(getCacheDir(), name);
        try {
            tempFile.createNewFile();
            FileOutputStream out = new FileOutputStream(tempFile);
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, out);
            out.close(); 
            Toast.makeText(getApplicationContext(), "파일 저장 성공", Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            Toast.makeText(getApplicationContext(), "파일 저장 실패", Toast.LENGTH_SHORT).show();
        }
    }

    public void detectCircleGPU() {
        Mat src = new Mat();
        Bitmap bmp32 = bitmap.copy(Bitmap.Config.ARGB_8888, true);
        Utils.bitmapToMat(bmp32, src);
        if (src.empty()) {
            Log.d(TAG,"detectCircle: the src matrix is empty.");
        }
        //GRAYSCALE (GPU)
        Mat gray = new Mat();
        Bitmap grayBmp = makeGrayscale(bmp32);
        Log.d(TAG,"Making grayscale done");
        capturedImageHolder.setImageBitmap(grayBmp); //makeGrayscale결과 확인용
        saveBitmap(bmp32,"original_grayscale.bmp"); //makeGrayscale결과 확인용
        saveBitmap(grayBmp,"converted_grayscale.bmp"); //makeGrayscale결과 확인용
        Bitmap emptyBitmap = Bitmap.createBitmap(grayBmp.getWidth(),
                grayBmp.getHeight(), grayBmp.getConfig());
        if (grayBmp.sameAs(emptyBitmap)) {
            Log.d(TAG,"Bitmap is empty");
        }
        Utils.bitmapToMat(grayBmp,gray);
        Imgproc.cvtColor(gray,gray,Imgproc.COLOR_BGR2GRAY);
        Log.d(TAG,"ImageType:"+gray.type());
        //삭제할 것
        //original GRAYSCALE (CPU)
//        Mat gray = new Mat();
//        Imgproc.cvtColor(src,gray,Imgproc.COLOR_BGR2GRAY);

//        Imgproc.medianBlur(gray, gray, 3);
        Mat circles = new Mat();
        Imgproc.HoughCircles(gray, circles, Imgproc.HOUGH_GRADIENT, 1.0,
                (double)gray.rows()/16, // change this value to detect circles
                                                // with different distances to each other
                100.0, 30.0, 0, 50); // change the last two parameters
        // (min_radius & max_radius) to detect larger circles
        int left_circles = 0;
        int right_circles = 0;

        //Get lines
        Mat lines = new Mat();
        Imgproc.Canny(gray, lines, 50, 150);
        Imgproc.HoughLines(lines, lines, 1, 1, 0);
        double x0 = 0.0;

        for (int i = 0; i < lines.cols(); i++) {
            double data[] = lines.get(0, i);
            double rho1 = data[0];
            double theta1 = data[1];
            double cosTheta = Math.cos(theta1);
            double sinTheta = Math.sin(theta1);
            x0 = cosTheta * rho1;
            double y0 = sinTheta * rho1;
            Point pt1 = new Point(x0 + 10000 * (-sinTheta), y0 + 10000 * cosTheta);
            Point pt2 = new Point(x0 - 10000 * (-sinTheta), y0 - 10000 * cosTheta);
            Imgproc.line(src, pt1, pt2, new Scalar(0, 0, 255), 2);
        }

        //Get Circles
        for (int x = 0; x < circles.cols(); x++) {
            double[] c = circles.get(0, x);
            if (c[0] < x0) {
                left_circles += 1;
            } else {
                right_circles += 1;
            }
            Point center = new Point(Math.round(c[0]), Math.round(c[1]));
            // circle center
            Imgproc.circle(src, center, 1, new Scalar(0,100,100), 3, 8, 0 );
            // circle outline
            int radius = (int) Math.round(c[2]);
            Imgproc.circle(src, center, radius, new Scalar(255,0,255), 3, 8, 0 );
        }



        text_left.setText("" + left_circles);
        text_right.setText("" + right_circles);

        data_int = abs(left_circles-right_circles);
        Log.d(TAG,"data_int: " + data_int);

        double percentage = ((double)right_circles/((double)left_circles + (double)right_circles)) * 8;
        Log.d(TAG,"percentage: "+percentage);

        for (int i = 0; i < 8; i++) {
            if (i+1 <= percentage)
                led[i] = 0;
            else
                led[i] = 1;
        }

        writeLEDDriver(led,led.length);

//        Log.d(TAG,"src.dims: %d, info.height: %d, info.width: %d",)
        Utils.matToBitmap(src, bitmap);

    }
    public void detectCircleCPU() {
        Mat src = new Mat();
        Bitmap bmp32 = bitmap.copy(Bitmap.Config.ARGB_8888, true);
        Utils.bitmapToMat(bmp32, src);
        if (src.empty()) {
            Log.d(TAG,"detectCircle: the src matrix is empty.");
        }

        Mat gray = new Mat();
        Imgproc.cvtColor(src,gray,Imgproc.COLOR_BGR2GRAY);

        Imgproc.medianBlur(gray, gray, 3);
        Mat circles = new Mat();
        Imgproc.HoughCircles(gray, circles, Imgproc.HOUGH_GRADIENT, 1.0,
                (double)gray.rows()/16, // change this value to detect circles with different distances to each other
                100.0, 30.0, 0, 80); // change the last two parameters
        // (min_radius & max_radius) to detect larger circles
        int left_circles = 0;
        int right_circles = 0;

        //Get lines
        Mat lines = new Mat();
        Imgproc.Canny(gray, lines, 50, 150);
        Imgproc.HoughLines(lines, lines, 1, 1, 0);
        double x0 = 0.0;

        for (int i = 0; i < lines.cols(); i++) {
            double data[] = lines.get(0, i);
            double rho1 = data[0];
            double theta1 = data[1];
            double cosTheta = Math.cos(theta1);
            double sinTheta = Math.sin(theta1);
            x0 = cosTheta * rho1;
            double y0 = sinTheta * rho1;
            Point pt1 = new Point(x0 + 10000 * (-sinTheta), y0 + 10000 * cosTheta);
            Point pt2 = new Point(x0 - 10000 * (-sinTheta), y0 - 10000 * cosTheta);
            Imgproc.line(src, pt1, pt2, new Scalar(0, 0, 255), 2);
        }

        //Get Circles
        for (int x = 0; x < circles.cols(); x++) {
            double[] c = circles.get(0, x);
            if (c[0] < x0) {
                left_circles += 1;
            } else {
                right_circles += 1;
            }
            Point center = new Point(Math.round(c[0]), Math.round(c[1]));
            // circle center
            Imgproc.circle(src, center, 1, new Scalar(0,100,100), 3, 8, 0 );
            // circle outline
            int radius = (int) Math.round(c[2]);
            Imgproc.circle(src, center, radius, new Scalar(255,0,255), 3, 8, 0 );
        }



        text_left.setText("" + left_circles);
        text_right.setText("" + right_circles);

        data_int = abs(left_circles-right_circles);
        Log.d(TAG,"data_int: " + data_int);

        double percentage = ((double)right_circles/((double)left_circles + (double)right_circles)) * 8;
        Log.d(TAG,"percentage: "+percentage);

        for (int i = 0; i < 8; i++) {
            if (i+1 <= percentage)
                led[i] = 0;
            else
                led[i] = 1;
        }

        writeLEDDriver(led,led.length);

//        Log.d(TAG,"src.dims: %d, info.height: %d, info.width: %d",)
        Utils.matToBitmap(src, bitmap);

    }

}