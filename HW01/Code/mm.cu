//ONLY MODIFY THIS FILE

#include "mm.h"

#define tx threadIdx.x
#define ty threadIdx.y
#define tz threadIdx.z

#define bx blockIdx.x
#define by blockIdx.y
#define bz blockIdx.z

// you may change the following values
#define TILEX 16
#define TILEY 16

// you may define other parameters here!
// you may define other macros here!
// you may define other functions here!

dim3 getDimGrid(const int m, const int n) {
	dim3 dimGrid(n/TILEX,n/TILEY);
	return dimGrid;
}
dim3 getDimBlock(const int m, const int n) {
	dim3 dimBlock(TILEX,TILEY);
	return dimBlock;
}

//-----------------------------------------------------------------------------
__global__ void kernelFunc(float* ad, float* bd, float* cd, const int m,const int n) {
	// write your GPU kernel function here
	// note that maximum # of threads per block is 1024
	int i = bx*blockDim.x+tx; // TILEX-1 * n/TILEX + n/TILEX - 1 = n - 1
	int j = by*blockDim.y+ty;
	int k;
	cd[n*j+i]=0;
	for(k=0;k<n;k++){
		cd[n*j+i] += ad[n*j+k]*bd[n*k+i];
	}
}
