#include "bmp.h"



// Plantform-idependently reads a little-endian integer from a file
uint32_t freadi32(FILE *file) {
    uint32_t ret = 0;
    for (uint32_t mult=1, i=0; i<4; i++, mult*=256)
        ret += mult * (uint32_t) fgetc(file);
    return ret;
}

uint16_t freadi16(FILE *file) {
    uint16_t ret = 0;
    ret += (uint16_t) fgetc(file);
    ret += 256 * (uint16_t) fgetc(file);
    return ret;
}


void fwritei16(FILE *file, uint16_t x) {
    fputc(x % 256, file);
    fputc(x / 256, file);
}


void fwritei32(FILE *file, uint32_t x) {
    for (int i=0; i<4; i++, x/=256)
        fputc(x % 256, file);
}



Image *BMP_Read(const char *fileName) {

    // =========================== OPEN FILE =========================

    FILE *file = fopen(fileName, "rb");

    if (!file) {
        errno = IMG_ERROR_UNABLE_TO_OPEN_FILE;
        return NULL;
    }

    // ============================ READ HEADERS ============================
    BMP_Header header;
    BMP_Info   info;

    #define _READ_U16(v) (v) =           freadi16(file);
    #define _READ_S16(v) (v) = (int16_t) freadi16(file);
    #define _READ_U32(v) (v) =           freadi32(file);
    #define _READ_S32(v) (v) = (int32_t) freadi32(file);

    _READ_U16(header.fileSignature)
    _READ_U32(header.fileSize)
    _READ_U16(header.reserved1)
    _READ_U16(header.reserved2)
    _READ_U32(header.dataOffset)

    _READ_U32(info.infoSize)
    _READ_S32(info.width)
    _READ_S32(info.height)
    _READ_U16(info.planesCount)
    _READ_U16(info.bitsPerPixel)
    _READ_U32(info.compression)
    _READ_U32(info.dataSize)
    _READ_S32(info.hPixelsPerMeter)
    _READ_S32(info.vPixelsPerMeter)
    _READ_U32(info.usedColors)
    _READ_U32(info.importantColors)

    #undef _READ_U16
    #undef _READ_S16
    #undef _READ_U32
    #undef _READ_S32


    // ========================== CHECK HEADERS ======================

    if (header.fileSignature != BMP_SIGNATURE) {
        printf("%i\n", header.fileSignature);
        fclose(file);
        errno = IMG_ERROR_NOT_BMP;
        return NULL;
    }

    if (
        !(info.bitsPerPixel == 24 || info.bitsPerPixel == 0)
        || info.compression  != 0
    ) {
        fclose(file);
        errno = IMG_ERROR_UNSUPPORTED_FORMAT;
        return NULL;
    }

    int compl = info.width % 4; // BMP feature that all rows must be complemented to 4 with 0..3 additional 0s

    // ============================= ALLOCATE MEMORY FOR IMAGES ==============================
    Image *image = CreateImage(info.width, info.height);

    if (!image) {
        fclose(file);
        errno = IMG_ERROR_MEM_ALLOC_FAILED;
        return NULL;
    }

    // ============================== READ PIXEL DATA ==============================
    for (int i = info.height-1; i >= 0; i--) // For each row
        if (
            fread(image->data + i*info.width, PIXEL_SIZE, info.width, file) // Try to read the row
            != info.width
        ) {                           // If reading fails
            fclose(file);             // Close everything
            DeleteImage(image);
            errno = IMG_ERROR_FILE_READ_FAILED;
            return NULL;
        } else                                       // But if the reading succeeds,
            for (int i = compl; i > 0; i--)          // Complement the width to 4 (BMP feature)
                fgetc(file);

    fclose(file);

    errno = IMG_OK;
    return image;

}



void BMP_Write(Image *image, const char *fileName) {

    if (!image || !image->data) {
        errno = IMG_ERROR_ARGUMENT_IS_NULL;
        return;
    }

    // ========================= START WRITING TO FILE ==========================

    size_t headersSize = BMP_HEADER_SIZE + BMP_INFO_SIZE; // Size in bytes
    size_t imageSize = image->width * image->height;      // Size in pixels

    // BMP complementation
    bool compl = image->width % 4;

    // Size in bytes
    size_t dataSize;

    if (compl)
        dataSize = imageSize * PIXEL_SIZE + image->height * compl; // Every row must be ended with additional 0s
    else
        dataSize = imageSize * PIXEL_SIZE;

    BMP_Header header = {
        fileSignature: BMP_SIGNATURE,
        fileSize:      headersSize + dataSize,
        reserved1:     0,
        reserved2:     0,
        dataOffset:    headersSize
    };

    BMP_Info info = {
        infoSize:        BMP_INFO_SIZE,
        width:           image->width,
        height:          image->height,
        planesCount:     1,
        bitsPerPixel:    24,
        compression:     0,
        dataSize:        dataSize,
        hPixelsPerMeter: 0,
        vPixelsPerMeter: 0,
        usedColors:      0,
        importantColors: 0
    };

    FILE *file = fopen(fileName, "wb");

    if (!file) {
        errno = IMG_ERROR_UNABLE_TO_OPEN_FILE;
        return;
    }

    #define _WRITE_U16(v) fwritei16(file,           (v));
    #define _WRITE_S16(v) fwritei16(file, (uint16_t)(v));
    #define _WRITE_U32(v) fwritei32(file,           (v));
    #define _WRITE_S32(v) fwritei32(file, (uint32_t)(v));

    _WRITE_U16(header.fileSignature)
    _WRITE_U32(header.fileSize)
    _WRITE_U16(header.reserved1)
    _WRITE_U16(header.reserved2)
    _WRITE_U32(header.dataOffset)

    _WRITE_U32(info.infoSize)
    _WRITE_S32(info.width)
    _WRITE_S32(info.height)
    _WRITE_U16(info.planesCount)
    _WRITE_U16(info.bitsPerPixel)
    _WRITE_U32(info.compression)
    _WRITE_U32(info.dataSize)
    _WRITE_S32(info.hPixelsPerMeter)
    _WRITE_S32(info.vPixelsPerMeter)
    _WRITE_U32(info.usedColors)
    _WRITE_U32(info.importantColors)

    #undef _WRITE_U16
    #undef _WRITE_S16
    #undef _WRITE_U32
    #undef _WRITE_S32

    // ============================== WRITE PIXEL DATA ==============================
    for (int i = info.height-1; i >= 0; i--)
        if (
            fwrite(image->data + i*info.width, sizeof(Pixel), info.width, file)
            != info.width
        ) {                                           // If writing fails
            fclose(file);                             // Close everything
            errno = IMG_ERROR_FILE_WRITE_FAILED;
            return;

        } else                                        // If writing succeded
            for (int i = compl; i > 0; i--)           // Complement the width to 4
                fputc(0, file);



    fclose(file);
    errno = IMG_OK;

}

