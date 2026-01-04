// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/QuickAssetAction.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "DebugHeader.h"
#include "ObjectTools.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"

/// <summary> 
/// Function that duplicates all selected assets n number of times
/// </summary>
/// Variable to set the number of duplicates per asset
/// <param name="NumOfDuplicates"></param>
void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	// Check to make sure number is greater than 0
	if (NumOfDuplicates <= 0)
	{
		// Use a message dialog to call error
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a VALID number of duplicates"));
		return;
	}

	// Get the assets that are selected in a local variable
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();

	uint32 AssetCount = 0;
	// For each selected asset
	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		// For the number of duplicates to make
		for (int32 i = 0; i < NumOfDuplicates; i++)
		{
			// Get the source objects asset path
			const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();
			// Generate a new name for the duplicate
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			// Append the old package path and new name to get the new asset path
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicatedAssetName);
			// Duplicate the asset
			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				// Save the duplicated asset
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				// iterate the counter
				++AssetCount;
			}
		}
	}

	// If sucsess show notification
	if (AssetCount > 0)
	{
		DebugHeader::ShowNotifyInfo(FString::FromInt(SelectedAssetsData.Num()) + TEXT(" Assets Duplicated Sucssefully"));
	}
	else
	{
		// If error use message dialog
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Asset Duplication Faliure"));
	}
}

/// <summary>
/// Function to add a prefix to the selected objects based on their class
/// </summary>
void UQuickAssetAction::AddPrefix()
{
	// Get the selected assets
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 AssetCount = 0;

	// Loop through them
	for (UObject* SelectedObject : SelectedObjects)
	{
		// If the object isnt valid, go to the next one
		if (!SelectedObject) continue;

		// Get the prefix if it exists
		FString* Prefix = PrefixMap.Find(SelectedObject->GetClass());

		if (!Prefix || Prefix->IsEmpty())
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName());
			continue;
		}

		// Store the objects name
		FString OldName = SelectedObject->GetName();

		// Make sure it dosent already have the prefix
		if (OldName.StartsWith(*Prefix))
		{
			DebugHeader::ShowNotifyInfo(OldName + TEXT(" already has the prefix ") + *Prefix);
			continue;
		}

		// If its a MI then remove the previous material prefix
		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		// Construct new name
		const FString NewNameWithPrefix = *Prefix + OldName;

		// rename the asset
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);
		++AssetCount;
	}

	if (AssetCount > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully renamed ") + FString::FromInt(AssetCount) + TEXT(" assets"));
	}
}

/// <summary>
/// Function to remove assets that arent being used by anything
/// </summary>
void UQuickAssetAction::RemovedUnusedAssets()
{
	// Get the selected assets data
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;

	// Clean up any registry issues
	FixUpRedirectors();

	// Loop through the selected assets
	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		// Check to see what rereferences they have
		TArray<FString> AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.ObjectPath.ToString());

		// If they dont have any, add em to the list
		if (AssetReferencers.Num() == 0)
		{
			UnusedAssetsData.Add(SelectedAssetData);
		}
	}

	if (UnusedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused assets found amongst selection"), false);
		return;
	}

	// Remove the assets that dont have any references
	int32 AssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);

	if (AssetsDeleted == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No assets were deleted"));
		return;
	}

	DebugHeader::ShowNotifyInfo(FString::FromInt(AssetsDeleted) + TEXT(" assets successfully deleted"));
}

void UQuickAssetAction::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	// Load and get the asset registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// Build a filter to find the correct asset registry info
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");
	
	// Get the ObjectRedirector assets in registry entries that match filter
	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	// For each asset
	for (const FAssetData& RedirectorAssetData : OutRedirectors)
	{
		// Cast the Asset back to a redirector, so it can redirect shit
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorAssetData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	// Load and get the asset tools module
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	// Pass the redirectors to asset tools so it can fix them
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}
