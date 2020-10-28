#include "images.h"

#define MIN(a, b) ((a)<(b) ? (a) : (b))

Pixel avgpix(Pixel pixel1, Pixel pixel2, uint a, uint b) {
    if (a + b == 0) {
        return pix (
            (pixel1.R + pixel2.R) / 2,
            (pixel1.G + pixel2.G) / 2,
            (pixel1.B + pixel2.B) / 2
        );
    } else {
        return pix (
            (pixel1.R * a + pixel2.R * b) / (a + b),
            (pixel1.G * a + pixel2.G * b) / (a + b),
            (pixel1.B * a + pixel2.B * b) / (a + b)
        );
    }
}

Image *CreateImage(uint width, uint height) {

    Image *newImage = (Image *) malloc(sizeof(Image));

    if (!newImage) {
        errno = IMG_ERROR_MEM_ALLOC_FAILED;
        return NULL;
    }

    newImage->width = width;
    newImage->height = height;

    newImage->data = calloc(width * height, sizeof(Pixel));

    if (!newImage->data) {
        free(newImage);
        errno = IMG_ERROR_MEM_ALLOC_FAILED;
        return NULL;
    }

    return newImage;

}


inline Image *CreateCompatible(Image *image) {
    if (!image) {
        errno = IMG_ERROR_ARGUMENT_IS_NULL;
        return NULL;
    }
    return CreateImage(image->width, image->height);
}


void DeleteImage(Image *image) {

    DeleteData(image);
    DeleteContainer(image);

}


void DeleteData(Image *image) {

    if (!image) {
        errno = IMG_ERROR_ARGUMENT_IS_NULL;
        return;
    }

    if (image->data)
        free(image->data);

}


void DeleteContainer(Image *image) {

    if (!image) {
        errno = IMG_ERROR_ARGUMENT_IS_NULL;
        return;
    }

    free(image);

}


void CopyFragment(Image *dst, uint dx, uint dy,
                  Image *src, uint sx, uint sy,
                  uint width, uint height)
{
    if (!dst || !dst->data || !src || !src->data) {
        errno = IMG_ERROR_ARGUMENT_IS_NULL;
        return;
    }

    if (width  + dx > dst->width)  width  = dst->width  - dx;
    if (height + dy > dst->height) height = dst->height - dy;

    for (uint y = 0; y < height; y++)
        memcpy(
            dst->data + (dy+y) * dst->width + dx,
            src->data + (sy+y) * src->width + sx,
            width * sizeof(Pixel)
        );
}



inline void CopyImage(Image *dst, uint dx, uint dy,
                      Image *src)
{
    CopyFragment(dst, dx, dy, src, 0, 0, src->width, src->height);
}



void FlipImage(Image *image, enum FlipMethod method) {
    if (!image || !image->data) {
        errno = IMG_ERROR_ARGUMENT_IS_NULL;
        return;
    }

    Image *tmpImage = CreateImage(image->width, image->height);
    if (!tmpImage) return;

    switch (method) {
        case FLIP_VERTICAL: {
            for (uint y = 0; y < image->height; y++)
                memcpy(
                    tmpImage->data + (image->height - 1 - y) * image->width,
                    image->data + y * image->width,
                    image->width * sizeof(Pixel)
                );
        }

        case FLIP_HORIZONTAL: {
            for (uint y = 0; y < image->height; y++)
                for (uint x = 0; x < image->width; x++)
                    pixi(tmpImage, x, y) = pixi(image, image->width - 1 - x, y);
        }

        default: {
            DeleteImage(tmpImage);
            errno = IMG_ERROR_INVALID_ARGUMENT;
            return;
        }

    }

    DeleteData(image);
    image->data = tmpImage->data;
    DeleteContainer(tmpImage);
}


void OverlayFragment(Image *dst, uint dx, uint dy,
                     Image *src, uint sx, uint sy,
                     uint width, uint height,
                     uint a, uint b)
{
    if (!dst || !dst->data || !src || !src->data) {
        errno = IMG_ERROR_ARGUMENT_IS_NULL;
        return;
    }

    if (width  + dx > dst->width ) width  = dst->width  - dx;
    if (height + dy > dst->height) height = dst->height - dy;

    for (uint y = 0; y < height; y++)
        for (uint x = 0; x < width; x++)
            pixi(dst, dx+x, dy+y) = avgpix(pixi(dst, dx+x, dy+y), pixi(src, sx+x, sy+y), a, b);
}


