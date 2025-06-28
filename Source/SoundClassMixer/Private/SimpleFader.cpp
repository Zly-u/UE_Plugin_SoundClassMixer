#include "SimpleFader.h"

FSimpleFader::FSimpleFader()
	: CurrentVolume(1.0f)
	, TargetVolume(1.0f)
	, FadeDuration(-1.0f)
	, ElapsedTime(0.0f)
	, FadeCurve(Audio::EFaderCurve::Linear)
{}

float FSimpleFader::AlphaToVolume(const float InAlpha, const Audio::EFaderCurve InCurve)
{
	switch (InCurve)
	{
	case Audio::EFaderCurve::Linear:
	{
		return InAlpha;
	}

	case Audio::EFaderCurve::SCurve:
	{
		float Volume = 0.5f * Audio::FastSin(PI * InAlpha - HALF_PI) + 0.5f;
		return FMath::Max(0.0f, Volume);
	}

	case Audio::EFaderCurve::Sin:
	{
		float Volume = Audio::FastSin(HALF_PI * InAlpha);
		return FMath::Max(0.0f, Volume);
	}

	case Audio::EFaderCurve::Logarithmic:
	{
		return Audio::ConvertToLinear(InAlpha);
	}

	default:
	{
		static_assert(static_cast<int32>(Audio::EFaderCurve::Count) == 4, "Possible missing switch case coverage for EAudioFade");
	}
	break;
	}

	return 1.0f;
}


// =====================================================================================================================


float FSimpleFader::GetVolume() const
{
	return AlphaToVolume(CurrentVolume, FadeCurve);
}

float FSimpleFader::GetFadeDuration() const
{
	return FadeDuration;
}

Audio::EFaderCurve FSimpleFader::GetCurve() const
{
	return FadeCurve;
}

float FSimpleFader::GetTargetVolume() const
{
	switch (FadeCurve)
	{
		case Audio::EFaderCurve::Linear:
		case Audio::EFaderCurve::SCurve:
		case Audio::EFaderCurve::Sin:
		{
			return TargetVolume;
		}

		case Audio::EFaderCurve::Logarithmic:
		{
			return Audio::ConvertToLinear(TargetVolume);
		}

		default:
		{
			static_assert(static_cast<int32>(Audio::EFaderCurve::Count) == 4, "Possible missing switch case coverage for EAudioFade");
		}
	}

	return 1.0f;
}


// =====================================================================================================================


bool FSimpleFader::IsFading() const
{
	return ElapsedTime < FadeDuration;
}

bool FSimpleFader::IsFadingIn() const
{
	return IsFading() && TargetVolume > CurrentVolume;
}

bool FSimpleFader::IsFadingOut() const
{
	return IsFading() && TargetVolume < CurrentVolume;
}


// =====================================================================================================================

void FSimpleFader::SetVolume(const float InVolume)
{
	CurrentVolume = InVolume;
	ElapsedTime = 0.0f;
	FadeCurve = Audio::EFaderCurve::Linear;
	FadeDuration = -1.0f;
	TargetVolume = InVolume;
}


// =====================================================================================================================


void FSimpleFader::StartFade(const float InVolume, const float InDuration, const Audio::EFaderCurve InCurve)
{
	if (InDuration <= 0.0f)
	{
		SetVolume(InVolume);
		return;
	}

	if (InCurve != Audio::EFaderCurve::Logarithmic)
	{
		if (FadeCurve == Audio::EFaderCurve::Logarithmic)
		{
			CurrentVolume = Audio::ConvertToLinear(CurrentVolume);
		}
		TargetVolume = InVolume;
	}
	else
	{
		constexpr float DecibelFloor = KINDA_SMALL_NUMBER; // -80dB
		if (FadeCurve != Audio::EFaderCurve::Logarithmic)
		{
			CurrentVolume = Audio::ConvertToDecibels(CurrentVolume, DecibelFloor);
		}
		TargetVolume = Audio::ConvertToDecibels(InVolume, DecibelFloor);
	}

	ElapsedTime = 0.0f;
	FadeCurve = InCurve;
	FadeDuration = InDuration;
}


void FSimpleFader::StopFade()
{
	if (FadeCurve == Audio::EFaderCurve::Logarithmic)
	{
		CurrentVolume = Audio::ConvertToLinear(CurrentVolume);
	}
	TargetVolume = CurrentVolume;
	FadeCurve = Audio::EFaderCurve::Linear;
	ElapsedTime = 0.0;
	FadeDuration = -1.0f;
}


// =====================================================================================================================


float FSimpleFader::GetVolumeAfterTime(float InDeltaTime) const
{
	InDeltaTime = FMath::Max(0.0f, InDeltaTime);

	// Keep stepping towards our target until we hit our stop time & Clamp
	float FutureAlpha = CurrentVolume;
	const float Duration = ElapsedTime + InDeltaTime;

	if (Duration < FadeDuration)
	{
		// Choose min/max bound and clamp dt to prevent unwanted spikes in volume
		float MinValue;
		float MaxValue;
		if (CurrentVolume < TargetVolume)
		{
			MinValue = CurrentVolume;
			MaxValue = TargetVolume;
		}
		else
		{
			MinValue = TargetVolume;
			MaxValue = CurrentVolume;
		}

		FutureAlpha = CurrentVolume + ((TargetVolume - CurrentVolume) * InDeltaTime / (FadeDuration - ElapsedTime));
		FutureAlpha = FMath::Clamp(FutureAlpha, MinValue, MaxValue);
	}

	return AlphaToVolume(FutureAlpha, FadeCurve);
}


void FSimpleFader::Update(float InDeltaTime)
{
	if (!IsFading())
	{
		return;
	}
	
	ElapsedTime += InDeltaTime;
	
	if (ElapsedTime >= FadeDuration)
	{
		CurrentVolume = TargetVolume;
		StopFade();
		return;
	}

	// Keep stepping towards target and clamp until fade duration has expired.
	// Choose min/max bound and clamp dt to prevent unwanted spikes in volume
	float MinValue;
	float MaxValue;
	if (CurrentVolume < TargetVolume)
	{
		MinValue = CurrentVolume;
		MaxValue = TargetVolume;
	}
	else 
	{
		MinValue = TargetVolume;
		MaxValue = CurrentVolume;
	}

	CurrentVolume += (TargetVolume - CurrentVolume) * InDeltaTime / (FadeDuration - ElapsedTime);
	CurrentVolume = FMath::Clamp(CurrentVolume, MinValue, MaxValue);
}
