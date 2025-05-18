#pragma once

class USoundClassMixerBlueprintFunctionLibrary;
class USoundClassMixerSubsystem;
class FAutoConsoleCommand;

class FSoundClassMixerCommands
{
	friend USoundClassMixerBlueprintFunctionLibrary;
	
public:
	static void RegisterCommands(USoundClassMixerSubsystem* InSoundClassMixerSubsystem);
	static void UnregisterCommands();

	static void ToggleDebugDraw_SoundClass();
	static void OnDrawDebug_SoundClass(UCanvas* Canvas, APlayerController* PC);
	static bool bDrawDebug_SoundClass;

	static void ToggleDebugDraw_SoundSubmix();
	static void OnDrawDebug_SoundSubmix(UCanvas* Canvas, APlayerController* PC);
	static bool bDrawDebug_SoundSubmix;

	static void FixupDebug(bool bIsSimulating);
	
private:
	static USoundClassMixerSubsystem* SoundClassMixerSubsystem;
	
	static FDelegateHandle DebugDrawDelegateHandle_SoundClass;
	static TSharedPtr<FAutoConsoleCommand> Command_SoundClass_ToggleDebug;
	static TSharedPtr<FAutoConsoleCommand> Command_SoundClass_FadeTo;
	static TSharedPtr<FAutoConsoleCommand> Command_SoundClass_SetVolume;

	static FDelegateHandle DebugDrawDelegateHandle_SoundSubmix;
	static TSharedPtr<FAutoConsoleCommand> Command_SoundSubmix_ToggleDebug;
	static TSharedPtr<FAutoConsoleCommand> Command_SoundSubmix_FadeTo;
	static TSharedPtr<FAutoConsoleCommand> Command_SoundSubmix_SetVolume;
};