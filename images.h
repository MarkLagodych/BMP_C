/*
 * A simple library for processing images and working with BMP format
 */

#ifndef IMG_H
#define IMG_H

#ifdef __cplusplus
    extern "C" {
#endif


#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

// ========================== TYPES ============================

typedef uint8_t Color; // A minimal type to store a pixel channel
typedef uint32_t uint; // A type, enough to store image width/height

typedef struct {
    Color B, G, R; // BGR, not RGB -- important (!!!) for reading BMP that stores all pixel channels flipped
} Pixel;

#define PIXEL_SIZE 3

typedef struct {
    uint width, height;
    Pixel *data;
} Image;

enum Img_Errors {
    IMG_OK = 0,
    IMG_ERROR_UNABLE_TO_OPEN_FILE = 0xA0,
    IMG_ERROR_NOT_BMP,
    IMG_ERROR_UNSUPPORTED_FORMAT,
    IMG_ERROR_MEM_ALLOC_FAILED,
    IMG_ERROR_ARGUMENT_IS_NULL,
    IMG_ERROR_FILE_READ_FAILED,
    IMG_ERROR_FILE_WRITE_FAILED,
    IMG_ERROR_INVALID_ARGUMENT
};

// ============================= FUNCTIONS ==============================

// Allocate memory to store image
Image *CreateImage(uint width, uint height);

Image *CreateCompatible(Image *image); // Creates an image with the same size

// Frees allocated memory
void DeleteImage(Image *image);     // Both functions below
void DeleteData(Image *image);      // Frees only pixel data
void DeleteContainer(Image *image); // Frees only image structure, not its pixel data

// Gets an access to pixel from image
#define pixi(image, x, y) \
    (image)->data [ (y) * (image)->width + (x) ]

// Makes a pixel from given R, G, B channels
#define pix(r, g, b) \
    (Pixel) {R: (r), G: (g), B: (b)}

// Makes an avarage from two pixels with ratio a/b
Pixel avgpix(Pixel pixel1, Pixel pixel2, uint a, uint b);


void CopyImage(Image *destination, // Image where to copy to
               uint dx, uint dy,   // Position where to copy to
               Image *source);     // Image to copy

void CopyFragment(Image *destination,       // Image where to copy to
                  uint dx, uint dy,         // Position where to copy to
                  Image *source,            // Image where to copy from
                  uint sx, uint sy,         // Fragment position
                  uint width, uint height); // Fragment size

enum FlipMethod {
    FLIP_VERTICAL,
    FLIP_HORIZONTAL
};
void FlipImage(Image *image, enum FlipMethod method);

// Overlay pixels with avgpix and stores the result into image1
void OverlayImage(Image *destination,
                  uint dx, uint dy,
                  Image *source,
                  uint a, uint b);

void OverlayFragment(Image *destination,
                     uint dx, uint dy,
                     Image *source,
                     uint sx, uint sy,
                     uint width, uint height,
                     uint a, uint b);

// Rotates only on -90/90 degrees clockwise
void RotateImage(Image *image,
                 int angle);


enum InterpolationMethod {
    RESIZE_NEAREST, // Nearest neighbour
    RESIZE_AVERAGE  // Average from surrounding pixels
};
void ResizeImage(Image *image,
                 uint newWidth,
                 uint newHeight,
                 enum InterpolationMethod method);


#ifdef __cplusplus
    } // extern "C"
#endif

#endif // IMG_H
