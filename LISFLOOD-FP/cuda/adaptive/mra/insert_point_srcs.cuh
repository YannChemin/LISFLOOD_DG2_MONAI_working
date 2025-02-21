#pragma once

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "../classes/AssembledSolution.h"
#include "../classes/PointSources.h"

#include "../utilities/get_lvl_idx.cuh"

__global__
void insert_point_srcs
(
	AssembledSolution d_assem_sol,
	PointSources      point_sources,
	real              dt,
	real              dx_finest
);