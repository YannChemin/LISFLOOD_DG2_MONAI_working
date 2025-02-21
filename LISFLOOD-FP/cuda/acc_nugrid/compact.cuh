#pragma once

#include "cuda_runtime.h"

#include "MortonCode.h"

namespace lis
{
namespace cuda
{
namespace acc_nugrid
{

// remove the even bits and squash together the odd bits of a Morton code
__host__ __device__ __forceinline__ int compact(MortonCode code)
{
	                                                            //               In binary:
	int coord = code & 0x55555555;         // -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0

	coord = (coord ^ (coord >> 1)) & 0x33333333; // --fe --dc --ba --98 --76 --54 --32 --10
	coord = (coord ^ (coord >> 2)) & 0x0f0f0f0f; // ---- fedc ---- ba98 ---- 7654 ---- 3210
	coord = (coord ^ (coord >> 4)) & 0x00ff00ff; // ---- ---- fedc ba98 ---- ---- 7654 3210
	coord = (coord ^ (coord >> 8)) & 0x0000ffff; // ---- ---- ---- ---- fedc ba98 7654 3210
	
	return coord;
}

}
}
}