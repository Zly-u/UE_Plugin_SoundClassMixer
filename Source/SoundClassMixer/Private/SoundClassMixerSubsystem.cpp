#include "SoundClassMixerSubsystem.h"

#include "ActiveSound.h"
#include "SoundClassMixer.h"
#include "SoundClassMixerCommands.h"
#include "SoundClassMixerSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

class USoundClassMixerSettings;

void USoundClassMixerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	check(!bInitialized);
	
	Super::Initialize(Collection);
	
	FSoundClassMixerCommands::RegisterCommands(this);

#if WITH_EDITOR
	ToggleDebugCommand = MakeShareable(new FAutoConsoleCommand(
		TEXT("SoundClassMixer.ToggleDebugDraw"),
		TEXT("Toggles drawing debug info from SoundClassMixerSubsystem."),
		FConsoleCommandDelegate::CreateUObject(this, &USoundClassMixerSubsystem::ToggleDebugDraw)
	));
	
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

	FSoundClassMixerCommands::UnregisterCommands();
	UDebugDrawService::Unregister(DebugDrawDelegateHandle);
	ToggleDebugCommand.Reset();
	
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
		
		// GEngine->AddOnScreenDebugMessage(
		// 	-1, 0.0, FColor::Red,
		// 	FString::Printf(
		// 		TEXT("%s Volume: %f -> %f"),
		// 		*SoundClass->GetName(),
		// 		SoundClassProps.Fader.GetVolume(), SoundClassProps.Fader.GetTargetVolume()
		// 	)
		// );
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

// =====================================================================================================================

void USoundClassMixerSubsystem::ToggleDebugDraw()
{
	bDrawDebug = !bDrawDebug;
	if (bDrawDebug)
	{
		DebugDrawDelegateHandle = UDebugDrawService::Register(
			TEXT("Game"),
			FDebugDrawDelegate::CreateUObject(this, &USoundClassMixerSubsystem::OnDrawDebug)
		);
	}
	else
	{
		UDebugDrawService::Unregister(DebugDrawDelegateHandle);
	}
}

class FCanvasTableItem : public FCanvasItem
{
public:
	FCanvasTableItem(const FVector2D& InPosition)
		: FCanvasItem(InPosition),
		BorderThickness(1.0f),
		CellLinesThickness(1.0f),
		Padding(5.0), SizeMultiplier(1.0)
	{}

