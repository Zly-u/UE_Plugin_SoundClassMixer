#include "SoundClassMixerCommands.h"

#include "CanvasItem.h"
#include "CanvasTableItem.h"
#include "Editor.h"
#include "SoundClassMixerSubsystem.h"
#include "Components/AudioComponent.h"
#include "Debug/DebugDrawService.h"
#include "Engine/Canvas.h"
#include "Sound/SoundSubmix.h"


#define LOCTEXT_NAMESPACE "SoundClassMixerModule"


USoundClassMixerSubsystem* FSoundClassMixerCommands::SoundClassMixerSubsystem = nullptr;

bool FSoundClassMixerCommands::bDrawDebug_SoundClass = false;
FDelegateHandle FSoundClassMixerCommands::DebugDrawDelegateHandle_SoundClass;
TSharedPtr<FAutoConsoleCommand> FSoundClassMixerCommands::Command_SoundClass_ToggleDebug;
TSharedPtr<FAutoConsoleCommand> FSoundClassMixerCommands::Command_SoundClass_FadeTo;
TSharedPtr<FAutoConsoleCommand> FSoundClassMixerCommands::Command_SoundClass_SetVolume;

bool FSoundClassMixerCommands::bDrawDebug_SoundSubmix = false;
FDelegateHandle FSoundClassMixerCommands::DebugDrawDelegateHandle_SoundSubmix;
TSharedPtr<FAutoConsoleCommand> FSoundClassMixerCommands::Command_SoundSubmix_ToggleDebug;
TSharedPtr<FAutoConsoleCommand> FSoundClassMixerCommands::Command_SoundSubmix_FadeTo;
TSharedPtr<FAutoConsoleCommand> FSoundClassMixerCommands::Command_SoundSubmix_SetVolume;


