
#include "CudaFractalRender.cuh"


u8 const * CudaRenderFractal(u32 const Width, u32 const Height)
{
	u8 * Image = new u8[Width * Height * 3];

	for (u32 x = 0; x < Width; ++ x)
	for (u32 y = 0; y < Height; ++ y)
	{
		Image[y * Width * 3 + x * 3 + 0] = 0;
		Image[y * Width * 3 + x * 3 + 1] = (u8) (x / (f32) Width * 255.f);
		Image[y * Width * 3 + x * 3 + 2] = (u8) (x / (f32) Width * 255.f);
	}
	return Image;
}
