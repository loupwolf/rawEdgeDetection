
#include "header.h"

int main() {
    notsimd("parrots_512x256.raw\0", 512, 256, 5); // 3 is for the kernel size, can be 5 or 7 too
    issimd("parrots_512x256.raw\0", 512, 256, 5); // 3 is for the kernel size, can be 5 or 7 too
    // We can see that the min/max filtering in C without SIMD is much slower that the same algorithm implemented in SIMD.
    // The outputs (images) with a bigger kernel gives more details on the contours.
    return 0;
}
