// ONLY MODIFY THIS FILE

#include "scan2.h"
#include "gpuerrors.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#define tx threadIdx.x
#define ty threadIdx.y
#define tz threadIdx.z

#define bx blockIdx.x
#define by blockIdx.y
#define bz blockIdx.z

__global__ void kernelFunc(float *ad, float *cd, float *ed, const int n)
{
    // This kernel implements scan algorithm in a way that in the end we have scan of each block (1024 cells) 
    __shared__ float Mds[1024];

    int r = bx * 1024 + tx;
    Mds[tx] = ad[r]; // Copy to shared Memory
    // ad[r] = tx + 1;  // For debug purposes.
    __syncthreads();

    // In this part the first stage of the tree in Blelloch algorithm is calculated.
    int j = 2;
    for (; j <= n; j = j * 2)
    {
        if ((r + 1) % j == 0)
            Mds[tx] = Mds[tx] + Mds[tx - j / 2];
        __syncthreads();
    }

    // Replace the last element in each block by 0 and also store it for later. This is done in the last thread of 
    // each block and is dependent on the number of threads.
    float e;
    if (n >= 1024 && tx == 1023)
    {
        e = Mds[tx];
        Mds[tx] = 0;
    }
    else if (n < 1024 && tx == n - 1)
    {
        e = Mds[tx];
        Mds[tx] = 0;
    }
    __syncthreads();

    // In this part the second stage of the tree in Blelloch algorithm is calculated.
    j = n;
    for (; j >= 2; j = j / 2)
    {
        if ((r + 1) % j == 0)
        {
            float t = Mds[tx];
            Mds[tx] = Mds[tx] + Mds[tx - j / 2];
            Mds[tx - j / 2] = t;
        }
        __syncthreads();
    }
    
    // For converting exclusive scan to inclusive scan, we need to have a shift to left and then the last element
    // should be replaced by total sum.
    if (tx >= 1)
        cd[r - 1] = Mds[tx];
    if (n >= 1024 && tx == 1023)
        cd[r] = e;
    else if (n < 1024 && tx == n - 1)
        cd[r] = e;
    __syncthreads();
    
    // The last element of each block shoud be stored as a output in order to add them to the next blocks. 
    // So that the scan algorithm is completed.
    if (tx == 1023)
    {
        ed[bx] = cd[r];
    }
}

__global__ void kernelFunc2(float *ed, float *cd, const int n)
{
    // This kernel is used to add sum of all elements in previous blocks in order to complete scan algrorithm and
    // compensate the effect of having seperate blocks in kernelFunc
    int r = bx * 1024 + tx;

    if (bx > 0)
    {
        cd[r] += 1 * ed[bx - 1];
    }
}

dim3 getDimGrid(const int n)
{
    // We need enough blocks to cover all cells of input vector so if all of them are fit into one block, we need one and if not
    // we need n/1024 due to the maximum possible number of threads in each block.
    if (n < 1024)
    {
        dim3 dimGrid(1, 1);
        return dimGrid;
    }
    else
    {
        dim3 dimGrid(n / 1024, 1);
        return dimGrid;
    }
}
dim3 getDimBlock(const int n)
{
    // If all of them are fit into one block, we need one block with enough threads and if not 
    // 1024 threads are specified due to the maximum possible number of threads in each block.
    if (n < 1024)
    {
        dim3 dimBlock(n, 1);
        return dimBlock;
    }
    else
    {
        dim3 dimBlock(1024, 1);
        return dimBlock;
    }
}

void gpuKernel(float *a, float *c, int n)
{
    // This function is run in the CPU. This is a recursive function that has 2 section:
    // 1. Compute scan of input vector 
    // 2. Compute scan of the last element of each block by the "gpuKernel" recursively
    // 3. Add computed scan of the last elements to blocks.
    // For n >= 2^26 the output is calculated by dividing into two subvector with size n/2 by
    // calculating scan of each of them serially. in order to link the first subvector to second subvector,
    // I added the last element of the calculated scan of first subvector to the first element of the second input subvector.
    // So for all m={1, ..., 20, 21, ..., 29} the output is correct.
    if (n < (1 << 26))
    {
        float *ad;
        float *cd;
        float *ed;
        float *e = (float *)malloc(n / 1024 * sizeof(float));

        // Phase 1
        HANDLE_ERROR(cudaMalloc((void **)&ad, n * sizeof(float)));
        HANDLE_ERROR(cudaMalloc((void **)&cd, n * sizeof(float)));
        HANDLE_ERROR(cudaMalloc((void **)&ed, (n / 1024) * sizeof(float)));

        HANDLE_ERROR(cudaMemcpy(ad, a, n * sizeof(float), cudaMemcpyHostToDevice));

        dim3 dimGrid = getDimGrid(n);  
        dim3 dimBlock = getDimBlock(n);

        if (n < 1024)
            kernelFunc<<<dimGrid, dimBlock>>>(ad, cd, ed, n);
        else
            kernelFunc<<<dimGrid, dimBlock>>>(ad, cd, ed, 1024);

        HANDLE_ERROR(cudaMemcpy(c, cd, n * sizeof(float), cudaMemcpyDeviceToHost));
        HANDLE_ERROR(cudaMemcpy(e, ed, n / 1024 * sizeof(float), cudaMemcpyDeviceToHost));

        HANDLE_ERROR(cudaFree(ad));
        HANDLE_ERROR(cudaFree(cd));
        HANDLE_ERROR(cudaFree(ed));

        float *e_s;
        if (n > 1024)
        {
            //printf("Hit!\n");
            e_s = (float *)malloc(n / 1024 * sizeof(float));
            gpuKernel(e, e_s, n / 1024);
        }
        else
        {
            e_s = e;
        }

        // Phase 2
        HANDLE_ERROR(cudaMalloc((void **)&ed, n / 1024 * sizeof(float)));
        HANDLE_ERROR(cudaMalloc((void **)&cd, n * sizeof(float)));

        HANDLE_ERROR(cudaMemcpy(ed, e_s, n / 1024 * sizeof(float), cudaMemcpyHostToDevice));
        HANDLE_ERROR(cudaMemcpy(cd, c, n * sizeof(float), cudaMemcpyHostToDevice));

        dimGrid = getDimGrid(n);
        dimBlock = getDimBlock(n);

        kernelFunc2<<<dimGrid, dimBlock>>>(ed, cd, n);

        HANDLE_ERROR(cudaMemcpy(c, cd, n * sizeof(float), cudaMemcpyDeviceToHost));

        HANDLE_ERROR(cudaFree(cd));
        HANDLE_ERROR(cudaFree(ed));
    }
    else
    {
        gpuKernel(a, c, n / 2);
        a[n / 2] += c[n / 2 - 1];
        gpuKernel(a + n / 2, c + n / 2, n / 2);
    }
}
