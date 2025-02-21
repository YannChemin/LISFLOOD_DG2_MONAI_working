#pragma once

#include "cuda_runtime.h"

#include "../types/real.h"

// inspired from https://core.ac.uk/download/pdf/196597598.pdf
__device__ __forceinline__
real h_init_parabolic_bowl
(
	const real& x_or_y_int,
	const real& z_int
)
{
	const real eta = C(0.2) * x_or_y_int + C(10.0);

	return max(C(0.0), eta - z_int);
}