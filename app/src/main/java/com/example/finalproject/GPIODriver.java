package com.example.finalproject;

import android.util.Log;

public class GPIODriver implements GPIOListener{
    private boolean mConnectFlag;

    private TranseThread mTranseThread;
    private GPIOListener mMainActivity;
    static {
        System.loadLibrary("GPIODriver");
    }

    private native static int openGPIODriver(String path);
    private native static void closeGPIODriver();
    private native char readGPIODriver();

    private native int getGPIOInterrupt();

    public GPIODriver() {mConnectFlag=false;}

    @Override
    public void onReceive(int val) {
        Log.e("test","4");
        if(mMainActivity != null) {
            mMainActivity.onReceive(val);
            Log.e("test", "2");
        }
    }
    public void setListener(GPIOListener a) {mMainActivity = a;}
    public int open(String driver) {
        if (mConnectFlag) return -1;

        if(openGPIODriver(driver) > 0) {
            mConnectFlag = true;
            mTranseThread = new TranseThread();
            mTranseThread.start();
            return 1;
        } else {
            return -1;
        }
    }
    public void close() {
        if(!mConnectFlag) return;
        mConnectFlag = false;
        closeGPIODriver();
    }
    protected void finalize() throws Throwable{
        close();
        super.finalize();
    }
    public char read() { return readGPIODriver(); }
    private class TranseThread extends Thread {

        @Override
        public void run() {
            super.run();
            try {
                while(mConnectFlag) {
                    try{
                        Log.e("test", "1");
                        onReceive(getGPIOInterrupt());
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            } catch (Exception e) {

            }
        }
    }
}
