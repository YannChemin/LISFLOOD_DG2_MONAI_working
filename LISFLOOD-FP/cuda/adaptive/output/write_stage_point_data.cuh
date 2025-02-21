#pragma once

#include "../utilities/cuda_utils.cuh"

#include <cstdio>
#include <cstdlib>

#include "../classes/AssembledSolution.h"
#include "../classes/ScaleCoefficients.h"
#include "../classes/SolverParams.h"
#include "../classes/PlottingParams.h"
#include "../classes/StagePoints.h"
#include "../classes/FinestGrid.h"

#include "../zorder/get_i_index.cuh"
#include "../zorder/get_j_index.cuh"

__host__
void write_stage_point_data
(
    const int&            mesh_dim,
	const SolverParams&   solver_params,
	const PlottingParams& plot_params,
	AssembledSolution     d_plot_assem_sol,
	FinestGrid            p_finest_grid,
	StagePoints           stage_points,
	const real&           current_time,
	const real&           dx_finest,
	const real&           dy_finest,
	const bool&           first_t_step
);