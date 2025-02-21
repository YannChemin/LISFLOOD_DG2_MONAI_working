#pragma once

#include "cuda_runtime.h"

#include "../classes/Detail.h"
#include "../classes/Details.h"
#include "../types/HierarchyIndex.h"

__device__ __forceinline__ DetailHW load_details_hw
(
	const Details&  d_details,
	const HierarchyIndex& h_idx
)
{
	return
	{
		{
			d_details.eta0.alpha[h_idx],
			d_details.eta0.beta[h_idx],
			d_details.eta0.gamma[h_idx]
		},
		{
			d_details.qx0.alpha[h_idx],
			d_details.qx0.beta[h_idx],
			d_details.qx0.gamma[h_idx]
		},
		{
			d_details.qy0.alpha[h_idx],
			d_details.qy0.beta[h_idx],
			d_details.qy0.gamma[h_idx]
		},
		{
			d_details.z0.alpha[h_idx],
			d_details.z0.beta[h_idx],
			d_details.z0.gamma[h_idx]
		}
	};
}

__device__ __forceinline__ DetailMW load_details_mw
(
	const Details&  d_details,
	const HierarchyIndex& h_idx
)
{
	return
	{
		{
			{
				d_details.eta0.alpha[h_idx],
				d_details.eta0.beta[h_idx],
				d_details.eta0.gamma[h_idx]
			},
			{
				d_details.eta1x.alpha[h_idx],
				d_details.eta1x.beta[h_idx],
				d_details.eta1x.gamma[h_idx]
			},
			{
				d_details.eta1y.alpha[h_idx],
				d_details.eta1y.beta[h_idx],
				d_details.eta1y.gamma[h_idx]
			}
		},
		{
			{
				d_details.qx0.alpha[h_idx],
				d_details.qx0.beta[h_idx],
				d_details.qx0.gamma[h_idx]
			},
			{
				d_details.qx1x.alpha[h_idx],
				d_details.qx1x.beta[h_idx],
				d_details.qx1x.gamma[h_idx]
			},
			{
				d_details.qx1y.alpha[h_idx],
				d_details.qx1y.beta[h_idx],
				d_details.qx1y.gamma[h_idx]
			}
		},
		{
			{
				d_details.qy0.alpha[h_idx],
				d_details.qy0.beta[h_idx],
				d_details.qy0.gamma[h_idx]
			},
			{
				d_details.qy1x.alpha[h_idx],
				d_details.qy1x.beta[h_idx],
				d_details.qy1x.gamma[h_idx]
			},
			{
				d_details.qy1y.alpha[h_idx],
				d_details.qy1y.beta[h_idx],
				d_details.qy1y.gamma[h_idx]
			}
		},
		{
			{
				d_details.z0.alpha[h_idx],
				d_details.z0.beta[h_idx],
				d_details.z0.gamma[h_idx]
			},
			{
				d_details.z1x.alpha[h_idx],
				d_details.z1x.beta[h_idx],
				d_details.z1x.gamma[h_idx]
			},
			{
				d_details.z1y.alpha[h_idx],
				d_details.z1y.beta[h_idx],
				d_details.z1y.gamma[h_idx]
			}
		}
	};
}