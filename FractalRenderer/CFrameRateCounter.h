
#pragma once

#include <ionCore.h>

class CFrameRateCounter
{

public:

	CFrameRateCounter()
		: FramesSum(0), FPS(0)
	{}

	void Update(f32 const DeltaTime)
	{
		if (! Equals(DeltaTime, 0.f))
			FPS = 1.f / DeltaTime;

		FramesSum += FPS;
		Frames.push_back(FPS);
		if (Frames.size() > 50)
		{
			FramesSum -= Frames.front();
			Frames.pop_front();
		}
	}

	f32 GetCurrent() const
	{
		return FPS;
	}

	f32 GetLastDuration() const
	{
		return 1 / FPS;
	}

	f32 GetAverage() const
	{
		return FramesSum / Frames.size();
	}

protected:

	std::list<f32> Frames;
	f32 FramesSum, FPS;

};
