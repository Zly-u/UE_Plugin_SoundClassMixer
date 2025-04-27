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

	static void OnDrawDebug(UCanvas* Canvas, APlayerController* PC);
	static void ToggleDebugDraw();
	static bool bDrawDebug;
	
	
private:
	static USoundClassMixerSubsystem* SoundClassMixerSubsystem;
	
	static FDelegateHandle DebugDrawDelegateHandle;
	
	static TSharedPtr<FAutoConsoleCommand> Command_ToggleDebug;
	static TSharedPtr<FAutoConsoleCommand> Command_FadeTo;
};