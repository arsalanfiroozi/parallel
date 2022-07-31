//ONLY MODIFY THIS FILE!
//YOU CAN MODIFY EVERYTHING IN THIS FILE!

#include "bmm.h"

#define tx threadIdx.x
#define ty threadIdx.y
#define tz threadIdx.z

#define bx blockIdx.x
#define by blockIdx.y
#define bz blockIdx.z

// TILEX and TILEY are used to set the number of threads in a CUDA block 
#define TILEX 32
#define TILEY 16
#if TILEX > TILEY
	#define TILEZ	TILEX
#else
	#define TILEZ	TILEY
#endif

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
__global__ void kernelFunc(float* ad, float* bd, float* cd, const int m, const int n) {
	__shared__ float Mds[TILEY][TILEZ];
	__shared__ float Nds[TILEZ][TILEX];
	
	int r = by * TILEY + ty;
	int c = bx * TILEX + tx;
	
	float tmp = 0;
	for(int m=0; m < n/TILEZ; m++){
		for(int i=0; i < TILEZ/TILEX; i++)
			//Mds[ty][m*TILEZ + tx*(TILEZ/TILEX) + i] = ad[r * n + (m*TILEZ + tx*(TILEZ/TILEX) + i)]; 
			Mds[ty][tx*(TILEZ/TILEX) + i] = ad[r * n + (m*TILEZ + tx*(TILEZ/TILEX) + i)]; 
		for(int i=0; i < TILEZ/TILEY; i++)
			//Nds[m*TILEZ + ty*(TILEZ/TILEY) + i][tx] = bd[(m*TILEZ + ty*(TILEZ/TILEY) + i)*n + c];
			Nds[ty*(TILEZ/TILEY) + i][tx] = bd[(m*TILEZ + ty*(TILEZ/TILEY) + i)*n + c];
		__syncthreads();
		for(int k=0; k<TILEZ; k++){
			tmp += Mds[ty][k] * Nds[k][tx];
		}
		__syncthreads();
	}
	cd[r*n+c]=tmp;
}
