#include "header.h"

void assembly3(int W, int ii, unsigned char* src, unsigned char *dst);
void assembly5(int W, int ii, unsigned char* src, unsigned char *dst);
void assembly7(int W, int ii, unsigned char* src, unsigned char *dst);

int issimd(char* filename, int width, int height, int kernelSize){
    clock_t start, end;
    start = clock();

    int W=width, H=height; // Image size fixed (or from header)
    unsigned char *src; // Pointers to arrays
    unsigned char *dst;

    // Allocate memory
    src = (unsigned char *) malloc (W*H*sizeof(unsigned char));
    dst = (unsigned char *) malloc (W*H*sizeof(unsigned char));
    //W = ( int ) malloc (sizeof(int));
    // Check if enough memory
    if (src == NULL || dst == NULL) {
        printf ("Out of memory!\n");
        exit (1);
    }
    // File pointer, open and read

    if(kernelSize!=7 && kernelSize!=5 && kernelSize!=3){
        printf("The kernel size should be either 3, 5 or 7\n");
        exit(1);
    }

    FILE *fp1 = fopen(filename,"r");
    if (fp1 != NULL) { // read only if file opened
    fread(src, sizeof(signed char), W*H, fp1);
    fclose(fp1);} // we close the file
    else {
        printf("Can't open specified file!");
        free(src);
        exit(1);
    }

    int ii= (int) ((H-(kernelSize-1))*W)/(16-(kernelSize-1)); // Set the counter

    switch(kernelSize){
        case(3):
            assembly3(W, ii, src, dst);
            break;
        case(5):
            assembly5(W, ii, src, dst);
            break;
        case(7):
            assembly7(W, ii, src, dst);
            break;
    }

    char *prefix = (char *) malloc((strlen(filename)-4+1)*sizeof(char));
    for(int i=0; i<(strlen(filename)-4+1); i++){
            *(prefix+i)=*(filename+i);
            if(i==(strlen(filename)-4)) *(prefix+i)='\0';
    }

    char *suffix = "out_SIMD.raw";
    char *destFilename = (char *) malloc((strlen(prefix)+13)*sizeof(char));
    strcpy(destFilename,prefix);
    strcat(destFilename,suffix);

    FILE *fp2 = fopen(destFilename,"wb");
    if (fp2 != NULL) { // read only if file opened
        fwrite(dst, sizeof(signed char), W*H, fp2);
        fclose(fp2);} // we close the file
    else {
        printf("Can�t open specified file!");
        free(dst);
        exit(1);
    }

    free(src); // to avoid memory leaks
    free(dst);
    free(destFilename);
    end = clock();
    float time = (float)1e3*(end - start)/CLOCKS_PER_SEC; //time in milliseconds
    printf("Time spent with SIMD code : %f\n", time);

    return 0;
}

void assembly3(int W, int ii, unsigned char* src, unsigned char *dst){
__asm__(
    "mov %[in], %%ebx\n"        // data in ptr of the line
    "mov %[l], %%ecx\n"         // counter
    "mov %[out], %%edx\n"       // data out pointer
    "mov %[width], %%eax\n"

    "test %%ecx, %%ecx\n"
    "je end3\n"
    "3:\n"
    "movdqu (%%ebx), %%xmm0\n"  // copy three lines in simd registers
    "movdqu (%%ebx, %%eax), %%xmm1\n"
    "shl $1, %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm2\n"
    "shr $1, %%eax\n"
    "pmaxub %%xmm0, %%xmm1\n"   // keep biggest value from each column
    "pmaxub %%xmm1, %%xmm2\n"

    "movdqu %%xmm2, %%xmm4\n"   // shift and keep biggest value per column
    "movdqu %%xmm2, %%xmm5\n"
    "psrldq $1, %%xmm4\n"
    "psrldq $2, %%xmm5\n"
    "pmaxub %%xmm2, %%xmm4\n"
    "pmaxub %%xmm4, %%xmm5\n"

    "movdqu (%%ebx), %%xmm0\n"  // copy three lines in simd registers
    "movdqu (%%ebx, %%eax), %%xmm1\n"
    "shl $1, %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm2\n"
    "shr $1, %%eax\n"
    "pminub %%xmm0, %%xmm1\n"   // keep smallest value from each column
    "pminub %%xmm1, %%xmm2\n"

    "movdqu %%xmm2, %%xmm6\n"   // shift and keep smallest value per column
    "movdqu %%xmm2, %%xmm7\n"
    "psrldq $1, %%xmm6\n"
    "psrldq $2, %%xmm7\n"
    "pminub %%xmm2, %%xmm6\n"
    "pmaxub %%xmm6, %%xmm7\n"

    "psubd %%xmm7, %%xmm5\n"    // subtract smallest value to greatest

    "movdqu %%xmm5, (%%edx)\n"

    "add $14, %%ebx\n"
    "add $14, %%edx\n"
    "sub $1, %%ecx\n"
    "jnz 3b\n"
    "end3:\n"
    : // No outputs
    : [in] "m" (src), [out] "m" (dst), [width] "m" (W), [l] "r" (ii) // Use "r" for loop counter to allow register usage
    : "eax", "ebx", "edx", "ecx", "memory", "xmm0", "xmm1", "xmm2", "xmm6", "xmm7", "xmm3", "xmm4", "xmm5" // Added "memory" to clobbers
);
}

