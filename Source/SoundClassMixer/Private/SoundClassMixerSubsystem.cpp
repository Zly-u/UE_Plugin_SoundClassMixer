#include "SoundClassMixerSubsystem.h"

#include "ActiveSound.h"
#include "SoundClassMixerCommands.h"
#include "SoundClassMixerSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Engine.h"
#include "Sound/SoundSubmix.h"

// =====================================================================================================================

void USoundClassMixerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	check(!bInitialized);
	
	Super::Initialize(Collection);

#if WITH_EDITOR
	FSoundClassMixerCommands::RegisterCommands(this);
#endif
	
	GatherSoundClasses();
	
	bInitialized = true;
}

void USoundClassMixerSubsystem::Deinitialize()
{
	check(bInitialized);

#if WITH_EDITOR
	FSoundClassMixerCommands::UnregisterCommands();
#endif
	
	bInitialized = false;
	
	Super::Deinitialize();
}

// =====================================================================================================================

ETickableTickType USoundClassMixerSubsystem::GetTickableTickType() const
{
	// By default (if the child class doesn't override GetTickableTickType), don't let CDOs ever tick: 
	return IsTemplate() ? ETickableTickType::Never : FTickableGameObject::GetTickableTickType();
}

bool USoundClassMixerSubsystem::IsAllowedToTick() const
{
	// No matter what IsTickable says, don't let CDOs or uninitialized world subsystems tick :
	// Note: even if GetTickableTickType was overridden by the child class and returns something else than ETickableTickType::Never for CDOs, 
	//  it's probably a mistake, so by default, don't allow ticking. If the child class really intends its CDO to tick, he can always override IsAllowedToTick...
	return !IsTemplate() && bInitialized;
}

void USoundClassMixerSubsystem::Tick(float DeltaTime)
{
	UpdateAudioClasses();
}

// =====================================================================================================================

void USoundClassMixerSubsystem::GatherSoundClasses()
{
	const USoundClassMixerSettings* Settings = GetDefault<USoundClassMixerSettings>();
	
	SoundClassMap.Empty();
	
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	
	FARFilter SoundClassFilter;
	SoundClassFilter.ClassNames.Add(USoundClass::StaticClass()->GetFName());
	SoundClassFilter.bRecursiveClasses = true;

	TArray<FAssetData> AssetDataList_SoundClasses;
	AssetRegistry.GetAssets(SoundClassFilter, AssetDataList_SoundClasses);
	for (const FAssetData& AssetData : AssetDataList_SoundClasses)
	{
		if (Settings->ExcludedSoundClassNames.Find(AssetData.AssetName.ToString()) != INDEX_NONE)
		{
			UE_LOG(LogSoundClassMixerSubsystem, Verbose, TEXT("Excluded SoundClass by Name: %s"), *AssetData.AssetName.ToString());

			continue;
		}
		
		USoundClass* SoundClass = Cast<USoundClass>(AssetData.GetAsset());
		if (!SoundClass)
		{
			continue;
		}

		bool bFoundClassPath = false;
		for (const TSoftObjectPtr<USoundClass>& Subclass : Settings->ExcludedSoundClasses)
		{
			if (Subclass.ToSoftObjectPath() == AssetData.ToSoftObjectPath())
			{
				bFoundClassPath = true;
				break;
			}
		}
		if (bFoundClassPath)
		{
			UE_LOG(LogSoundClassMixerSubsystem, Verbose, TEXT("Excluded SoundClass by Class: %s"), *SoundClass->GetName());
			continue;
		}

		UE_LOG(LogSoundClassMixerSubsystem, Verbose, TEXT("Added SoundClass: %s"), *SoundClass->GetName());
		SoundClassMap.Add(SoundClass);
	}

	
	FARFilter SoundSubmixFilter;
	SoundSubmixFilter.ClassNames.Add(USoundSubmix::StaticClass()->GetFName());
	SoundSubmixFilter.bRecursiveClasses = true;
	
	TArray<FAssetData> AssetDataList_SoundSubmixes;
	AssetRegistry.GetAssets(SoundSubmixFilter, AssetDataList_SoundSubmixes);
	for (const FAssetData& AssetData : AssetDataList_SoundSubmixes)
	{
		USoundSubmix* SoundSubmix = Cast<USoundSubmix>(AssetData.GetAsset());
		if (!SoundSubmix)
		{
			continue;
		}

		UE_LOG(LogSoundClassMixerSubsystem, Verbose, TEXT("Added SoundSubmix: %s"), *SoundSubmix->GetName());
		SoundSubmixMap.Add(SoundSubmix);
	}
}

