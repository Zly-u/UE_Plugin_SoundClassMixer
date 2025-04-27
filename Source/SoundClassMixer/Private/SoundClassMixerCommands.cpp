#include "SoundClassMixerCommands.h"

#include "SoundClassMixerSubsystem.h"
#include "Components/AudioComponent.h"


#define LOCTEXT_NAMESPACE "SoundClassMixerModule"

USoundClassMixerSubsystem* FSoundClassMixerCommands::SoundClassMixerSubsystem = nullptr;

void FSoundClassMixerCommands::RegisterCommands(USoundClassMixerSubsystem* InSoundClassMixerSubsystem)
{
	SoundClassMixerSubsystem = InSoundClassMixerSubsystem;
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("SoundClassMixer.FadeTo"),
		TEXT("Smoothly adjusts Volume to the target Volume."),
		FConsoleCommandWithArgsDelegate::CreateLambda(
			[&](const TArray<FString>& Args)
			{
				if (Args.Num() < 3)
				{
					UE_LOG(LogTemp, Display, TEXT("Not enough of args passed, %d/3"), Args.Num());
					return;
				}
				
				const FString& SoundClassName = Args[0];
				const float AdjustVolumeLevel = FCString::Atof(*Args[1]);
				const float AdjustVolumeDuration = FCString::Atof(*Args[2]);
				
				UE_LOG(LogTemp, Display, TEXT("Test: %s, %f, %f"), *SoundClassName, AdjustVolumeLevel, AdjustVolumeDuration);

				const USoundClass* FoundSoundClass = SoundClassMixerSubsystem->FindSoundClassByName(SoundClassName);
				if (!FoundSoundClass)
				{
					UE_LOG(LogTemp, Error, TEXT("Could not find Sound Class with name: %s"), *SoundClassName);
					return;
				}
				const FSoundClassSubSysProperties* FoundSoundClassProps = SoundClassMixerSubsystem->SoundClassMap.Find(FoundSoundClass);

				SoundClassMixerSubsystem->AdjustVolumeInternal(
					FoundSoundClass,
					AdjustVolumeDuration, AdjustVolumeLevel,
					FoundSoundClassProps->Fader.GetVolume() > AdjustVolumeLevel,
					EAudioFaderCurve::Linear 
				);
			}
		),
		ECVF_Default
	);
}

void FSoundClassMixerCommands::UnregisterCommands()
{
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("SoundClassMixer.FadeTo"), false);
}

#undef LOCTEXT_NAMESPACE
