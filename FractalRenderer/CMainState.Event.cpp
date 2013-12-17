
#include "CMainState.h"

extern u32 IterationIncrement;

void CMainState::OnEvent(SKeyboardEvent & Event)
{
	if (! Event.Pressed)
	{
		static double const MoveSpeed = 0.07;
		switch (Event.Key)
		{

		case EKey::W:

			cY += (CurrentFractal == EFT_BURNING_SHIP ? -1 : 1) * sY * MoveSpeed;
			PrintLocation();
			Reset();
			break;

		case EKey::A:

			cX -= sX * MoveSpeed;
			PrintLocation();
			Reset();
			break;

		case EKey::S:

			cY -= (CurrentFractal == EFT_BURNING_SHIP ? -1 : 1) * sY * MoveSpeed;
			PrintLocation();
			Reset();
			break;

		case EKey::D:

			cX += sX * MoveSpeed;
			PrintLocation();
			Reset();
			break;

		case EKey::Comma:

			++ CurrentFractal;
			if (CurrentFractal >= EFT_COUNT)
				CurrentFractal = 0;
			std::cout << "Fractal: " << CurrentFractal << std::endl;
			break;

		case EKey::M:

			++ CurrentSettings;
			if (CurrentSettings >= ESS_COUNT)
				CurrentSettings = 0;
			std::cout << "Multisample: " << CurrentSettings << std::endl;
			break;

		case EKey::Num1:
		case EKey::Num2:
		case EKey::Num3:
		case EKey::Num4:
		case EKey::Num5:
		case EKey::Num6:
		case EKey::Num7:
		case EKey::Num8:

			CurrentSettings = (int) Event.Key - (int) EKey::Num1;
			if (CurrentSettings >= ESS_COUNT)
				CurrentSettings = ESS_COUNT - 1;
			std::cout << "Multisample: " << CurrentSettings << std::endl;
			break;

		case EKey::Z:

			sX *= 0.5;
			sY *= 0.5;
			Reset();
			break;

		case EKey::X:

			sX *= 2.0;
			sY *= 2.0;
			Reset();
			break;

		case EKey::Q:

			sX *= 0.75;
			sY *= 0.75;
			Reset();
			break;

		case EKey::E:

			sX *= 1.33;
			sY *= 1.33;
			Reset();
			break;

		case EKey::R:

			Reset();
			break;
				
		case EKey::G:
				
			if (max_iteration)
				max_iteration *= 2;
			else
				++ max_iteration;
				
			printf("iteration cap: %d\n", max_iteration);
			Reset();
				
			break;

		case EKey::B:
				
			max_iteration /= 2;
				
			printf("iteration cap: %d\n", max_iteration);
			Reset();
				
			break;
				
		case EKey::RightBracket:
				
			if (IterationIncrement)
				IterationIncrement *= 2;
			else
				++ IterationIncrement;
				
			printf("IterationIncrement: %d\n", IterationIncrement);
			Reset();
				
			break;

		case EKey::LeftBracket:
				
			IterationIncrement /= 2;
				
			printf("IterationIncrement: %d\n", IterationIncrement);
			Reset();
				
			break;
				
		case EKey::H:
				
			ScaleFactor --;
			RecalcScale();
				
			break;
				
		case EKey::N:
				
			ScaleFactor ++;
			RecalcScale();
				
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

		case EKey::I:

			++ SetColorCounter;

			SetSetColor();

			break;

		case EKey::K:

			-- SetColorCounter;

			SetSetColor();

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
