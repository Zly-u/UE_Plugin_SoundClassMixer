#include "SoundClassMixerCommands.h"

#include "CanvasItem.h"
#include "CanvasTableItem.h"
#include "SoundClassMixerSubsystem.h"
#include "Components/AudioComponent.h"
#include "Engine/Canvas.h"

#define LOCTEXT_NAMESPACE "SoundClassMixerModule"

USoundClassMixerSubsystem* FSoundClassMixerCommands::SoundClassMixerSubsystem = nullptr;
bool FSoundClassMixerCommands::bDrawDebug = false;

TSharedPtr<FAutoConsoleCommand> FSoundClassMixerCommands::Command_FadeTo;

FDelegateHandle FSoundClassMixerCommands::DebugDrawDelegateHandle;
TSharedPtr<FAutoConsoleCommand> FSoundClassMixerCommands::Command_ToggleDebug;



void FSoundClassMixerCommands::RegisterCommands(USoundClassMixerSubsystem* InSoundClassMixerSubsystem)
{
	SoundClassMixerSubsystem = InSoundClassMixerSubsystem;

	Command_ToggleDebug = MakeShareable(new FAutoConsoleCommand(
		TEXT("SoundClassMixer.ToggleDebugDraw"),
		TEXT("Toggles drawing debug info from SoundClassMixerSubsystem."),
		FConsoleCommandDelegate::CreateStatic(&FSoundClassMixerCommands::ToggleDebugDraw)
	));
	
	Command_FadeTo = MakeShareable(new FAutoConsoleCommand(
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
	));
}

void FSoundClassMixerCommands::UnregisterCommands()
{
	UDebugDrawService::Unregister(DebugDrawDelegateHandle);
	
	Command_FadeTo.Reset();
	Command_ToggleDebug.Reset();
}

void FSoundClassMixerCommands::ToggleDebugDraw()
{
	bDrawDebug = !bDrawDebug;
	if (bDrawDebug)
	{
		DebugDrawDelegateHandle = UDebugDrawService::Register(
			TEXT("Game"),
			FDebugDrawDelegate::CreateStatic(&FSoundClassMixerCommands::OnDrawDebug)
		);
	}
	else
	{
		UDebugDrawService::Unregister(DebugDrawDelegateHandle);
	}
}

void FSoundClassMixerCommands::OnDrawDebug(UCanvas* Canvas, APlayerController* PC)
{
	TArray<USoundClass*> Keys;
	SoundClassMixerSubsystem->SoundClassMap.GenerateKeyArray(Keys);
	Keys.Sort([](const USoundClass& A, const USoundClass& B)
	{
		return A.GetName() < B.GetName();
	});

	const FVector2D Origin(50, 50);
	FCanvasTableItem Table(Origin);
	Table.SetBorderThickness(2.f);
	Table.SetPadding(3.f);
	for (const USoundClass* Key : Keys)
	{
		const FSoundClassSubSysProperties* Props = SoundClassMixerSubsystem->SoundClassMap.Find(Key);
		Table.AddElement("Current Volume", Key->GetName(), FString::Printf(TEXT("%.4f"), Props->Fader.GetVolume()), FLinearColor::White);
		Table.AddElement("Target Volume", Key->GetName(), FString::Printf(TEXT("%.4f"), Props->Fader.GetTargetVolume()), FLinearColor::White);
	}
	
	Canvas->DrawItem(Table);
}

#undef LOCTEXT_NAMESPACE
