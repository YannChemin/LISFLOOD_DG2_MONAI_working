#include "reverse_z_order_act_idcs.cuh"

void lis::cuda::acc_nugrid::reverse_z_order_act_idcs
(
	MortonCode*       d_reverse_z_order,
	MortonCode*       d_indices,
	AssembledSolution d_buf_sol,
	AssembledSolution d_sol,
	int               array_length
)
{
	void*  d_temp_storage = NULL;
	size_t temp_storage   = 0;

	// this launch only decides how much temp_storage is needed for allocation to d_temp_storage
	// use the large possible data type, here NUMERIC_TYPE in d_sol.z, to allocate a largest d_temp_storage
	// that will be able to accommodate all further sorts
	CHECK_CUDA_ERROR( cub::DeviceRadixSort::SortPairs
	(
		d_temp_storage,
		temp_storage,
		d_reverse_z_order,
		d_indices,
		d_buf_sol.act_idcs,
		d_sol.act_idcs,
		array_length
	) );

	d_temp_storage = malloc_device(temp_storage);

	CHECK_CUDA_ERROR( cub::DeviceRadixSort::SortPairs
	(
		d_temp_storage,
		temp_storage,
		d_reverse_z_order,
		d_indices,
		d_buf_sol.act_idcs,
		d_sol.act_idcs,
		array_length
	) );

	CHECK_CUDA_ERROR( cub::DeviceRadixSort::SortPairs
	(
		d_temp_storage,
		temp_storage,
		d_reverse_z_order,
		d_indices,
		d_buf_sol.levels,
		d_sol.levels,
		array_length
	) );

	CHECK_CUDA_ERROR( free_device(d_temp_storage) );
}