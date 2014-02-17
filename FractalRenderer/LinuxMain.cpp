
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ionCore/ionTypes.h>
#include <mpi.h>

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

char * GetArgument(char ** begin, char ** end, std::string const & option)
{
	char ** itr = std::find(begin, end, option);
	if (itr != end && ++itr != end)
	{
		return *itr;
	}
	return 0;
}

void GetStringArgument(char ** begin, char ** end, std::string const & option, std::string * argument)
{
	char * string;
	if (string = GetArgument(begin, end, option))
		*argument = string;
}

void GetIntArgument(char ** begin, char ** end, std::string const & option, int * argument)
{
	char * string;
	if (string = GetArgument(begin, end, option))
		*argument = atoi(string);
}

void GetUintArgument(char ** begin, char ** end, std::string const & option, uint * argument)
{
	char * string;
	if (string = GetArgument(begin, end, option))
		*argument = atoi(string);
}

void GetDoubleArgument(char ** begin, char ** end, std::string const & option, double * argument)
{
	char * string;
	if (string = GetArgument(begin, end, option))
		*argument = atof(string);
}

bool IfArgumentExists(char ** begin, char ** end, std::string const & option)
{
	return std::find(begin, end, option) != end;
}

class Main
{

public:

	void Run(int argc, char ** argv)
	{
		Init(argc, argv);
		SetupBuffer();
		for (uint i = ProcessorId; i < FrameCount; i += ProcessorCount)
			DrawFrame(i);
		Cleanup();
	}

protected:

	void DrawFrame(uint Frame)
	{
		static f64 const ZoomSpeed = 0.995;
		static f64 const RotateSpeed = 0.001;

		printf("Rendering frame %d of %d (index %d) on system %d\n", Frame+1, FrameCount, Frame, ProcessorId);
		for (int j = LastFrameRendered; j < Frame; ++ j)
		{
			Renderer.Params.Scale.X *= ZoomSpeed;
			Renderer.Params.Scale.Y *= ZoomSpeed;
		}
		Renderer.Params.SetRotation(RotateSpeed * Frame);
		Renderer.Reset();
		DoRender(Frame);

		LastFrameRendered = Frame;
	}

	void Init(int argc, char ** argv)
	{
		LastFrameRendered = 0;

		ScreenSizeX = 1600, ScreenSizeY = 900;
		MultiSample = 4;
		OutputDirectory = ".";
		FrameCount = 40;

		MPI_Init(& argc, & argv);

		MPI_Comm_size(MPI_COMM_WORLD, & ProcessorCount);
		MPI_Comm_rank(MPI_COMM_WORLD, & ProcessorId);

		if (! ProcessorId)
			printf("There are %d processors.\n", ProcessorCount);

		GetUintArgument(argv, argv+argc, "-w", & ScreenSizeX);
		GetUintArgument(argv, argv+argc, "-h", & ScreenSizeY);
		GetUintArgument(argv, argv+argc, "-m", & MultiSample);
		GetStringArgument(argv, argv+argc, "-d", & OutputDirectory);
		GetUintArgument(argv, argv+argc, "-c", & FrameCount);
		GetDoubleArgument(argv, argv+argc, "-x", & Renderer.Params.Center.X);
		GetDoubleArgument(argv, argv+argc, "-y", & Renderer.Params.Center.Y);
		GetDoubleArgument(argv, argv+argc, "-z", & Renderer.Params.Scale.Y);
		GetUintArgument(argv, argv+argc, "-l", & Renderer.Params.IterationMax);
		GetUintArgument(argv, argv+argc, "-i", & IterationIncrement);
		Renderer.Params.Scale.X = Renderer.Params.Scale.Y;

		if (! ProcessorId)
		{
			printf("Doing %dx%d render with %d MS\n", ScreenSizeX, ScreenSizeY, MultiSample);
			printf("%d frames.\n", FrameCount);
			printf("Writing images to '%s/'\n", OutputDirectory.c_str());
			printf("Rendering at %.15f, %.15f\n", Renderer.Params.Center.X, Renderer.Params.Center.Y);
			printf("Rendering scale is %.7f\n", Renderer.Params.Scale.Y);
			printf("Iteration incrememnt is %d\n", IterationIncrement);
			printf("\n");
		}

		MPI_Barrier(MPI_COMM_WORLD);

		Renderer.Params.Stride = 3;
		Renderer.Params.MultiSample = MultiSample;
		Renderer.Init(cvec2u(ScreenSizeX, ScreenSizeY));
		Renderer.IterationIncrement = IterationIncrement;

		BufferSize = ScreenSizeX * ScreenSizeY * sizeof(u8) * 3;
	}

	void SetupBuffer()
	{
		CheckedCudaCall(cudaMalloc((void**) & DeviceBuffer, BufferSize));
		CheckedCudaCall(cudaMemset(DeviceBuffer, 0, BufferSize));
	}

	void DoRender(int const Frame)
	{
		while (! Renderer.Done())
		{
			//printf("Doing render at %d\n", Renderer.GetIterationMax());
			Renderer.Render(DeviceBuffer);
		}

		u8 * Copy = new u8[BufferSize];
		CheckedCudaCall(cudaMemcpy(Copy, DeviceBuffer, BufferSize, cudaMemcpyDeviceToHost), "MemCpy");

		std::stringstream FileName;
		FileName << OutputDirectory + "/Image";
		FileName << std::setw(5) << std::setfill('0') << Frame;
		FileName << ".png";

		FlipImage(Copy, ScreenSizeX, ScreenSizeY);
		stbi_write_png(FileName.str().c_str(), ScreenSizeX, ScreenSizeY, 3, Copy, ScreenSizeX * 3);
		delete [] Copy;
	}

	void Cleanup()
	{
		CheckedCudaCall(cudaFree(DeviceBuffer), "Free");

		printf("Finished on system %d\n", ProcessorId);
		MPI_Finalize();
	}


	CudaFractalRenderer Renderer;

	void * DeviceBuffer;
	u32 BufferSize;

	u32 ScreenSizeX, ScreenSizeY;
	u32 MultiSample;
	u32 FrameCount;
	u32 IterationIncrement;
	std::string OutputDirectory;

	u32 LastFrameRendered;

	int ProcessorId, ProcessorCount;

};

int main(int argc, char * argv[])
{
	Main m;
	m.Run(argc, argv);
	return 0;
}
