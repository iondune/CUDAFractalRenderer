
#pragma once

#include <string>
#include <iostream>
#include <cstdio>

#include <cuda.h>
#include <cuda_runtime.h>
#include <device_functions.h>
#include <device_launch_parameters.h>

#include <ionCore/ionTypes.h>
#include <ionCore/ionUtils.h>


#define __universal__ __host__ __device__

static cudaError_t CheckedCudaCall(cudaError_t const status, std::string const & function = "")
{
	if (status != cudaSuccess)
	{
		if (function != "")
			std::cerr << "CUDA call to " << function;
		else
			std::cerr << "CUDA call";
		std::cerr << " failed with error '" << cudaGetErrorString(status) << "' (" << status << ")" << std::endl;
	}

	return status;
}

static void CheckCudaResults(std::string const & function = "")
{
	cudaThreadSynchronize();

	cudaError_t error = cudaGetLastError();
	if (error != cudaSuccess)
		printf("CUDA error during %s: %s\n", function.c_str(), cudaGetErrorString(error));
}
