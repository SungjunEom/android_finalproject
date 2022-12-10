package com.example.finalproject;

public class SegmentDriver {
    private boolean mConnectFlag;

    static {
        System.loadLibrary("SegmentDriver");
    }

    private native static int openSegmentDriver(String path);
    private native static void closeSegmentDriver();
    private native static void writeSegmentDriver(byte[] data, int length);

    public SegmentDriver() {
        mConnectFlag = false;
    }

    public int open(String driver) {
        if(mConnectFlag) return -1;

        if(openSegmentDriver(driver)>0) {
            mConnectFlag = true;
            return 1;
        } else {
            return -1;
        }
    }

    public void close() {
        if(!mConnectFlag) return;
        mConnectFlag = false;
        closeSegmentDriver();
    }

    protected void finalize() throws Throwable {
        close();
        super.finalize();
    }

    public void write(byte[] data) {
        if(!mConnectFlag) return;

        writeSegmentDriver(data,data.length);
    }
}
