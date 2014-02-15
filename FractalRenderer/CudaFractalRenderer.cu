
#include "CudaFractalRenderer.cuh"
#include "CudaFractalKernels.cuh"


void CudaFractalRenderer::Init(cvec2u const & ScreenSize)
{
	Params.ScreenSize = ScreenSize;
	Params.Scale.X *= Params.ScreenSize.X / (f64) Params.ScreenSize.Y;

	DeviceStates = 0;
	FullReset();
}

CudaFractalRenderer::~CudaFractalRenderer()
{
	cudaFree(DeviceStates);
	cudaFree(DeviceHistogram);
}

void CudaFractalRenderer::FullReset()
{
	u32 const ScreenCount = Params.ScreenSize.X * Params.ScreenSize.Y * Params.MultiSample * Params.MultiSample;
	u32 const StateCount = ScreenCount * sizeof(SPixelState);

	IterationIncrement = 100;

	if (DeviceStates)
		cudaFree(DeviceStates);
	printf("Allocating %d bytes on GPU (%.2f GB) (%d states of size %d).\n", StateCount, StateCount / 1000.0 / 1000.0 / 1000.0, ScreenCount, sizeof(SPixelState));
	CheckedCudaCall(cudaMalloc((void**) & DeviceStates, StateCount), "cudaMalloc");

	DeviceHistogram = 0;
	Reset();
}

void CudaFractalRenderer::Reset()
{
	HistogramSize = (Params.IterationMax + 1) * sizeof(u32);
	u32 const BlockSize = 16;
	u32 const MSWidth = Params.ScreenSize.X * Params.MultiSample;
	u32 const MSHeight = Params.ScreenSize.Y * Params.MultiSample;

	if (DeviceHistogram)
		cudaFree(DeviceHistogram);
	CheckedCudaCall(cudaMalloc((void**) & DeviceHistogram, HistogramSize), "cudaMalloc");
	CheckedCudaCall(cudaMemset(DeviceHistogram, 0, HistogramSize), "cudaMemset");

	dim3 const Grid(
		MSWidth / BlockSize + (MSWidth % BlockSize ? 1 : 0),
		MSHeight / BlockSize + (MSHeight % BlockSize ? 1 : 0));
	dim3 const Block(BlockSize, BlockSize);
	InitKernel<<<Grid, Block>>>(DeviceStates, Params);
	CheckCudaResults("init");

	IterationMax = 10;
}

void CudaFractalRenderer::SoftReset()
{
	u32 const NewHistogramSize = (Params.IterationMax + 1) * sizeof(u32);

	u32 * NewDeviceHistogram;
	cudaMalloc((void**) & NewDeviceHistogram, NewHistogramSize);
	cudaMemset(NewDeviceHistogram, 0, NewHistogramSize);
	cudaMemcpy(NewDeviceHistogram, DeviceHistogram, HistogramSize, cudaMemcpyDeviceToDevice);
	cudaFree(DeviceHistogram);
	DeviceHistogram = NewDeviceHistogram;
	HistogramSize = NewHistogramSize;
}

void CudaFractalRenderer::Render(void * deviceBuffer)
{
	u32 const BlockSize = 16;
	SFractalParams ParamsCopy = Params;

	if (IterationMax < ParamsCopy.IterationMax)
	{
		IterationMax = Min(IterationMax + IterationIncrement, ParamsCopy.IterationMax);

		if (IterationMax <= ParamsCopy.IterationMax)
		{
			ParamsCopy.IterationMax = IterationMax;
			{
				dim3 const Grid(
					ParamsCopy.ScreenSize.X * Params.MultiSample / BlockSize + (ParamsCopy.ScreenSize.X * Params.MultiSample % BlockSize ? 1 : 0),
					ParamsCopy.ScreenSize.Y * Params.MultiSample / BlockSize + (ParamsCopy.ScreenSize.Y * Params.MultiSample % BlockSize ? 1 : 0));
				dim3 const Block(BlockSize, BlockSize);
				HistogramKernel<<<Grid, Block>>>(DeviceStates, DeviceHistogram, ParamsCopy);
				CheckCudaResults("histogram");
				DrawKernel<<<Grid, Block>>>(deviceBuffer, DeviceStates, DeviceHistogram, ParamsCopy);
				CheckCudaResults("draw");
			}
			{
				dim3 const Grid(
					ParamsCopy.ScreenSize.X / BlockSize + (ParamsCopy.ScreenSize.X % BlockSize ? 1 : 0),
					ParamsCopy.ScreenSize.Y / BlockSize + (ParamsCopy.ScreenSize.Y % BlockSize ? 1 : 0));
				dim3 const Block(BlockSize, BlockSize);
				FinalKernel<<<Grid, Block>>>(deviceBuffer, DeviceStates, DeviceHistogram, ParamsCopy);
				CheckCudaResults("final");
			}
		}
	}
}

u32 CudaFractalRenderer::GetIterationMax() const
{
	return IterationMax;
}
