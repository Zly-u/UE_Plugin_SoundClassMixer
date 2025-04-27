#include "CanvasTableItem.h"

void FCanvasTableItem::Draw(FCanvas* InCanvas)
{
	SetupTableCorners();
		
	// Bg for Elements
	FCanvasTileItem Bg(
		Position + FVector2D(MaxRowLabelWidth, MaxColumnLabelHeight),
		CalculatedTableSize - FVector2D(MaxRowLabelWidth, MaxColumnLabelHeight),
		WindowBgColor
	);
	Bg.BlendMode = SE_BLEND_Translucent;
	InCanvas->DrawItem(Bg);

	// Bg for Columns Headers
	FCanvasTileItem CH_Bg(
		Position + FVector2D(MaxRowLabelWidth, 0),
		FVector2D(CalculatedTableSize.X - MaxRowLabelWidth, MaxColumnLabelHeight),
		ColumnsCellColor
	);
	CH_Bg.BlendMode = SE_BLEND_Translucent;
	InCanvas->DrawItem(CH_Bg);

	// Bg for Rows Headers
	FCanvasTileItem CR_Bg(
		Position + FVector2D(0, MaxColumnLabelHeight),
		FVector2D(MaxRowLabelWidth, CalculatedTableSize.Y - MaxColumnLabelHeight),
		RowsCellColor
	);
	CR_Bg.BlendMode = SE_BLEND_Translucent;
	InCanvas->DrawItem(CR_Bg);
		
	FBatchedElements* BatchedElements = InCanvas->GetBatchedElements(FCanvas::ET_Line);
	const FHitProxyId HitProxyId = InCanvas->GetHitProxyId();
		
	// Draw Row/Column Separator.
	BatchedElements->AddLine(
		FVector(
			Position.X,
			Position.Y,
			0.f
		),
		FVector(
			Position.X + MaxRowLabelWidth,
			Position.Y + MaxColumnLabelHeight,
			0.0f
		),
		WindowBgColor, HitProxyId, CellLinesThickness
	);
		
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
				Padding + PrevColumnLabelPos, Padding + Position.Y,
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
				Padding + Position.X, Padding + PrevRowLabelPos,
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
				FTextData& TextData = TableElements[RowLabelIndex][ColumnLabelIndex];
				InCanvas->DrawShadowedString(
					Padding + PrevColumnLabelPos, Padding + PrevRowLabelPos,
					*TextData.Text,
					RenderFont,
					TextData.Color
				);
					
				PrevColumnLabelPos += TableColumnWidth[ColumnLabelIndex];
			}

			PrevRowLabelPos += TableRowHeight[RowLabelIndex];
		}
	}
}

// =====================================================================================================================

void FCanvasTableItem::AddElement(const FString& ColumnName, const FString& RowName, const FString& Value)
{
	AddElement(ColumnName, RowName, Value, FLinearColor::Green);
}

void FCanvasTableItem::AddElement(const FString& ColumnName, const FString& RowName, const FString& Value,
                                  const FLinearColor& TextColor)
{
	const UFont* RenderFont = GEngine->GetMediumFont();

	MaxColumnLabelHeight = FMath::Max(MaxColumnLabelHeight, RenderFont->GetStringHeightSize(*ColumnName)); 
	MaxRowLabelWidth = FMath::Max(MaxRowLabelWidth, RenderFont->GetStringSize(*RowName));
		
	const int32 ColumnWidth = Padding * 2.f + FMath::Max(
		FMath::Max(
			RenderFont->GetStringSize(*ColumnName),
			RenderFont->GetStringSize(*RowName)
		),
		RenderFont->GetStringSize(*Value)
	);

	const int32 RowHeight = Padding * 2.f + FMath::Max(
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
		
	TableElements[Y_Index][X_Index] = {
		Value,
		TextColor
	};
}

// =====================================================================================================================

void FCanvasTableItem::SetPadding(const float NewPadding)
{
	Padding = NewPadding;
}

void FCanvasTableItem::SetBorderThickness(const float NewThickness)
{
	BorderThickness = NewThickness;
}

void FCanvasTableItem::SetCellLinesThickness(const float NewThickness)
{
	CellLinesThickness = NewThickness;
}

void FCanvasTableItem::SetSizeMultiplier(const float NewSizeMultiplier)
{
	SizeMultiplier = NewSizeMultiplier;
}

void FCanvasTableItem::SetWindowBgColor(const FLinearColor& NewWindowBgColor)
{
	WindowBgColor = NewWindowBgColor;
}

void FCanvasTableItem::SetBorderColor(const FLinearColor& NewBorderColor)
{
	BorderColor = NewBorderColor;
}

void FCanvasTableItem::SetCellsLinesColor(const FLinearColor& NewCellsLinesColor)
{
	CellsLinesColor = NewCellsLinesColor;
}

void FCanvasTableItem::SetColumnsCellColor(const FLinearColor& NewColumnsCellColor)
{
	ColumnsCellColor = NewColumnsCellColor;
}

void FCanvasTableItem::SetColumnsLabelsColor(const FLinearColor& NewColumnsLabelsColor)
{
	ColumnsLabelsColor = NewColumnsLabelsColor;
}

void FCanvasTableItem::SetRowsCellColor(const FLinearColor& NewRowsCellColor)
{
	RowsCellColor = NewRowsCellColor;
}

void FCanvasTableItem::SetRowsLabelsColor(const FLinearColor& NewRowsLabelsColor)
{
	RowsLabelsColor = NewRowsLabelsColor;
}

// =====================================================================================================================

void FCanvasTableItem::SetupTableCorners()
{
	MaxRowLabelWidth += Padding * 2.f;
	MaxColumnLabelHeight += Padding * 2.f;
		
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