	virtual void Draw(FCanvas* InCanvas) override {
		SetupTableCorners();

		// Elements BG
		FCanvasTileItem Bg(
			Position + FVector2D(MaxRowLabelWidth, MaxColumnLabelHeight),
			CalculatedTableSize - FVector2D(MaxRowLabelWidth, MaxColumnLabelHeight),
			WindowBgColor
		);
		Bg.BlendMode = SE_BLEND_Translucent;
		InCanvas->DrawItem(Bg);

		// Columns Headers Bg
		FCanvasTileItem CH_Bg(
			Position + FVector2D(MaxRowLabelWidth, 0),
			FVector2D(CalculatedTableSize.X - MaxRowLabelWidth, MaxColumnLabelHeight),
			ColumnsCellColor
		);
		CH_Bg.BlendMode = SE_BLEND_Translucent;
		InCanvas->DrawItem(CH_Bg);

		// Rows Headers Bg
		FCanvasTileItem CR_Bg(
			Position + FVector2D(0, MaxColumnLabelHeight),
			FVector2D(MaxRowLabelWidth, CalculatedTableSize.Y - MaxColumnLabelHeight),
			RowsCellColor
		);
		CR_Bg.BlendMode = SE_BLEND_Translucent;
		InCanvas->DrawItem(CR_Bg);
		
		FBatchedElements* BatchedElements = InCanvas->GetBatchedElements(FCanvas::ET_Line);
		const FHitProxyId HitProxyId = InCanvas->GetHitProxyId();
		
		// Draw each Column line.
		int32 PrevColumnPos = Position.X + MaxRowLabelWidth; // Skip the first section of Row labels.
		for (int32 ColumnIndex = 0; ColumnIndex < TableColumnLabels.Num(); ColumnIndex++)
		{
			BatchedElements->AddLine(
				FVector(
					PrevColumnPos,
					Position.Y,
					0.0f
				),
				FVector(
					PrevColumnPos,
					Position.Y + CalculatedTableSize.Y,
					0.0f
				),
				CellsLinesColor, HitProxyId, CellLinesThickness
			);

			PrevColumnPos += TableColumnWidth[ColumnIndex];
		}

		// Draw each Row line.
		int32 PrevRowPos = Position.Y + MaxColumnLabelHeight; // Skip the first section of Column labels.
		for (int32 RowIndex = 0; RowIndex < TableRowHeight.Num(); RowIndex++)
		{
			BatchedElements->AddLine(
				FVector(
					Position.X,
					PrevRowPos,
					0.0f
				),
				FVector(
					Position.X + CalculatedTableSize.X,
					PrevRowPos,
					0.0f
				),
				CellsLinesColor, HitProxyId, CellLinesThickness
			);
			
			PrevRowPos += TableRowHeight[RowIndex];
		}
		
		// Draw the box.
		for (int32 EdgeIndex = 0; EdgeIndex < TableCorners.Num(); EdgeIndex++)
		{
			const int32 NextCornerIndex = (EdgeIndex + 1) % TableCorners.Num();
			BatchedElements->AddLine(
				TableCorners[EdgeIndex], TableCorners[NextCornerIndex],
				BorderColor, HitProxyId, BorderThickness
			);
		}
		
		const UFont* RenderFont = GEngine->GetTinyFont();

		{
			// Draw Column Labels.
			int32 PrevColumnLabelPos = Position.X + MaxRowLabelWidth;
			for (int32 ColumnLabelIndex = 0; ColumnLabelIndex < TableColumnLabels.Num(); ColumnLabelIndex++)
			{
				InCanvas->DrawShadowedString(
					PrevColumnLabelPos, Position.Y,
					*TableColumnLabels[ColumnLabelIndex],
					RenderFont,
					ColumnsLabelsColor
				);

				PrevColumnLabelPos += TableColumnWidth[ColumnLabelIndex];
			}
		}

		{
			// Draw Raw Labels.
			int32 PrevRowLabelPos = Position.Y + MaxColumnLabelHeight;
			for (int32 RowLabelIndex = 0; RowLabelIndex < TableRowLabels.Num(); RowLabelIndex++)
			{
				InCanvas->DrawShadowedString(
					Position.X, PrevRowLabelPos,
					*TableRowLabels[RowLabelIndex],
					RenderFont,
					RowsLabelsColor
				);

				PrevRowLabelPos += TableRowHeight[RowLabelIndex];
			}
		}

		{
			int32 PrevRowLabelPos = Position.Y + MaxColumnLabelHeight;
			for (int32 RowLabelIndex = 0; RowLabelIndex < TableRowLabels.Num(); RowLabelIndex++)
			{
				int32 PrevColumnLabelPos = Position.X + MaxRowLabelWidth;
				for (int32 ColumnLabelIndex = 0; ColumnLabelIndex < TableColumnLabels.Num(); ColumnLabelIndex++)
				{
					InCanvas->DrawShadowedString(
						PrevColumnLabelPos, PrevRowLabelPos,
						*TableElements[RowLabelIndex][ColumnLabelIndex],
						RenderFont,
						RowsLabelsColor
					);
					
					PrevColumnLabelPos += TableColumnWidth[ColumnLabelIndex];
				}

				PrevRowLabelPos += TableRowHeight[RowLabelIndex];
			}
		}
	}

	/* Expose the functions defined in the base class. */
	using FCanvasItem::Draw;