void assembly5(int W, int ii, unsigned char* src, unsigned char *dst){
__asm__(
    "mov %[in], %%ebx\n"        // data in ptr of the line
    "mov %[l], %%ecx\n"         // counter
    "mov %[out], %%edx\n"       // data out pointer
    "mov %[width], %%eax\n"

    "test %%ecx, %%ecx\n"
    "je end5\n"
    "5:\n"
    "movdqu (%%ebx), %%xmm0\n"  // copy five lines in simd registers
    "movdqu (%%ebx, %%eax), %%xmm1\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm2\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm3\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm4\n"
    "mov %[width], %%eax\n"
    "pmaxub %%xmm0, %%xmm1\n"   // keep biggest value from each column
    "pmaxub %%xmm1, %%xmm2\n"
    "pmaxub %%xmm2, %%xmm3\n"
    "pmaxub %%xmm3, %%xmm4\n"

    "movdqu %%xmm4, %%xmm0\n"   // shift and keep biggest value per column
    "movdqu %%xmm4, %%xmm1\n"
    "movdqu %%xmm4, %%xmm2\n"
    "movdqu %%xmm4, %%xmm3\n"
    "psrldq $1, %%xmm1\n"
    "psrldq $2, %%xmm2\n"
    "psrldq $3, %%xmm3\n"
    "psrldq $4, %%xmm4\n"
    "pmaxub %%xmm4, %%xmm3\n"
    "pmaxub %%xmm3, %%xmm2\n"
    "pmaxub %%xmm2, %%xmm1\n"
    "pmaxub %%xmm1, %%xmm0\n"

    "movdqu (%%ebx), %%xmm1\n"  // copy five lines in simd registers
    "movdqu (%%ebx, %%eax), %%xmm2\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm3\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm4\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm5\n"
    "mov %[width], %%eax\n"
    "pminub %%xmm1, %%xmm2\n"   // keep smallest value from each column
    "pminub %%xmm2, %%xmm3\n"
    "pminub %%xmm3, %%xmm4\n"
    "pminub %%xmm4, %%xmm5\n"

    "movdqu %%xmm5, %%xmm1\n"   // shift and keep smallest value per column
    "movdqu %%xmm5, %%xmm2\n"
    "movdqu %%xmm5, %%xmm3\n"
    "movdqu %%xmm5, %%xmm4\n"
    "psrldq $1, %%xmm1\n"
    "psrldq $2, %%xmm2\n"
    "psrldq $3, %%xmm3\n"
    "psrldq $4, %%xmm4\n"
    "pminub %%xmm5, %%xmm4\n"
    "pminub %%xmm4, %%xmm3\n"
    "pminub %%xmm3, %%xmm2\n"
    "pminub %%xmm2, %%xmm1\n"

    "psubd %%xmm1, %%xmm0\n" // subtract smallest value to greatest

    "movdqu %%xmm0, (%%edx)\n"

    "add $12, %%ebx\n"
    "add $12, %%edx\n"
    "sub $1, %%ecx\n"
    "jnz 5b\n"
    "end5:\n"
    : // No outputs
    : [in] "m" (src), [out] "m" (dst), [width] "m" (W), [l] "r" (ii) // Use "r" for loop counter to allow register usage
    : "eax", "ebx", "edx", "ecx", "memory", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5","xmm6", "xmm7","ymm0","ymm1" // Added "memory" to clobbers
);
}

