#include "SoundNode_DopplerEx.h"
#include "ActiveSound.h"
#include "Kismet/KismetMathLibrary.h"


/*-----------------------------------------------------------------------------
         USoundNodeDopplerEx implementation.
-----------------------------------------------------------------------------*/
USoundNodeDopplerEx::USoundNodeDopplerEx(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DopplerIntensity = 1.0f;
	bUseSmoothing = false;
	SmoothingInterpSpeed = 5.0f;
}


void USoundNodeDopplerEx::ParseNodes( FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD(sizeof(float));
	DECLARE_SOUNDNODE_ELEMENT(float, CurrentPitchScale);

	FSoundParseParameters UpdatedParams = ParseParams;

	check(AudioDevice);
	const TArray<FListener>& Listeners = AudioDevice->GetListeners();

	int32 ListenerIndex = ActiveSound.GetClosestListenerIndex();
	if (ListenerIndex >= Listeners.Num())
	{
		ListenerIndex = 0;
	}

	const FListener& Listener = Listeners[ListenerIndex];

	// Default the parse to using the setting for smoothing
	if (*RequiresInitialization)
	{
		*RequiresInitialization = 0;

		// First time, do no smoothing, but initialize the current pitch scale value to the first value returned from this function
		CurrentPitchScale = GetDopplerPitchMultiplier(CurrentPitchScale, false, Listener, ParseParams.Transform.GetTranslation(), ParseParams.Velocity, AudioDevice->GetDeviceDeltaTime());
		UpdatedParams.Pitch *= CurrentPitchScale;
	}
	else
	{
		// Subsequent calls to this will do smoothing from the first initial value
		UpdatedParams.Pitch *= GetDopplerPitchMultiplier(CurrentPitchScale, bUseSmoothing, Listener, ParseParams.Transform.GetTranslation(), ParseParams.Velocity, AudioDevice->GetDeviceDeltaTime());
	}

	Super::ParseNodes(AudioDevice, NodeWaveInstanceHash, ActiveSound, UpdatedParams, WaveInstances);
}


float USoundNodeDopplerEx::GetDopplerPitchMultiplier(float& CurrentPitchScale, bool bSmooth, FListener const& InListener, const FVector Location, const FVector Velocity, float DeltaTime)
{
	static constexpr float SpeedOfSoundInAirAtSeaLevel = 33000.f;		// cm/sec

	FVector DistanceBetweenListenerAndSource = InListener.Transform.GetTranslation() - Location;
	DistanceBetweenListenerAndSource = DistanceBetweenListenerAndSource.GetClampedToSize(0, MaxVelocityLimit);
	
	FVector const SourceToListenerNorm = DistanceBetweenListenerAndSource.GetSafeNormal();

	// find source and listener speeds along the line between them
	float const SourceVelMagTowardListener = Velocity | SourceToListenerNorm;
	float const ListenerVelMagAwayFromSource = InListener.Velocity | SourceToListenerNorm;
	
	float const ListenerToSourceClampedDist = FMath::Clamp(SourceVelMagTowardListener - ListenerVelMagAwayFromSource, -MaxVelocityLimit, MaxVelocityLimit);
	
	float const InvDopplerPitchScale = 1.f - ( ListenerToSourceClampedDist / SpeedOfSoundInAirAtSeaLevel );
	float const PitchScale = 1.f / InvDopplerPitchScale;
	float const FinalPitchScale = FMath::Clamp((PitchScale - 1.f) * DopplerIntensity, -MaxPitchChangeLimit, MaxPitchChangeLimit) + 1.f;

	if (bSmooth)
	{
		CurrentPitchScale = UKismetMathLibrary::FInterpTo(CurrentPitchScale, FinalPitchScale, DeltaTime, SmoothingInterpSpeed);

		return CurrentPitchScale;
	}
	
	return FinalPitchScale;
}