	void AddElement(const FString& ColumnName, const FString& RowName, const FString& Value)
	{
		const UFont* RenderFont = GEngine->GetMediumFont();

		MaxColumnLabelHeight = FMath::Max(MaxColumnLabelHeight, RenderFont->GetStringHeightSize(*ColumnName)); 
		MaxRowLabelWidth = FMath::Max(MaxRowLabelWidth, RenderFont->GetStringSize(*RowName));
		
		const int32 ColumnWidth = FMath::Max(
			FMath::Max(
				RenderFont->GetStringSize(*ColumnName),
				RenderFont->GetStringSize(*RowName)
			),
			RenderFont->GetStringSize(*Value)
		);

		const int32 RowHeight = FMath::Max(
			FMath::Max(
				RenderFont->GetStringHeightSize(*ColumnName),
				RenderFont->GetStringHeightSize(*RowName)
			),
			RenderFont->GetStringHeightSize(*Value)
		);
		
		const int32 X_Index = TableColumnLabels.AddUnique(ColumnName);
		const int32 Y_Index = TableRowLabels.AddUnique(RowName);

		TableColumnWidth.SetNumZeroed(X_Index + 1);
		TableRowHeight.SetNumZeroed(Y_Index + 1);
		
		TableColumnWidth[X_Index] = ColumnWidth;
		TableRowHeight[Y_Index] = RowHeight;

		if (TableElements.Num() < Y_Index + 1)
		{
			TableElements.SetNumZeroed(Y_Index + 1);
		}
		
		auto& Row = TableElements[Y_Index];
		if (Row.Num() < X_Index + 1)
		{
			Row.SetNumZeroed(X_Index + 1);
		}
		
		TableElements[Y_Index][X_Index] = Value;
	}

public:
	void SetPadding(const float NewPadding)
	{
		Padding = NewPadding;
	}
	void SetBorderThickness(const float NewThickness)
	{
		BorderThickness = NewThickness;
	}
	void SetCellLinesThickness(const float NewThickness)
	{
		CellLinesThickness = NewThickness;
	}
	void SetSizeMultiplier(const float NewSizeMultiplier)
	{
		SizeMultiplier = NewSizeMultiplier;
	}
	
private:
	void SetupTableCorners()
	{
		CalculatedTableSize = FVector2D(
			MaxRowLabelWidth,
			MaxColumnLabelHeight
		);

		for (const int32 ColumnWidth : TableColumnWidth)
		{
			CalculatedTableSize.X += ColumnWidth;
		}

		for (const int32 RowHeight : TableRowHeight)
		{
			CalculatedTableSize.Y += RowHeight;
		}
		
		TableCorners.Empty();
		// Top
		TableCorners.Add(FVector(Position.X, Position.Y, 0.0f));
		// Right
		TableCorners.Add(FVector(Position.X + CalculatedTableSize.X, Position.Y, 0.0f));
		// Bottom
		TableCorners.Add(FVector(Position.X + CalculatedTableSize.X, Position.Y + CalculatedTableSize.Y, 0.0f));
		// Left
		TableCorners.Add(FVector(Position.X, Position.Y + CalculatedTableSize.Y, 0.0f));
	}
	
public:
	FLinearColor WindowBgColor	 = FLinearColor(0,0,0, 0.4);
	FLinearColor BorderColor	 = FLinearColor::Black;
	FLinearColor CellsLinesColor = FLinearColor(1,1,1, 0.25);

	FLinearColor ColumnsCellColor	= FLinearColor(0, 0.05, 0, 0.4);
	FLinearColor ColumnsLabelsColor = FLinearColor::Green;

	FLinearColor RowsCellColor	 = FLinearColor(0, 0.05, 0, 0.4);
	FLinearColor RowsLabelsColor = FLinearColor::Green;
	
	float BorderThickness;
	float CellLinesThickness;
	float Padding;
	float SizeMultiplier;
	
private:
	FVector2D CalculatedTableSize;
	TArray<FVector> TableCorners;
	
	TArray<FString> TableColumnLabels;
	TArray<int32> TableColumnWidth;
	
	TArray<FString> TableRowLabels;
	TArray<int32> TableRowHeight;
	
	TArray<TArray<FString>> TableElements;

	int32 MaxColumnLabelHeight = 0;
	int32 MaxRowLabelWidth = 0;
};

void USoundClassMixerSubsystem::OnDrawDebug(UCanvas* Canvas, APlayerController* PC)
{
	TArray<USoundClass*> Keys;
	SoundClassMap.GenerateKeyArray(Keys);
	Keys.Sort([](const USoundClass& A, const USoundClass& B)
	{
		return A.GetName() < B.GetName();
	});

	const FVector2D Origin(50, 50);
	FCanvasTableItem Table(Origin);
	for (const USoundClass* Key : Keys)
	{
		const FSoundClassSubSysProperties* Props = SoundClassMap.Find(Key);
		Table.AddElement("Current Volume", Key->GetName(), FString::Printf(TEXT("%.4f"), Props->Fader.GetVolume()));
		Table.AddElement("Target Volume", Key->GetName(), FString::Printf(TEXT("%.4f"), Props->Fader.GetTargetVolume()));
	}
	
	Canvas->DrawItem(Table);
}

#endif

// =====================================================================================================================

TStatId USoundClassMixerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USoundClassMixerSubsystem, STATGROUP_Tickables);
}