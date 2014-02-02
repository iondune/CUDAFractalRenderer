
#include "CMainState.h"


void CMainState::OnEvent(SKeyboardEvent & Event)
{
	if (! Event.Pressed)
	{
		static double const MoveSpeed = 0.07;
		switch (Event.Key)
		{

		case EKey::Escape:

			Application->Close();
			break;

		case EKey::W:

			FractalRenderer.Params.Center.Y += FractalRenderer.Params.Scale.Y * MoveSpeed;
			PrintLocation();
			FractalRenderer.Reset();
			break;

		case EKey::A:

			FractalRenderer.Params.Center.X -= FractalRenderer.Params.Scale.X * MoveSpeed;
			PrintLocation();
			FractalRenderer.Reset();
			break;

		case EKey::S:

			FractalRenderer.Params.Center.Y -= FractalRenderer.Params.Scale.Y * MoveSpeed;
			PrintLocation();
			FractalRenderer.Reset();
			break;

		case EKey::D:

			FractalRenderer.Params.Center.X += FractalRenderer.Params.Scale.X * MoveSpeed;
			PrintLocation();
			FractalRenderer.Reset();
			break;

		case EKey::BackSlash:

			DumpFrames = true;
			FractalRenderer.Reset();
			break;

		case EKey::Num1:
		case EKey::Num2:
		case EKey::Num3:
		case EKey::Num4:

			Multisample = (int) Event.Key - (int) EKey::Num1 + 1;
			std::cout << "Multisample: " << Multisample << std::endl;
			FractalRenderer.Params.MultiSample = Clamp(Multisample, 1, 4);
			FractalRenderer.FullReset();
			break;

		case EKey::Z:

			FractalRenderer.Params.Scale.X *= 0.5;
			FractalRenderer.Params.Scale.Y *= 0.5;
			FractalRenderer.Reset();
			break;

		case EKey::X:

			FractalRenderer.Params.Scale.X *= 2.0;
			FractalRenderer.Params.Scale.Y *= 2.0;
			FractalRenderer.Reset();
			break;

		case EKey::Q:

			FractalRenderer.Params.Scale.X *= 0.75;
			FractalRenderer.Params.Scale.Y *= 0.75;
			FractalRenderer.Reset();
			break;

		case EKey::E:

			FractalRenderer.Params.Scale.X *= 1.33;
			FractalRenderer.Params.Scale.Y *= 1.33;
			FractalRenderer.Reset();
			break;

		case EKey::R:

			FractalRenderer.Reset();
			break;
				
		case EKey::G:
				
			if (FractalRenderer.Params.IterationMax)
				FractalRenderer.Params.IterationMax *= 2;
			else
				++ FractalRenderer.Params.IterationMax;
				
			printf("iteration cap: %d\n", FractalRenderer.Params.IterationMax);
			FractalRenderer.SoftReset();
				
			break;

		case EKey::B:
				
			FractalRenderer.Params.IterationMax /= 2;
				
			printf("iteration cap: %d\n", FractalRenderer.Params.IterationMax);
			FractalRenderer.Reset();
				
			break;
				
		case EKey::RightBracket:
				
			if (FractalRenderer.IterationIncrement)
				FractalRenderer.IterationIncrement *= 2;
			else
				++ FractalRenderer.IterationIncrement;
				
			printf("IterationIncrement: %d\n", FractalRenderer.IterationIncrement);
				
			break;

		case EKey::LeftBracket:
				
			FractalRenderer.IterationIncrement /= 2;
			printf("IterationIncrement: %d\n", FractalRenderer.IterationIncrement);
				
			break;

		case EKey::U:

			++ CurrentColor;
			if (CurrentColor >= (int) ColorMaps.size())
				CurrentColor = 0;

			break;

		case EKey::J:

			-- CurrentColor;
			if (CurrentColor < 0)
				CurrentColor = ColorMaps.size() - 1;

			break;
				
		}
	}
}

void CMainState::OnEvent(SMouseEvent & Event)
{
	switch (Event.Type)
	{

	case SMouseEvent::EType::Click:

		break;

	case SMouseEvent::EType::Move:

		break;

	}
}
