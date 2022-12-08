#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <jni.h>

int fd = 0;
int fd_segment = 0;

JNIEXPORT jint JNICALL
Java_com_example_finalproject_MainActivity_openLEDDriver(JNIEnv *env, jclass clazz, jstring path) {
    jboolean iscopy;
    const char *path_utf = (*env)->GetStringUTFChars(env, path, &iscopy);
    fd = open(path_utf, O_WRONLY);
    (*env)->ReleaseStringUTFChars(env, path, path_utf);

    if(fd<0) return -1;
    else return 1;
}

JNIEXPORT void JNICALL
Java_com_example_finalproject_MainActivity_closeLEDDriver(JNIEnv *env, jclass clazz) {
if(fd>0) close(fd);
}

JNIEXPORT void JNICALL
Java_com_example_finalproject_MainActivity_writeLEDDriver(JNIEnv *env, jclass clazz, jbyteArray data, jint length) {
jbyte* chars = (*env)->GetByteArrayElements(env, data, 0);
if(fd>0) write(fd, (unsigned char*)chars, length);
(*env)->ReleaseByteArrayElements(env, data, chars, 0);
}

JNIEXPORT jint JNICALL
Java_com_example_finalproject_MainActivity_openSegmentDriver(JNIEnv *env, jclass clazz, jstring path) {
    jboolean iscopy;
    const char *path_utf = (*env)->GetStringUTFChars(env, path, &iscopy);
    fd_segment = open(path_utf, O_WRONLY);
    (*env)->ReleaseStringUTFChars(env, path, path_utf);

    if(fd_segment<0) return -1;
    else return 1;
}

JNIEXPORT void JNICALL
Java_com_example_finalproject_MainActivity_closeSegmentDriver(JNIEnv *env, jclass clazz) {
    if(fd_segment>0) close(fd_segment);
}

JNIEXPORT void JNICALL
Java_com_example_finalproject_MainActivity_writeSegmentDriver(JNIEnv *env, jclass clazz, jbyteArray data, jint length) {
    jbyte* chars = (*env)->GetByteArrayElements(env, data, 0);
    if(fd_segment>0) write(fd_segment, (unsigned char*)chars, length);
    (*env)->ReleaseByteArrayElements(env, data, chars, 0);
}