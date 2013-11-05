
#pragma once

#include <ionCore/ionTypes.h>
#include "cuda_runtime.h"


#define __universal__ __host__ __device__

template <typename T>
struct CudaVec2
{
	T X, Y;

	__universal__ CudaVec2()
		: X(), Y()
	{}

	__universal__ CudaVec2(T const v)
		: X(v), Y(v)
	{}

	__universal__ CudaVec2(T const x, T const y)
		: X(x), Y(y)
	{}

	__universal__ CudaVec2 operator + (CudaVec2 const & v) const
	{
		CudaVec2 result(*this);
		return result += v;
	}

	__universal__ CudaVec2 operator += (CudaVec2 const & v)
	{
		X += v.X;
		Y += v.Y;

		return * this;
	}

	__universal__ CudaVec2 operator - (CudaVec2 const & v) const
	{
		CudaVec2 result(*this);
		return result -= v;
	}

	__universal__ CudaVec2 operator -= (CudaVec2 const & v)
	{
		X -= v.X;
		Y -= v.Y;

		return * this;
	}

	__universal__ CudaVec2 operator * (CudaVec2 const & v) const
	{
		CudaVec2 result(*this);
		return result *= v;
	}

	__universal__ CudaVec2 operator *= (CudaVec2 const & v)
	{
		X *= v.X;
		Y *= v.Y;

		return * this;
	}

	__universal__ CudaVec2 operator / (CudaVec2 const & v) const
	{
		CudaVec2 result(*this);
		return result /= v;
	}

	__universal__ CudaVec2 operator /= (CudaVec2 const & v)
	{
		X /= v.X;
		Y /= v.Y;

		return * this;
	}

	__universal__ CudaVec2 operator -() const
	{
		CudaVec2 result;
		result.X = -X;
		result.Y = -Y;
		return result;
	}

	__universal__ T Length() const
	{
		return sqrt(X * X + Y * Y);
	}

	__universal__ friend T Dot(CudaVec2 const & v1, CudaVec2 const & v2)
	{
		return v1.X * v2.X + v1.Y * v2.Y;
	}

	__universal__ CudaVec2 Normalize() const
	{
		return * this / length();
	}

	__universal__ friend CudaVec2 Normalize(CudaVec2 const & v)
	{
		return v / v.length();
	}

};

typedef CudaVec2<f32> cvec2f;
typedef CudaVec2<f64> cvec2d;
typedef CudaVec2<s32> cvec2i;
typedef CudaVec2<u32> cvec2u;
