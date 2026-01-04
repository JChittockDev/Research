// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvancedDeletionWidget.h"

/// runs on widget construction
void SAdvancedDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	// Declare a font we want to use

	StoredAssetsData = InArgs._AssetsDataToStore;
	
	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30;

#pragma region DeclareSlots
	// first slot
	ChildSlot
	[
		// Main Vertical Box
		SNew(SVerticalBox)
		
		// First slot in vertical box (Title)
		+SVerticalBox::Slot().AutoHeight()
		[
			// create text box for first slot
			// set its attributes
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("AdvancedDeletion")))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]

		// Second slot in vertical box (Condition Drop down)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
		]

		// Third slot in vertical box (AssetList)
		+ SVerticalBox::Slot().VAlign(VAlign_Fill)
		[
			SNew(SScrollBox)

			+SScrollBox::Slot()
			[
				SNew(SListView<TSharedPtr<FAssetData>>)
				.ItemHeight(24.f)
				.ListItemsSource(&StoredAssetsData)
				.OnGenerateRow(this, &SAdvancedDeletionTab::OnGenerateRowForList)
			]
		]

		// Third slot in vertical box (AssetList)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
		]
	];

#pragma endregion
}

TSharedRef<ITableRow> SAdvancedDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);
	}

	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget = SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
	[
		SNew(SHorizontalBox)

		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.FillWidth(0.5f)
		[
			ConstructCheckBox(AssetDataToDisplay)
		]
		// First slot for check box
		
		// Second slot for displaying asset class name

		// Third slot for physical asset name
		+SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(FText::FromString(DisplayAssetName))
		]
		// Fourth slot for a button

		//SNew(STextBlock)
		//.Text(FText::FromString(DisplayAssetName))
	];

	return ListViewRowWidget;
}

TSharedRef<SCheckBox> SAdvancedDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.OnCheckStateChanged(this, &SAdvancedDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay)
	.Visibility(EVisibility::Visible);
	return ConstructedCheckBox;
}

void SAdvancedDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{

}
