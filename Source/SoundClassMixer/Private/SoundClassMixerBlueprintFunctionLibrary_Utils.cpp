#include "SoundClassMixerBlueprintFunctionLibrary.h"


float USoundClassMixerBlueprintFunctionLibrary::ConvertToLinear(float Value)
{
	return Audio::ConvertToLinear(Value);
}

float USoundClassMixerBlueprintFunctionLibrary::ConvertToDecibels(const float Value)
{
	return Audio::ConvertToDecibels(Value);
}