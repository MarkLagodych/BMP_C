#include "../images.h"
#include "../bmp.h"

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
