#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <jni.h>

int fd_segment = 0;

JNIEXPORT jint JNICALL
Java_com_example_finalproject_SegmentDriver_openSegmentDriver(JNIEnv *env, jclass clazz,
                                                              jstring path) {
    jboolean iscopy;
    const char *path_utf = (*env)->GetStringUTFChars(env, path, &iscopy);
    fd_segment = open(path_utf, O_WRONLY);
    (*env)->ReleaseStringUTFChars(env, path, path_utf);

    if(fd_segment<0) return -1;
    else return 1;
}

JNIEXPORT void JNICALL
Java_com_example_finalproject_SegmentDriver_closeSegmentDriver(JNIEnv *env, jclass clazz) {
if(fd_segment>0) close(fd_segment);
}

JNIEXPORT void JNICALL
Java_com_example_finalproject_SegmentDriver_writeSegmentDriver(JNIEnv *env,
                                                               jclass clazz,
                                                               jbyteArray data,
                                                               jint length) {
jbyte* chars = (*env)->GetByteArrayElements(env, data, 0);
if(fd_segment>0) write(fd_segment, (unsigned char*)chars, length);
(*env)->ReleaseByteArrayElements(env, data, chars, 0);
}