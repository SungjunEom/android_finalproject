#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <CL/opencl.h>
#include "ImageProcessing.h"

#define CL_FILE "/data/local/tmp/Grayscale.cl"
#define LOG_TAG "DEBUG"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define CHECK_CL(expression) {                        \
	cl_int err = (expression);                       \
	if (err < 0 && err > -64) {                      \
		printf("Error on line %d. error code: %d\n", \
				__LINE__, err);                      \
		exit(0);                                     \
	}                                                \
}

//int opencl_infra_creation(cl_context&       context,
//                          cl_platform_id&   cpPlatform,
//                            cl_device_id&     device_id,
//                            cl_command_queue& queue,
//                            cl_program&       program,
//                            cl_kernel&        kernel,
//                            char*             kernel_file_buffer,
//                            size_t            kernel_file_size,
//                            BMPHEADER&        bmpHeader,
//                            unsigned char*    kernel_name,
//                            cl_mem&           d_src,
//                            cl_mem&           d_dst
//                            );
//
//int launch_the_kernel(cl_context&       context,
//                    cl_command_queue& queue,
//                    cl_kernel&        kernel,
//                    size_t            globalSize,
//                    size_t            localSize,
//                    BMPHEADER&        bmpHeader,
//                    cl_mem&           d_src,
//                    cl_mem&           d_dst,
//                    unsigned char*    image,
//                    unsigned char*    blured_img
//                    );

JNIEXPORT jobject JNICALL
Java_com_example_finalproject_MainActivity_makeGrayscale
        (JNIEnv *env, jclass clazz , jobject bitmap)
{
//    jbyte * pszBuf = env->GetByteArrayElements( bitmap, NULL );
//    jbyteArray result;
//    int iBufSize = env->GetArrayLength( bitmap );

    //reading bmp
    LOGD("reading bitmap info...");
    AndroidBitmapInfo info;
    int ret;
    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return NULL;
    }
    LOGD("width:%d height:%d stride:%d", info.width, info.height, info.stride);
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not RGBA_8888!");
        return NULL;
    }


    //read pixels of bitmap into native memory :
    LOGD("reading bitmap pixels...");
    void* bitmapPixels;
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return NULL;
    }
    uint32_t* src = (uint32_t*) bitmapPixels;
    uint32_t* tempPixels = (uint32_t*)malloc(info.height * info.width*4);
    int pixelsCount = info.height * info.width;
    memcpy(tempPixels, src, sizeof(uint32_t) * pixelsCount);
    //ending bmp reading

    FILE *file_handle;
    char *kernel_file_buffer, *file_log;
    size_t kernel_file_size, log_size;
    struct timeval start, end, timer;

    BMPHEADER bmpHeader;
    unsigned char *image;
    unsigned char *blured_img;

    //unsigned char* cl_file_name = (unsigned char *) "Grayscale.cl";
    unsigned char* kernel_name = (unsigned char *) "kernel_grayscale";

    // Device input buffers
    cl_mem d_src;
    // Device output buffer
    cl_mem d_dst;

    cl_platform_id clPlatform;        // OpenCL platform
    cl_device_id device_id;           // device ID
    cl_context context;               // context
    cl_command_queue queue;           // command queue
    cl_program program;               // program
    cl_kernel kernel;                 // kernel

    file_handle=fopen(CL_FILE, "r");
    if(file_handle==NULL)
    {
        printf("Couldn't find the file");
        exit(1);
    }
    printf("Program initiated\n");

    //read kernel file
    fseek(file_handle, 0, SEEK_END);
    kernel_file_size =ftell(file_handle);
    rewind(file_handle);
    kernel_file_buffer = (char*)malloc(kernel_file_size+1);//TODO
    kernel_file_buffer[kernel_file_size]='\0';
    fread(kernel_file_buffer, sizeof(char),
          kernel_file_size, file_handle);
    fclose(file_handle);

    //read image to processing
    // Initialize vectors on host
    int i;

    size_t globalSize, localSize, grid;
    cl_int err;

    // Number of work items in each local work group
    localSize = 64;
    int n_pix = info.width * info.height;

    //Number of total work items - localSize must be devisor
    grid = (n_pix % localSize) ? (n_pix / localSize) + 1 : n_pix / localSize;
    globalSize = grid * localSize;

    LOGD("calc grid and globalSize");
    //openCL 기반 실행

    //Bind to platform
    LOGD("error check");
    CHECK_CL(clGetPlatformIDs(1, &clPlatform, NULL));
    LOGD("error end check");
    //Get ID for the device
    CHECK_CL(clGetDeviceIDs(clPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL));
    //Create a context
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    CHECK_CL(err);
    //Create a command queue
    queue = clCreateCommandQueue(context, device_id, 0, &err);
    CHECK_CL(err);
    //Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1,
                                        (const char**)&kernel_file_buffer,
                                        &kernel_file_size, &err);
    CHECK_CL(err);
    //Build the program executable
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    LOGD("error 22 check");
    CHECK_CL(err);
    if (err != CL_SUCCESS) {
        //LOGD("%s", err);
        size_t len;
        char buffer[4096];
        LOGD("Error: Failed to build program executable!");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer),
                              buffer, &len);

        LOGD("%s", buffer);
        //exit(1);
    }
    LOGD("error 323 check");

    //Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, kernel_name, &err);
    CHECK_CL(err);

    LOGD("Starting Kernel");

    //////openCL 커널 수행
    //Create the input and output arrays in device memory for our calculation
    d_src = clCreateBuffer(context, CL_MEM_READ_ONLY,
                           sizeof(uint32_t)*info.width*info.height, NULL, NULL);
    d_dst = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                           sizeof(uint32_t)*info.width*info.height, NULL, NULL);

    LOGD("Creating buffers finished");
    //Write our data set into the input array in device memory
    CHECK_CL(clEnqueueWriteBuffer(queue, d_src, CL_TRUE, 0,
                                  sizeof(uint32_t)*info.width*info.height,
                                  tempPixels, 0, NULL, NULL));

    //Set the arguments to our compute kernel
    CHECK_CL(clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_src));
    CHECK_CL(clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_dst));
    CHECK_CL(clSetKernelArg(kernel, 2, sizeof(uint32_t), &info.width));
    CHECK_CL(clSetKernelArg(kernel, 3, sizeof(uint32_t), &info.height));


    LOGD("Setting kernel arguments finished, Kernel initiated");
    //Execute the kernel over the entire range of the data set
    CHECK_CL(clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL));
    //Wait for the command queue to get serviced before reading back results
    CHECK_CL(clFinish(queue));
    //read the results form the device
    CHECK_CL(clEnqueueReadBuffer(queue, d_dst, CL_TRUE, 0,
                                 sizeof(uint32_t)*info.width*info.height, src, 0, NULL, NULL));


    // release OpenCL resources
    CHECK_CL(clReleaseMemObject(d_src));
    CHECK_CL(clReleaseMemObject(d_dst));
    CHECK_CL(clReleaseProgram(program));
    CHECK_CL(clReleaseKernel(kernel));
    CHECK_CL(clReleaseCommandQueue(queue));
    CHECK_CL(clReleaseContext(context));
    LOGD("Ending Kernel");

    AndroidBitmap_unlockPixels(env, bitmap);
    //
    // free the native memory used to store the pixels
    //
    free(tempPixels);
    return bitmap;
