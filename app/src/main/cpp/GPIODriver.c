#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <jni.h>

int fd = 0;

JNIEXPORT jint JNICALL
Java_com_example_finalproject_GPIODriver_openGPIODriver(JNIEnv *env, jclass clazz, jstring path) {
    jboolean iscopy;
    const char *path_utf = (*env)->GetStringUTFChars(env, path, &iscopy);
    fd = open(path_utf, O_RDONLY);
    (*env)->ReleaseStringUTFChars(env, path, path_utf);

    if(fd<0) return -1;
    else return 1;
}

JNIEXPORT void JNICALL
Java_com_example_finalproject_GPIODriver_closeGPIODriver(JNIEnv *env, jclass clazz) {
if(fd>0) close(fd);
}

JNIEXPORT jchar JNICALL
Java_com_example_finalproject_GPIODriver_readGPIODriver(JNIEnv *env, jobject obj) {
    char ch = 0;

    if (fd>0) {
        read(fd, &ch,1);
    }

    return ch;
}

JNIEXPORT jint JNICALL
Java_com_example_finalproject_GPIODriver_getGPIOInterrupt(JNIEnv *env, jobject obj) {
    int ret = 0;
    char value[100];
    char* ch1 = "Up";
    char* ch2 = "Down";
    char* ch3 = "Left";
    char* ch4 = "Right";
    char* ch5 = "Center";
    ret = read(fd, &value,100);

    if(ret<0)
        return -1;
    else {
        if(strcmp(ch1,value) == 0)
            return 1;
        else if (strcmp(ch2,value) == 0)
            return 2;
        else if (strcmp(ch3,value) == 0)
            return 3;
        else if (strcmp(ch4,value) == 0)
            return 4;
        else if (strcmp(ch5,value) == 0)
            return 5;
    }
    return 0;
}

//JNIEXPORT void JNICALL
//Java_com_example_switch_1example_JNIDriver_writeDriver(JNIEnv *env, jclass clazz, jbyteArray data, jint length) {
//jbyte* chars = (*env)->GetByteArrayElements(env, data, 0);
//if(fd>0) write(fd, (unsigned char*)chars, length);
//(*env)->ReleaseByteArrayElements(env, data, chars, 0);
//}