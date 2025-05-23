#pragma once

#include "CanvasItem.h"

class FCanvasTableItem final : public FCanvasItem
{
public:
	FCanvasTableItem(const FVector2D& InPosition)
		: FCanvasItem(InPosition),
		BorderThickness(1.0f),
		CellLinesThickness(1.0f),
		Padding(0.0), SizeMultiplier(1.0)
	{}

	virtual void Draw(FCanvas* InCanvas) override;

	using FCanvasItem::Draw; // Expose the functions defined in the base class.

	void AddElement(const FString& ColumnName, const FString& RowName, const FString& Value);
	void AddElement(const FString& ColumnName, const FString& RowName, const FString& Value, const FLinearColor& TextColor);

public:
	void SetPadding(const float NewPadding);
	void SetBorderThickness(const float NewThickness);
	void SetCellLinesThickness(const float NewThickness);
	void SetSizeMultiplier(const float NewSizeMultiplier);

	void SetWindowBgColor(const FLinearColor& NewWindowBgColor);
	void SetBorderColor(const FLinearColor& NewBorderColor);
	void SetCellsLinesColor(const FLinearColor& NewCellsLinesColor);

	void SetColumnsCellColor(const FLinearColor& NewColumnsCellColor);
	void SetColumnsLabelsColor(const FLinearColor& NewColumnsLabelsColor);

	void SetRowsCellColor(const FLinearColor& NewRowsCellColor);
	void SetRowsLabelsColor(const FLinearColor& NewRowsLabelsColor);

private:
	void SetupTableCorners();

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
	// TODO: SizeMultiplier
	float SizeMultiplier;
	
private:
	struct FTextData
	{
		FString Text;
		FLinearColor Color;
	};
	
	FVector2D CalculatedTableSize;
	TArray<FVector> TableCorners;
	
	TArray<FString> TableColumnLabels;
	TArray<int32> TableColumnWidth;
	
	TArray<FString> TableRowLabels;
	TArray<int32> TableRowHeight;
	
	TArray<TArray<FTextData>> TableElements;

	int32 MaxColumnLabelHeight = 0;
	int32 MaxRowLabelWidth = 0;
};
