//
// Created by Firework on 2022-12-01.
//

#ifndef FINAL_PROJECT_IMAGEPROCESSING_H
#define FINAL_PROJECT_IMAGEPROCESSING_H
typedef struct __attribute__((__packed__)) BMPHeader
{
    char bfType[2];       /* "BM" */
    int bfSize;           /* Size of file in bytes */
    int bfReserved;       /* set to 0 */
    int bfOffBits;        /* Byte offset to actual bitmap data (= 54) */
    int biSize;           /* Size of BITMAPINFOHEADER, in bytes (= 40) */
    int biWidth;          /* Width of image, in pixels */
    int biHeight;         /* Height of images, in pixels */
    short biPlanes;       /* Number of planes in target device (set to 1) */
    short biBitCount;     /* Bits per pixel (24 in this case) */
    int biCompression;    /* Type of compression (0 if no compression) */
    int biSizeImage;      /* Image size, in bytes (0 if no compression) */
    int biXPelsPerMeter;  /* Resolution in pixels/meter of display device */
    int biYPelsPerMeter;  /* Resolution in pixels/meter of display device */
    int biClrUsed;        /* Number of colors in the color table (if 0, use
                             maximum allowed by biBitCount) */
    int biClrImportant;   /* Number of important colors.  If 0, all colors
                             are important */
}BMPHEADER;

int write_bmp(const char *filename, int width, int height, char *rgb);

char *read_bmp(unsigned char *filename, BMPHEADER *bmpHeader);


#endif //FINAL_PROJECT_IMAGEPROCESSING_H