// =====================================================================================================================

void USoundClassMixerSubsystem::AdjustSoundClassVolumeInternal(
	const USoundClass* SoundClassAsset,
	float AdjustVolumeDuration, float AdjustVolumeLevel,
	const bool bInIsFadeOut, const EAudioFaderCurve FadeCurve
)
{
	if (!SoundClassAsset)
	{
		UE_LOG(LogSoundClassMixerSubsystem, Error, TEXT("Passed Sound Class is invalid."))
		return;
	}
	
	AdjustVolumeDuration = FMath::Max(0.0f, AdjustVolumeDuration);
	AdjustVolumeLevel = FMath::Max(0.0f, AdjustVolumeLevel);
	if (bInIsFadeOut && FMath::IsNearlyZero(AdjustVolumeDuration) && FMath::IsNearlyZero(AdjustVolumeLevel))
	{
		return;
	}

	FSoundSubSysProperties* FoundSoundClassProps = SoundClassMap.Find(SoundClassAsset);
	check(FoundSoundClassProps);

	FoundSoundClassProps->bIsFading = bInIsFadeOut || FMath::IsNearlyZero(AdjustVolumeLevel);

	DECLARE_CYCLE_STAT(TEXT("USoundClassMixerSubsystem.SoundClass.AdjustVolume"), STAT_SoundClassAdjustVolume, STATGROUP_AudioThreadCommands);
	FAudioThread::RunCommandOnAudioThread(
		[FadeCurve, bInIsFadeOut, FoundSoundClassProps, AdjustVolumeDuration, AdjustVolumeLevel]
		{
			Audio::FVolumeFader& AudioClassFader = FoundSoundClassProps->Fader;

			const float InitialTargetVolume = AudioClassFader.GetTargetVolume();

			// Ignore fade out request if requested volume is higher than current target.
			if (bInIsFadeOut && AdjustVolumeLevel >= InitialTargetVolume)
			{
				return;
			}

			const bool ToZeroVolume = FMath::IsNearlyZero(AdjustVolumeLevel);
			if (bInIsFadeOut || ToZeroVolume)
			{
				// If negative, active indefinitely, so always make sure set to minimum positive value for active fade.
				const float OldActiveDuration = AudioClassFader.GetActiveDuration();
				const float NewActiveDuration = OldActiveDuration < 0.0f
					? AdjustVolumeDuration
					: FMath::Min(OldActiveDuration, AdjustVolumeDuration);
				
				AudioClassFader.SetActiveDuration(NewActiveDuration);
			}

			AudioClassFader.StartFade(AdjustVolumeLevel, AdjustVolumeDuration, static_cast<Audio::EFaderCurve>(FadeCurve));
		},
		GET_STATID(STAT_SoundClassAdjustVolume)
	);
}

USoundClass* USoundClassMixerSubsystem::FindSoundClassByName(const FString& SoundClassName)
{
	for (auto It = SoundClassMap.CreateIterator(); It; ++It)
	{
		if (It->Key->GetName() == SoundClassName)
		{
			return It->Key;
		}
	}

	return nullptr;
}

// =====================================================================================================================

