# LISFLOODv82_MWDG2_CUDA
LISFLOOD v8.2 using Multi Scaling with Haar wavelet (DG2)
https://zenodo.org/records/13121102

LISFLOOD-FP8.0 includes second-order discontinuous Galerkin (DG2) and first-order finite volume (FV1) solvers of the 2D shallow water equations for modelling a wide range of flows, including rapidly-propagating, supercritical flows, shock waves, or flows over very smooth surfaces. The DG2/FV1 solvers are parallelised for the multi-core CPU architecture, but do not integrate with the subgrid channel model nor with the CPU-specific optimisations.

To decrease the computational costs, the ACC, FV1 and DG2 solvers have been accelerated using Graphics Processing Units (GPUs) (Shaw et al., 2021; Sharifian et al., 2023). The suitable applications for each of these solvers of are summarised in the table below, along with their potential limitations.


DG2 
Fluvial flooding; Dam-breaks; tsunamis; Flows around hydraulic structures	
High computational cost for large-scale applications; Restrictive time-step for applications that involve thin flows


# Dataset
https://zenodo.org/records/8075133
GPU-parallelisation of Haar wavelet-based grid resolution adaptation for fast finite volume modelling: application to shallow water flows.

article: https://iwaponline.com/jh/article/25/4/1210/95732/GPU-parallelisation-of-Haar-wavelet-based-grid

MONAI dataset is used here

# Tested on:

Debian Bookworm fresh install

CUDA drivers install from here: https://docs.nvidia.com/datacenter/tesla/driver-installation-guide/index.html

and here: https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html

with NVIDIA Quadro P1000 on CUDA 12.6

# Compile

cmake -S . -B build 

cmake --build build