//
//    env->ReleaseByteArrayElements( bitmap, pszBuf, 0 );
}

//int opencl_infra_creation(cl_context&       context,
//                        cl_platform_id&   cpPlatform,
//                        cl_device_id&     device_id,
//                        cl_command_queue& queue,
//                        cl_program&       program,
//                        cl_kernel&        kernel,
//                        char*             kernel_file_buffer,
//                                size_t            kernel_file_size,
//                        BMPHEADER&        bmpHeader,
//                        unsigned char*    kernel_name,
//                                cl_mem&           d_src,
//                        cl_mem&           d_dst
//                        ) {
//
//                        cl_int err;
//
//                        // Bind to platform
//                        CHECK_CL(clGetPlatformIDs(1, &cpPlatform, NULL));
//
//                        // Get ID for the device
//                        CHECK_CL(clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL));
//
//                        // Create a context
//                        context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
//
//                        // Create a command queue
//                        queue = clCreateCommandQueue(context, device_id, 0, &err);
//                        CHECK_CL(err);
//
//                        // Create the compute program from the source buffer
//                        program = clCreateProgramWithSource(context, 1,
//                                                            (const char **) & kernel_file_buffer, &kernel_file_size, &err);
//                        CHECK_CL(err);
//
//                        // Build the program executable
//                        CHECK_CL(clBuildProgram(program, 0, NULL, NULL, NULL, NULL));
//
//                        // Create the compute kernel in the program we wish to run
//                        kernel = clCreateKernel(program,
//                                                reinterpret_cast<const char *>(kernel_name), &err);
//                        CHECK_CL(err);
//
//                        return 0;
//}
//
//int launch_the_kernel(cl_context&       context,
//                    cl_command_queue& queue,
//                    cl_kernel&        kernel,
//                    size_t            globalSize,
//                            size_t            localSize,
//                    BMPHEADER&        bmpHeader,
//                    cl_mem&           d_src,
//                    cl_mem&           d_dst,
//                    unsigned char*    image,
//                    unsigned char*    blured_img
//                    ) {
//
//                    cl_int err;
//                    struct timeval start, end, timer;
//
//                    // Create the input and output arrays in device memory for our calculation
//                    d_src = clCreateBuffer(context, CL_MEM_READ_ONLY, bmpHeader.biSizeImage, NULL, &err);
//                    CHECK_CL(err);
//                    d_dst = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bmpHeader.biSizeImage, NULL, &err);
//                    CHECK_CL(err);
//
//                    // Write our data set into the input array in device memory
//                    CHECK_CL(clEnqueueWriteBuffer(queue, d_src, CL_TRUE, 0,
//                                                 bmpHeader.biSizeImage, image, 0, NULL, NULL));
//
//                    // Set the arguments to our compute kernel
//                    CHECK_CL(clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_src));
//                    CHECK_CL(clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_dst));
//                    CHECK_CL(clSetKernelArg(kernel, 2, sizeof(int), &bmpHeader.biWidth));
//                    CHECK_CL(clSetKernelArg(kernel, 3, sizeof(int), &bmpHeader.biHeight));
//
//                    gettimeofday(&start, NULL);
//
//                    // Execute the kernel over the entire range of the data set
//                    CHECK_CL(clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize,
//                                                   0, NULL, NULL));
//                    // Wait for the command queue to get serviced before reading back results
//                    CHECK_CL(clFinish(queue));
//
//                    // Read the results from the device
//                    CHECK_CL(clEnqueueReadBuffer(queue, d_dst, CL_TRUE, 0,
//                                                bmpHeader.biSizeImage, blured_img, 0, NULL, NULL ));
//                    gettimeofday(&end, NULL);
//
//                    timersub(&end, &start, &timer);
//                    printf("GPUtime : %lf\n", (timer.tv_usec/1000.0 + timer.tv_sec *1000.0));
//
//                    return 0;
//}


