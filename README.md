# BMP_C
## A simple C library for operating on images and working with BMP files

`images.c`, `images.h` include functions, needed to operate on images

`bmp.c`, `bmp.h` include functions, needed to read/write simple 24-bit RGB .bmp files


# Example

```c
#include "images.h"
#include "bmp.h"

int main() {

    // Read a few .bmp images
    Image *img1 = BMP_Read("1.bmp");
    Image *img2 = BMP_Read("2.bmp");

    // Create image with the same size as img1
    Image *img3 = CreateCompatible(img1);

    // Overlay img2 on the img3 (black by default)
    //  starting from point(0;0)
    // Saturation ratio img3:img2 -- 1:2
    OverlayImage(img3, 0, 0, img2, 1, 2);

    CopyFragment(img3, 130, 130, img1, 130, 130, img1->width-260, img1->height-260);

    ResizeImage(img3, 300, 200, RESIZE_AVERAGE);

    BMP_Write(img3, "result.bmp");

    // Free all the memory
    DeleteImage(img1);
    DeleteImage(img2);
    DeleteImage(img3);

    return errno;
}
```

Source images

![](https://raw.githubusercontent.com/MarkLagodych/assets/main/BMP_C/1.bmp?token=AQXYVQ6GKNIA45KESKRSL4K7TFSFQ)
![](https://raw.githubusercontent.com/MarkLagodych/assets/main/BMP_C/2.bmp?token=AQXYVQZDO4PTL375MOGBWCC7TFSKC)

Result

![](https://raw.githubusercontent.com/MarkLagodych/assets/main/BMP_C/result.bmp?token=AQXYVQ4BKWGFIXTYGGFE7VC7TFSMA)

## Brief description of main types

```c
typedef uint8_t Color;
typedef uint32_t uint;
```
`Color` is a minimal type to store a pixel channel.
`uint` is a type, enough to store image width/height


Here is the declaration of `Pixel`:
```c
typedef struct {
    Color B, G, R;
} Pixel;
```

Here is the declaration of `Image`:
```c
typedef struct {
    uint width, height;
    Pixel *data;
} Image;
```

So, when declaring `Image *img;` we declare a pointer to an image container that has a pointer to pixel data.
Why so complicated? The thing is it is easier to call functions that change images.
And because of so many pointers, we have several `Delete..` functions:

```c
// Frees allocated memory
void DeleteImage(Image *image);     // Both functions below
void DeleteData(Image *image);      // Frees only pixel data
void DeleteContainer(Image *image); // Frees only image structure, not its pixel data
```

`DeleteImage` may be called in the end of using your image.

`DeleteContainer` and `DeleteData` may be called inside your functions that operate on images.

## Brief description of main macros/functions

- `pix(r, g, b)` macro is a shortcut to create a `Pixel` structure from given R, G and B values

- `pixi(image, x, y)` macro get an access to an image pixel.
    Example:
    ```c
    pixi(img1, 10, 34) = pix(255, 255, 0);
    ```
    This code sets a pixel of img1 with coordinates (10;34) to yellow color.

- ```Pixel avgpix(Pixel pixel1, Pixel pixel2, uint a, uint b);```
  This function makes an avarage from two pixels with ratio a:b
  
- ```Image *CreateImage(uint width, uint height);```
  Allocates memory to store an image with given size

- ```Image *CreateCompatible(Image *image);```
  Creates an image with the same size