void USoundClassMixerSubsystem::AdjustSoundSubmixVolumeInternal(
	const USoundSubmix* SoundSubmixAsset,
	float AdjustVolumeDuration, float AdjustVolumeLevel,
	const bool bInIsFadeOut, const EAudioFaderCurve FadeCurve
)
{
	if (!SoundSubmixAsset)
	{
		UE_LOG(LogSoundClassMixerSubsystem, Error, TEXT("Passed Sound Submix is invalid."))
		return;
	}
	
	AdjustVolumeDuration = FMath::Max(0.0f, AdjustVolumeDuration);
	AdjustVolumeLevel = FMath::Max(0.0f, AdjustVolumeLevel);
	if (bInIsFadeOut && FMath::IsNearlyZero(AdjustVolumeDuration) && FMath::IsNearlyZero(AdjustVolumeLevel))
	{
		return;
	}

	FSoundSubSysProperties* FoundSoundSubmixProps = SoundSubmixMap.Find(SoundSubmixAsset);
	check(FoundSoundSubmixProps);

	FoundSoundSubmixProps->bIsFading = bInIsFadeOut || FMath::IsNearlyZero(AdjustVolumeLevel);

	DECLARE_CYCLE_STAT(TEXT("USoundClassMixerSubsystem.SoundSubmix.AdjustVolume"), STAT_SoundClassAdjustVolume, STATGROUP_AudioThreadCommands);
	FAudioThread::RunCommandOnAudioThread(
		[FadeCurve, bInIsFadeOut, FoundSoundSubmixProps, AdjustVolumeDuration, AdjustVolumeLevel]
		{
			Audio::FVolumeFader& AudioClassFader = FoundSoundSubmixProps->Fader;

			const float InitialTargetVolume = AudioClassFader.GetTargetVolume();

			// Ignore fade out request if requested volume is higher than current target.
			if (bInIsFadeOut && AdjustVolumeLevel >= InitialTargetVolume)
			{
				return;
			}

			const bool ToZeroVolume = FMath::IsNearlyZero(AdjustVolumeLevel);
			if (bInIsFadeOut || ToZeroVolume)
			{
				// If negative, active indefinitely, so always make sure set to minimum positive value for active fade.
				const float OldActiveDuration = AudioClassFader.GetActiveDuration();
				const float NewActiveDuration = OldActiveDuration < 0.0f
					? AdjustVolumeDuration
					: FMath::Min(OldActiveDuration, AdjustVolumeDuration);
				
				AudioClassFader.SetActiveDuration(NewActiveDuration);
			}

			AudioClassFader.StartFade(AdjustVolumeLevel, AdjustVolumeDuration, static_cast<Audio::EFaderCurve>(FadeCurve));
		},
		GET_STATID(STAT_SoundClassAdjustVolume)
	);
}

USoundSubmix* USoundClassMixerSubsystem::FindSoundSubmixByName(const FString& SoundSubmixName)
{
	for (auto It = SoundSubmixMap.CreateIterator(); It; ++It)
	{
		if (It->Key->GetName() == SoundSubmixName)
		{
			return It->Key;
		}
	}

	return nullptr;
}

// =====================================================================================================================

void USoundClassMixerSubsystem::UpdateAudioClasses()
{
	if (!IsInAudioThread())
	{
		check(IsInGameThread());
	
		FAudioThread::RunCommandOnAudioThread([Mixer = this]
			{
				Mixer->UpdateAudioClasses();
			}
		);
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	const float DeltaTime = FApp::GetDeltaTime();
	for (auto It = SoundClassMap.CreateIterator(); It; ++It)
	{
		USoundClass* SoundClass = It->Key;
		FSoundSubSysProperties& SoundClassProps = It->Value;

		// Clamp the delta time to a reasonable max delta time.
		SoundClassProps.Fader.Update(FMath::Min(DeltaTime, 0.5f));
		SoundClass->Properties.Volume = SoundClassProps.Fader.GetVolume();
	}
	
	for (auto It = SoundSubmixMap.CreateIterator(); It; ++It)
	{
		USoundSubmix* SoundSubmix = It->Key;
		FSoundSubSysProperties& SoundSubmixProps = It->Value;

		// Clamp the delta time to a reasonable max delta time.
		SoundSubmixProps.Fader.Update(FMath::Min(DeltaTime, 0.5f));
		SoundSubmix->SetSubmixOutputVolume(World, SoundSubmixProps.Fader.GetVolume());
	}
}

// =====================================================================================================================

TStatId USoundClassMixerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USoundClassMixerSubsystem, STATGROUP_Tickables);
}