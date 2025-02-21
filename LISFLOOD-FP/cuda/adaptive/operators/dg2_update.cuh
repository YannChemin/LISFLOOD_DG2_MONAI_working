#pragma once

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "cub/block/block_scan.cuh"

#include "../utilities/BLOCK_VAR_MACROS.cuh"
#include "../types/MortonCode.h"
#include "../classes/Neighbours.h"
#include "../classes/FlowCoeffs.h"
#include "../classes/SolverParams.h"
#include "../classes/SimulationParams.h"
#include "../classes/FlowVector.h"

#include "get_leg_basis.cuh"
#include "get_bed_src.cuh"
#include "flux_HLL.cuh"

__global__
void dg2_update
(
    Neighbours        d_neighbours,
    AssembledSolution d_assem_sol_load,
    AssembledSolution d_assem_sol_store,
    SolverParams      solver_params,
    SimulationParams  sim_params,
    real              dx_finest,
    real              dy_finest,
    real              dt,
    int               test_case,
    real*             d_dt_CFL,
    bool              rkdg2
);