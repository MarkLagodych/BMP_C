/*
 * A simple BMP read/write library.
 * !! Only 24bit RGB bitmaps allowed !!
 *
 */


#ifndef BMP_H
#define BMP_H

#include "images.h"

// ======================= TYPES =========================

#define BMP_SIGNATURE 0x4d42 // 'B' 'M' but in little-endian

// These structures are present in all BMP files
typedef struct {
    uint16_t fileSignature;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;
} BMP_Header;

#define BMP_HEADER_SIZE 14

typedef struct {
    uint32_t infoSize;
     int32_t width;
     int32_t height;
    uint16_t planesCount;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t dataSize;
     int32_t hPixelsPerMeter;
     int32_t vPixelsPerMeter;
    uint32_t usedColors;
    uint32_t importantColors;
} BMP_Info;

#define BMP_INFO_SIZE 40

// ========================= FUNCTIONS =======================

// Reads a BMP file
// Only 24-bit RGB BMPs allowed
Image *BMP_Read(const char *fileName);

// Writes an image into a file
void BMP_Write(Image *image, const char *fileName);

#endif // BMP_H
