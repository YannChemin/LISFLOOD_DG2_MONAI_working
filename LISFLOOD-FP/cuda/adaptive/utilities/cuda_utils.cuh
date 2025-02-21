#pragma once

#include "cuda_runtime.h"

#include "../utilities/CHECK_CUDA_ERROR.cuh"

#include "cstdio"

__host__
cudaError_t sync(int dummy = 0);

__host__
cudaError_t peek();

__host__
cudaError_t reset();

__host__
cudaError_t copy_cuda
(
	void* dst,
	void* src,
	size_t bytes
);

__host__
cudaError_t copy_cuda_async
(
	void* dst,
	void* src,
	size_t bytes
);

__host__
void* malloc_device
(
	size_t bytes
);

__host__
cudaError_t free_device
(
	void* ptr
);

__host__
void* malloc_pinned
(
	size_t bytes
);

__host__
cudaError_t free_pinned
(
	void* ptr
);