void assembly7(int W, int ii, unsigned char* src, unsigned char *dst){
__asm__(
    "mov %[in], %%ebx\n"        // data in ptr of the line
    "mov %[l], %%ecx\n"         // counter
    "mov %[out], %%edx\n"       // data out pointer
    "mov %[width], %%eax\n"

    "test %%ecx, %%ecx\n"
    "je end7\n"
    "7:\n"
    "movdqu (%%ebx), %%xmm0\n"  // copy seven lines in simd registers
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm1\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm2\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm3\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm4\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm5\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm6\n"
    "mov %[width], %%eax\n"
    "pmaxub %%xmm0, %%xmm1\n"   // keep biggest value from each column
    "pmaxub %%xmm1, %%xmm2\n"
    "pmaxub %%xmm2, %%xmm3\n"
    "pmaxub %%xmm3, %%xmm4\n"
    "pmaxub %%xmm4, %%xmm5\n"
    "pmaxub %%xmm5, %%xmm6\n"


    "movdqu %%xmm6, %%xmm0\n"   // shift and keep biggest value per column
    "movdqu %%xmm6, %%xmm1\n"
    "movdqu %%xmm6, %%xmm2\n"
    "movdqu %%xmm6, %%xmm3\n"
    "movdqu %%xmm6, %%xmm4\n"
    "movdqu %%xmm6, %%xmm5\n"
    "psrldq $1, %%xmm1\n"
    "psrldq $2, %%xmm2\n"
    "psrldq $3, %%xmm3\n"
    "psrldq $4, %%xmm4\n"
    "psrldq $5, %%xmm5\n"
    "psrldq $6, %%xmm6\n"
    "pmaxub %%xmm6, %%xmm5\n"
    "pmaxub %%xmm5, %%xmm4\n"
    "pmaxub %%xmm4, %%xmm3\n"
    "pmaxub %%xmm3, %%xmm2\n"
    "pmaxub %%xmm2, %%xmm1\n"
    "pmaxub %%xmm1, %%xmm0\n"

    "movdqu (%%ebx), %%xmm1\n"  // copy seven lines in simd registers
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm2\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm3\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm4\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm5\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm6\n"
    "add %[width], %%eax\n"
    "movdqu (%%ebx, %%eax), %%xmm7\n"
    "mov %[width], %%eax\n"
    "pminub %%xmm1, %%xmm2\n"   // keep smallest value from each column
    "pminub %%xmm2, %%xmm3\n"
    "pminub %%xmm3, %%xmm4\n"
    "pminub %%xmm4, %%xmm5\n"
    "pminub %%xmm5, %%xmm6\n"
    "pminub %%xmm6, %%xmm7\n"

    "movdqu %%xmm7, %%xmm1\n"   // shift and keep smallest value per column
    "movdqu %%xmm7, %%xmm2\n"
    "movdqu %%xmm7, %%xmm3\n"
    "movdqu %%xmm7, %%xmm4\n"
    "movdqu %%xmm7, %%xmm5\n"
    "movdqu %%xmm7, %%xmm6\n"
    "psrldq $1, %%xmm1\n"
    "psrldq $2, %%xmm2\n"
    "psrldq $3, %%xmm3\n"
    "psrldq $4, %%xmm4\n"
    "psrldq $5, %%xmm5\n"
    "psrldq $6, %%xmm6\n"
    "pminub %%xmm7, %%xmm6\n"
    "pminub %%xmm6, %%xmm5\n"
    "pminub %%xmm5, %%xmm4\n"
    "pminub %%xmm4, %%xmm3\n"
    "pminub %%xmm3, %%xmm2\n"
    "pminub %%xmm2, %%xmm1\n"

    "psubd %%xmm1, %%xmm0\n"    // subtract smallest value to greatest

    "movdqu %%xmm0, (%%edx)\n"

    "add $10, %%ebx\n"
    "add $10, %%edx\n"
    "sub $1, %%ecx\n"
    "jnz 7b\n"
    "end7:\n"
    : // No outputs
    : [in] "m" (src), [out] "m" (dst), [width] "m" (W), [l] "r" (ii) // Use "r" for loop counter to allow register usage
    : "eax", "ebx", "edx", "ecx", "memory", "xmm0", "xmm1", "xmm2", "xmm6", "xmm7", "xmm3", "xmm4", "xmm5" // Added "memory" to clobbers
);
}
