
#include "CMainState.h"

using namespace ion;


void CMainState::OnEvent(IEvent & Event)
{
	if (InstanceOf<SKeyboardEvent>(Event))
	{
		SKeyboardEvent KeyboardEvent = As<SKeyboardEvent>(Event);

		if (! KeyboardEvent.Pressed)
		{
			static double const MoveSpeed = 0.07;
			switch (KeyboardEvent.Key)
			{

			case EKey::Escape:

				Window->Close();
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
				//FractalRenderer.Reset();
				break;

			case EKey::Slash:

				RenderZoom = ! RenderZoom;
				FractalRenderer.Params.SetRotation(LastRotation = 0);
				FractalRenderer.Reset();
				break;

			case EKey::Num1:
			case EKey::Num2:
			case EKey::Num3:
			case EKey::Num4:

				Multisample = (int) KeyboardEvent.Key - (int) EKey::Num1 + 1;
				std::cout << "Multisample: " << Multisample << std::endl;
				FractalRenderer.Params.MultiSample = Clamp(Multisample, 1, 4);
				FractalRenderer.FullReset();
				break;

			case EKey::Z:

				FractalRenderer.Params.Scale.X *= 0.5;
				FractalRenderer.Params.Scale.Y *= 0.5;
				PrintLocation();
				FractalRenderer.Reset();
				break;

			case EKey::X:

				FractalRenderer.Params.Scale.X *= 2.0;
				FractalRenderer.Params.Scale.Y *= 2.0;
				PrintLocation();
				FractalRenderer.Reset();
				break;

			case EKey::C:

				FractalRenderer.Params.SetRotation(FractalRenderer.Params.Angle + 0.1);
				FractalRenderer.Reset();
				break;

			case EKey::V:

				FractalRenderer.Params.SetRotation(FractalRenderer.Params.Angle - 0.1);
				FractalRenderer.Reset();
				break;

			case EKey::Q:

				FractalRenderer.Params.Scale.X *= 0.75;
				FractalRenderer.Params.Scale.Y *= 0.75;
				PrintLocation();
				FractalRenderer.Reset();
				break;

			case EKey::E:

				FractalRenderer.Params.Scale.X *= 1.33;
				FractalRenderer.Params.Scale.Y *= 1.33;
				PrintLocation();
				FractalRenderer.Reset();
				break;

			case EKey::Comma:

				FractalRenderer.Params.Scale.X *= 0.9901;
				FractalRenderer.Params.Scale.Y *= 0.9901;
				PrintLocation();
				FractalRenderer.Reset();
				break;

			case EKey::Period:

				FractalRenderer.Params.Scale.X *= 1.01;
				FractalRenderer.Params.Scale.Y *= 1.01;
				PrintLocation();
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

			case EKey::Grave:

				ShowText = ! ShowText;
				break;

			}
		}
	}
	else if (InstanceOf<SMouseEvent>(Event))
	{
		SMouseEvent & MouseEvent = As<SMouseEvent>(Event);

		switch (MouseEvent.Type)
		{

		case SMouseEvent::EType::Click:

			break;

		case SMouseEvent::EType::Move:

			break;

		}
	}
}
