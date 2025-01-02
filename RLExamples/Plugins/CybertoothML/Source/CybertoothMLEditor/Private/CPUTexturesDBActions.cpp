// Fill out your copyright notice in the Description page of Project Settings.


#include "CPUTexturesDBActions.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FCPUTexturesDBActions::FCPUTexturesDBActions(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}

FColor FCPUTexturesDBActions::GetTypeColor() const
{
	return FColor::Red;
}

void FCPUTexturesDBActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);
}


void FCPUTexturesDBActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	/*
	auto Textures = GetTypedWeakObjectPtrs<UTexture>(InObjects);

	Section.AddMenuEntry(
		"Texture_CreateCPUData",
		LOCTEXT("Texture_CreateCPUData", "Create CPU Data"),
		LOCTEXT("Texture_CreateCPUDataTooltip", "Creates a new CPU Data using this texture."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Material"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FCPUTexturesDBActions::ExecuteCreateCPUData, Textures),
			FCanExecuteAction()
		)
	);
	*/
}


void FCPUTexturesDBActions::ExecuteCreateCPUData(TArray<TWeakObjectPtr<UTexture>> Objects)
{
	UE_LOG(LogTemp, Warning, TEXT("Here"));
	/*
	const FString DefaultSuffix = TEXT("_Mat");

	if (Objects.Num() == 1)
	{
		auto Object = Objects[0].Get();

		if (Object)
		{
			// Determine an appropriate name
			FString Name;
			FString PackagePath;
			CreateUniqueAssetName(Object->GetOutermost()->GetName(), DefaultSuffix, PackagePath, Name);

			// Create the factory used to generate the asset
			UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
			Factory->InitialTexture = Object;

			FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			ContentBrowserModule.Get().CreateNewAsset(Name, FPackageName::GetLongPackagePath(PackagePath), UMaterial::StaticClass(), Factory);
		}
	}
	else
	{
		TArray<UObject*> ObjectsToSync;

		for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
		{
			auto Object = (*ObjIt).Get();
			if (Object)
			{
				FString Name;
				FString PackageName;
				CreateUniqueAssetName(Object->GetOutermost()->GetName(), DefaultSuffix, PackageName, Name);

				// Create the factory used to generate the asset
				UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
				Factory->InitialTexture = Object;

				FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
				UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, FPackageName::GetLongPackagePath(PackageName), UMaterial::StaticClass(), Factory);

				if (NewAsset)
				{
					ObjectsToSync.Add(NewAsset);
				}
			}
		}

		if (ObjectsToSync.Num() > 0)
		{
			FAssetTools::Get().SyncBrowserToAssets(ObjectsToSync);
		}
	}
	*/
}


#undef LOCTEXT_NAMESPACE