//OpenCL kernel. Each work item takes care of one element of c
 
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
 
#define RGB8888_A(p) ((p & (0xff<<24))      >> 24 )
#define RGB8888_B(p) ((p & (0xff << 16)) >> 16 )
#define RGB8888_G(p) ((p & (0xff << 8))  >> 8 )
#define RGB8888_R(p) (p & (0xff) )
 
__kernel void kernel_gray(__global int *src, __global int *dst, const int width, const int height){
    //int id = get_global_id(0);
    
    int row = get_global_id(0)/width;
    int col = get_global_id(0)%width;
 
    int a,r, g, b;
    int gray;
 
    float red = 0, green = 0, blue = 0;
    int pixel = src[width * row + col];
    a = RGB8888_A(pixel);
    r = RGB8888_R(pixel);
    g = RGB8888_G(pixel);
    b = RGB8888_B(pixel);
 
 
    red = r * 0.2126;
    green = g * 0.7152;
    blue = b * 0.0722;
    gray = red+green+blue;
 
    int v = (a << 24) + (gray << 16) + (gray << 8) + (gray);
    dst[width * row + col] = v;
}