void FSoundClassMixerCommands::RegisterCommands(USoundClassMixerSubsystem* InSoundClassMixerSubsystem)
{
	SoundClassMixerSubsystem = InSoundClassMixerSubsystem;

	//------------------------------------------------------------------------------------

#if WITH_EDITOR
	FEditorDelegates::PostPIEStarted.AddStatic(&FSoundClassMixerCommands::FixupDebug);
#endif
	
	//------------------------------------------------------------------------------------
	
	Command_SoundClass_ToggleDebug = MakeShareable(new FAutoConsoleCommand(
		TEXT("SoundClassMixer.SoundClass.ToggleDebugDraw"),
		TEXT("Toggles drawing debug info from SoundClassMixerSubsystem for SoundClass."),
		FConsoleCommandDelegate::CreateStatic(&FSoundClassMixerCommands::ToggleDebugDraw_SoundClass)
	));
	
	Command_SoundClass_FadeTo = MakeShareable(new FAutoConsoleCommand(
		TEXT("SoundClassMixer.SoundClass.FadeTo"),
		TEXT("<FString Name> <float TargetVolume> <float Duration> Smoothly adjusts Volume to the target Volume of a SoundClass."),
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
				const FSoundSubSysProperties* FoundSoundClassProps = SoundClassMixerSubsystem->SoundClassMap.Find(FoundSoundClass);

				SoundClassMixerSubsystem->AdjustSoundClassVolumeInternal(
					FoundSoundClass,
					AdjustVolumeDuration, AdjustVolumeLevel,
					FoundSoundClassProps->Fader.GetVolume() > AdjustVolumeLevel,
					EAudioFaderCurve::Linear 
				);
			}
		),
		ECVF_Default
	));

	Command_SoundClass_SetVolume = MakeShareable(new FAutoConsoleCommand(
		TEXT("SoundClassMixer.SoundClass.SetVolume"),
		TEXT("<FString Name> <float Volume> Set sound of a specific SoundClass."),
		FConsoleCommandWithArgsDelegate::CreateLambda(
			[&](const TArray<FString>& Args)
			{
				if (Args.Num() < 2)
				{
					UE_LOG(LogTemp, Display, TEXT("Not enough of args passed, %d/2"), Args.Num());
					return;
				}

				const FString& SoundClassName = Args[0];
				const float NewVolumeLevel = FCString::Atof(*Args[1]);
				
				USoundClass* FoundSoundClass = SoundClassMixerSubsystem->FindSoundClassByName(SoundClassName);
				if (!FoundSoundClass)
				{
					UE_LOG(LogTemp, Error, TEXT("Could not find Sound Class with name: %s"), *SoundClassName);
					return;
				}

				const FSoundSubSysProperties* FoundSoundClassProps = SoundClassMixerSubsystem->SoundClassMap.Find(FoundSoundClass);
				checkf(FoundSoundClassProps, TEXT("SoundClass Properties are not found."))
				
				SoundClassMixerSubsystem->SetSoundClassVolumeInternal(
					FoundSoundClass,
					NewVolumeLevel
				);
			}
		),
		ECVF_Default
	));

	
	//------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------

				
	Command_SoundSubmix_ToggleDebug = MakeShareable(new FAutoConsoleCommand(
		TEXT("SoundClassMixer.SoundSubmix.ToggleDebugDraw"),
		TEXT("Toggles drawing debug info from SoundClassMixerSubsystem for SoundSubmixes."),
		FConsoleCommandDelegate::CreateStatic(&FSoundClassMixerCommands::ToggleDebugDraw_SoundSubmix)
	));

	Command_SoundSubmix_SetVolume = MakeShareable(new FAutoConsoleCommand(
			TEXT("SoundClassMixer.SoundSubmix.SetVolume"),
			TEXT("<FString Name> <float Volume> Set sound of a specific SoundSubmix."),
			FConsoleCommandWithArgsDelegate::CreateLambda(
				[&](const TArray<FString>& Args)
				{
					if (Args.Num() < 2)
					{
						UE_LOG(LogTemp, Display, TEXT("Not enough of args passed, %d/2"), Args.Num());
						return;
					}

					const FString& SoundSubmixName = Args[0];
					const float NewVolumeLevel = FCString::Atof(*Args[1]);
				
					USoundSubmix* FoundSubmixClass = SoundClassMixerSubsystem->FindSoundSubmixByName(SoundSubmixName);
					if (!FoundSubmixClass)
					{
						UE_LOG(LogTemp, Error, TEXT("Could not find Sound Submix with name: %s"), *SoundSubmixName);
						return;
					}

					const FSoundSubSysProperties* FoundSoundClassProps = SoundClassMixerSubsystem->SoundSubmixMap.Find(FoundSubmixClass);
					checkf(FoundSoundClassProps, TEXT("SoundSubmix Properties are not found."))
				
					SoundClassMixerSubsystem->SetSoundSubmixVolumeInternal(
						FoundSubmixClass,
						NewVolumeLevel
					);
				}
			),
			ECVF_Default
		));
	
	Command_SoundSubmix_FadeTo = MakeShareable(new FAutoConsoleCommand(
		TEXT("SoundClassMixer.SoundSubmix.FadeTo"),
		TEXT("<FString Name> <float TargetVolume> <float Duration> Smoothly adjusts Volume to the target Volume of a SoundSubmix."),
		FConsoleCommandWithArgsDelegate::CreateLambda(
			[&](const TArray<FString>& Args)
			{
				if (Args.Num() < 3)
				{
					UE_LOG(LogTemp, Display, TEXT("Not enough of args passed, %d/3"), Args.Num());
					return;
				}
				
				const FString& SoundSubmixName = Args[0];
				const float AdjustVolumeLevel = FCString::Atof(*Args[1]);
				const float AdjustVolumeDuration = FCString::Atof(*Args[2]);
				
				UE_LOG(LogTemp, Display, TEXT("Test: %s, %f, %f"), *SoundSubmixName, AdjustVolumeLevel, AdjustVolumeDuration);
	
				const USoundSubmix* FoundSoundSubmix = SoundClassMixerSubsystem->FindSoundSubmixByName(SoundSubmixName);
				if (!FoundSoundSubmix)
				{
					UE_LOG(LogTemp, Error, TEXT("Could not find Sound Class with name: %s"), *SoundSubmixName);
					return;
				}
				
				const FSoundSubSysProperties* FoundSoundClassProps = SoundClassMixerSubsystem->SoundSubmixMap.Find(FoundSoundSubmix);
	
				SoundClassMixerSubsystem->AdjustSoundSubmixVolumeInternal(
					FoundSoundSubmix,
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
	UDebugDrawService::Unregister(DebugDrawDelegateHandle_SoundClass);
	UDebugDrawService::Unregister(DebugDrawDelegateHandle_SoundSubmix);
	
	Command_SoundClass_FadeTo.Reset();
	Command_SoundClass_ToggleDebug.Reset();
	
	Command_SoundSubmix_FadeTo.Reset();
	Command_SoundSubmix_ToggleDebug.Reset();
}

// =========================================================================================================

void FSoundClassMixerCommands::ToggleDebugDraw_SoundClass()
{
	bDrawDebug_SoundClass = !bDrawDebug_SoundClass;
	if (bDrawDebug_SoundClass)
	{
		DebugDrawDelegateHandle_SoundClass = UDebugDrawService::Register(
			TEXT("Game"),
			FDebugDrawDelegate::CreateStatic(&FSoundClassMixerCommands::OnDrawDebug_SoundClass)
		);
	}
	else
	{
		UDebugDrawService::Unregister(DebugDrawDelegateHandle_SoundClass);
	}
}

void FSoundClassMixerCommands::OnDrawDebug_SoundClass(UCanvas* Canvas, APlayerController* PC)
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
		const FSoundSubSysProperties* Props = SoundClassMixerSubsystem->SoundClassMap.Find(Key);
		Table.AddElement("Current Volume", Key->GetName(), FString::Printf(TEXT("%.4f"), Props->Fader.GetVolume()), FLinearColor::White);
		Table.AddElement("Target Volume", Key->GetName(), FString::Printf(TEXT("%.4f"), Props->Fader.GetTargetVolume()), FLinearColor::White);
	}
	
	Canvas->DrawItem(Table);
}

// =========================================================================================================

void FSoundClassMixerCommands::ToggleDebugDraw_SoundSubmix()
{
	bDrawDebug_SoundSubmix = !bDrawDebug_SoundSubmix;
	if (bDrawDebug_SoundSubmix)
	{
		DebugDrawDelegateHandle_SoundSubmix = UDebugDrawService::Register(
			TEXT("Game"),
			FDebugDrawDelegate::CreateStatic(&FSoundClassMixerCommands::OnDrawDebug_SoundSubmix)
		);
	}
	else
	{
		UDebugDrawService::Unregister(DebugDrawDelegateHandle_SoundSubmix);
	}
}

void FSoundClassMixerCommands::OnDrawDebug_SoundSubmix(UCanvas* Canvas, APlayerController* PC)
{
	TArray<USoundSubmix*> Keys;
	SoundClassMixerSubsystem->SoundSubmixMap.GenerateKeyArray(Keys);
	Keys.Sort([](const USoundSubmix& A, const USoundSubmix& B)
	{
		return A.GetName() < B.GetName();
	});

	const FVector2D Origin(50, 50);
	FCanvasTableItem Table(Origin);
	Table.SetBorderThickness(2.f);
	Table.SetPadding(3.f);
	for (const USoundSubmix* Key : Keys)
	{
		const FSoundSubSysProperties* Props = SoundClassMixerSubsystem->SoundSubmixMap.Find(Key);
		Table.AddElement("Current Volume", Key->GetName(), FString::Printf(TEXT("%.4f"), Props->Fader.GetVolume()), FLinearColor::White);
		Table.AddElement("Target Volume", Key->GetName(), FString::Printf(TEXT("%.4f"), Props->Fader.GetTargetVolume()), FLinearColor::White);
	}
	
	Canvas->DrawItem(Table);
}

// =========================================================================================================

void FSoundClassMixerCommands::FixupDebug(bool bIsSimulating)
{
	bDrawDebug_SoundClass = !bDrawDebug_SoundClass;
	bDrawDebug_SoundSubmix = !bDrawDebug_SoundSubmix;

	UDebugDrawService::Unregister(DebugDrawDelegateHandle_SoundClass);
	UDebugDrawService::Unregister(DebugDrawDelegateHandle_SoundSubmix);
	
	ToggleDebugDraw_SoundSubmix();
	ToggleDebugDraw_SoundClass();
}


#undef LOCTEXT_NAMESPACE
