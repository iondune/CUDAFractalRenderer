
#include <sstream>
#include <iomanip>
#include <ionCore/ionTypes.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "CudaFractalRenderer.cuh"


void FlipImage(u8 * Image, u32 const X, u32 const Y)
{
	for (u32 j = 0; j < Y / 2; ++ j)
	for (u32 i = 0; i < X; ++ i)
	for (u32 c = 0; c < 3; ++ c)
	{
		std::swap(Image[i * 3 + j * X * 3 + c], Image[i * 3 + (Y - 1 - j) * X * 3 + c]);
	}
}


int main(int argc, char * argv[])
{
	u32 const ScreenSizeX = 1600, ScreenSizeY = 900;
	u32 MultiSample = 4;

	CudaFractalRenderer Renderer;
	Renderer.Params.Stride = 3;
	Renderer.Params.MultiSample = MultiSample;
	Renderer.Init(cvec2u(ScreenSizeX, ScreenSizeY));

	void * DeviceBuffer;
	u32 const BufferSize = ScreenSizeX * ScreenSizeY * sizeof(u8) * 3;
	CheckedCudaCall(cudaMalloc((void**) & DeviceBuffer, BufferSize));
	CheckedCudaCall(cudaMemset(DeviceBuffer, 0, BufferSize));

	while (! Renderer.Done())
	{
		printf("Doing render at %d\n", Renderer.GetIterationMax());
		Renderer.Render(DeviceBuffer);
	}

	u8 * Copy = new u8[BufferSize];
	CheckedCudaCall(cudaMemcpy(Copy, DeviceBuffer, BufferSize, cudaMemcpyDeviceToHost), "MemCpy");
	CheckedCudaCall(cudaFree(DeviceBuffer), "Free");

	std::stringstream FileName;
	FileName << "OutputImages";
	FileName << std::setw(5) << std::setfill('0') << /*CurrentDumpFrame ++*/ 0;
	FileName << ".png";

	FlipImage(Copy, ScreenSizeX, ScreenSizeY);
	stbi_write_png(FileName.str().c_str(), ScreenSizeX, ScreenSizeY, 3, Copy, ScreenSizeX * 3);
	delete [] Copy;

	return 0;
}
