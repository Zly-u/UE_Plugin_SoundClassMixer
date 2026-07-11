#pragma once

#include "AudioDevice.h"
#include "Sound/SoundNode.h"
#include "SoundNode_DopplerEx.generated.h"

struct FActiveSound;
struct FSoundParseParameters;

/** 
 * Computes doppler pitch shift
 */
UCLASS(HideCategories = Object, EditInlineNew, Meta = (DisplayName = "Doppler Extended"))
class USoundNodeDopplerEx : public USoundNode
{
	GENERATED_UCLASS_BODY()

	/* How much to scale the doppler shift (1.0 is normal). */
	UPROPERTY(EditAnywhere, Category = "Doppler Extended")
	float DopplerIntensity;

	/** Whether or not to do a smooth interp to our doppler */
	UPROPERTY(EditAnywhere, Category = "Doppler Extended")
	bool bUseSmoothing;

	/** Speed at which to interp pitch scale */
	UPROPERTY(EditAnywhere, Category = "Doppler Extended", meta = (EditCondition = "bUseSmoothing"))
	float SmoothingInterpSpeed;
	
	
	/** Speed past which the pitch will reset. */
	UPROPERTY(EditAnywhere, Category = "Doppler Extended")
	float MaxVelocityLimit = 99999.f;
	
	UPROPERTY(EditAnywhere, Category = "Doppler Extended")
	float MaxPitchChangeLimit = 10.f;

	
	public:
		//~ Begin USoundNode Interface. 
		virtual void ParseNodes( FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances ) override;
		//~ End USoundNode Interface. 

		
	protected:
		float GetDopplerPitchMultiplier(float& CurrentPitchScale, bool bSmooth, FListener const& InListener, const FVector Location, const FVector Velocity, float DeltaTime);
};