inline void OverlayImage(Image *dst, uint dx, uint dy,
                         Image *src,
                         uint a, uint b)
{
    if (!src) {
        errno = IMG_ERROR_ARGUMENT_IS_NULL;
        return;
    }
    OverlayFragment(dst, dx, dy, src, 0, 0, src->width, src->height, a, b);
}


void RotateImage(Image *image, int a) {
    if (!image || !image->data) {
        errno = IMG_ERROR_ARGUMENT_IS_NULL;
        return;
    }

    if (a != 90 && a != -90) {
        errno = IMG_ERROR_INVALID_ARGUMENT;
        return;
    }

    Image *newImage = CreateImage(image->height, image->width);
    if (!newImage) return;

    if (a == 90) {
        for (uint y = 0; y < image->height; y++)
            for (uint x = 0; x < image->width; x++)
                pixi(newImage, y, x) = pixi(image, image->width - 1 - x, y);

    } else {
        for (uint y = 0; y < image->height; y++)
            for (uint x = 0; x < image->width; x++)
                pixi(newImage, y, x) = pixi(image, x, image->height - 1 - y);

    }

    DeleteData(image);
    image->data = newImage->data;
    image->width = newImage->width;
    image->height = newImage->height;
    DeleteContainer(newImage);
}


// All destination pixels are dyed into nearest source pixels color
void _Resize_NEAREST(Image *dst, Image *src) {
    // coefficients for finding the size of the source image from the destination
    float kx = (float) src->width / dst->width,
          ky = (float) src->height / dst->height;

    for (uint y = 0; y < dst->height; y++)
        for (uint x = 0; x < dst->width; x++)
            pixi(dst, x, y) = pixi(
                src,
                MIN((uint) (kx * x), src->width-1),
                MIN((uint) (ky * y), src->height-1)
            );
}

// All destination pixels are dyed into an average of 4 nearest source pixels colors
void _Resize_AVERAGE(Image *dst, Image *src) {
    // coefficients for finding the size of the source image from the destination
    float kx = (float) src->width / dst->width,
          ky = (float) src->height / dst->height;

    for (uint y = 0; y < dst->height; y++)
        for (uint x = 0; x < dst->width; x++) {

            // Compute coordinates of 4 nearest pixels
            uint x1 = floor(kx * x), x2 = ceil(kx * x);
            uint y1 = floor(ky * y), y2 = ceil(ky * y);
            x1 = MIN(x1, src->width-1);
            x2 = MIN(x2, src->width-1);
            y1 = MIN(y1, src->height-1);
            y2 = MIN(y2, src->height-1);

            // Compute average and dye
            pixi(dst, x, y).R = (
                pixi(src, x1, y1).R +
                pixi(src, x1, y2).R +
                pixi(src, x2, y1).R +
                pixi(src, x2, y2).R
            ) / 4;

            pixi(dst, x, y).G = (
                pixi(src, x1, y1).G +
                pixi(src, x1, y2).G +
                pixi(src, x2, y1).G +
                pixi(src, x2, y2).G
            ) / 4;

            pixi(dst, x, y).B = (
                pixi(src, x1, y1).B +
                pixi(src, x1, y2).B +
                pixi(src, x2, y1).B +
                pixi(src, x2, y2).B
            ) / 4;

        }
}


void ResizeImage(Image *image, uint newWidth, uint newHeight, enum InterpolationMethod method) {
    if (!image || !image->data) {
        errno = IMG_ERROR_ARGUMENT_IS_NULL;
        return;
    }

    Image *newImage = CreateImage(newWidth, newHeight);


    switch (method) {

        case RESIZE_NEAREST: {
            _Resize_NEAREST(newImage, image);
            break;
        }

        case RESIZE_AVERAGE: {
            _Resize_AVERAGE(newImage, image);
            break;
        }

        default: {
            DeleteImage(newImage);
            errno = IMG_ERROR_INVALID_ARGUMENT;
            return;
        }

    }

    DeleteData(image);
    image->data = newImage->data;
    image->width = newImage->width;
    image->height = newImage->height;
    DeleteContainer(newImage);
}
