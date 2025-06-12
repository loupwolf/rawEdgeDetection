#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int notsimd(char* filename, int width, int height, int kernelSize);
int issimd(char* filename, int width, int height, int kernelSize);

unsigned char* contourExtraction(char* src,int width, int height, int kernel);
