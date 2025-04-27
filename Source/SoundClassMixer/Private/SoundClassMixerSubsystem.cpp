#include "SoundClassMixerSubsystem.h"

#include "ActiveSound.h"
#include "SoundClassMixerCommands.h"
#include "SoundClassMixerSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Engine.h"

class USoundClassMixerSettings;

void USoundClassMixerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	check(!bInitialized);
	
	Super::Initialize(Collection);
	
	FSoundClassMixerCommands::RegisterCommands(this);
	
#if WITH_EDITOR
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	
	AssetRegistryModule.Get().OnAssetAdded().AddUObject(this, &USoundClassMixerSubsystem::OnSoundClassAssetAdded);
	AssetRegistryModule.Get().OnAssetRemoved().AddUObject(this, &USoundClassMixerSubsystem::OnSoundClassAssetRemoved);
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
	SoundClassMap.Empty();
	
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	
	FARFilter Filter;
	Filter.ClassNames.Add(USoundClass::StaticClass()->GetFName());
	Filter.bRecursiveClasses = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);


	const USoundClassMixerSettings* Settings = GetDefault<USoundClassMixerSettings>();
	
	for (const FAssetData& AssetData : AssetDataList)
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
}

void USoundClassMixerSubsystem::AdjustVolumeInternal(
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

	FSoundClassSubSysProperties* FoundSoundClassProps = SoundClassMap.Find(SoundClassAsset);
	check(FoundSoundClassProps);

	FoundSoundClassProps->bIsFading = bInIsFadeOut || FMath::IsNearlyZero(AdjustVolumeLevel);

	DECLARE_CYCLE_STAT(TEXT("USoundClassMixerSubsystem.AdjustVolume"), STAT_SoundClassAdjustVolume, STATGROUP_AudioThreadCommands);
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

	const float DeltaTime = FApp::GetDeltaTime();
	for (auto It = SoundClassMap.CreateIterator(); It; ++It)
	{
		USoundClass* SoundClass = It->Key;
		FSoundClassSubSysProperties& SoundClassProps = It->Value;

		// Clamp the delta time to a reasonable max delta time.
		SoundClassProps.Fader.Update(FMath::Min(DeltaTime, 0.5f));
		SoundClass->Properties.Volume = SoundClassProps.Fader.GetVolume();
	}
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

#if WITH_EDITOR

void USoundClassMixerSubsystem::OnSoundClassAssetAdded(const FAssetData& AssetData)
{
	UE_LOG(LogSoundClassMixerSubsystem, Display, TEXT("[OnSoundClassAssetAdded] Checking Class: %s"), *USoundClass::StaticClass()->GetFName().ToString())
	
	if (AssetData.AssetClass != USoundClass::StaticClass()->GetFName())
	{
		return;
	}

	USoundClass* NewSoundClass = Cast<USoundClass>(AssetData.GetAsset());
	if (!NewSoundClass)
	{
		UE_LOG(LogSoundClassMixerSubsystem, Display, TEXT("Failed to add a SoundClass: %s"), *AssetData.AssetClass.ToString())
		return;
	}

	UE_LOG(LogSoundClassMixerSubsystem, Display, TEXT("Sound Class added: %s"), *NewSoundClass->GetName())
	SoundClassMap.Add(NewSoundClass);
}

void USoundClassMixerSubsystem::OnSoundClassAssetRemoved(const FAssetData& AssetData)
{
	UE_LOG(LogSoundClassMixerSubsystem, Display, TEXT("[OnSoundClassAssetRemoved] Checking Class: %s"), *USoundClass::StaticClass()->GetFName().ToString())
	
	if (AssetData.AssetClass != USoundClass::StaticClass()->GetFName())
	{
		return;
	}

	const USoundClass* SoundClassToRemove = Cast<USoundClass>(AssetData.GetAsset());
	if (!SoundClassToRemove)
	{
		return;
	}

	UE_LOG(LogSoundClassMixerSubsystem, Display, TEXT("Sound Class Removed: %s"), *SoundClassToRemove->GetName())
	SoundClassMap.Remove(SoundClassToRemove);
}

#endif

// =====================================================================================================================

TStatId USoundClassMixerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USoundClassMixerSubsystem, STATGROUP_Tickables);
}