#pragma once

class USoundClassMixerBlueprintFunctionLibrary;
class USoundClassMixerSubsystem;

class FSoundClassMixerCommands
{
	friend USoundClassMixerBlueprintFunctionLibrary;
	
public:
	static void RegisterCommands(USoundClassMixerSubsystem* InSoundClassMixerSubsystem);
	static void UnregisterCommands();

private:
	static USoundClassMixerSubsystem* SoundClassMixerSubsystem; 
};