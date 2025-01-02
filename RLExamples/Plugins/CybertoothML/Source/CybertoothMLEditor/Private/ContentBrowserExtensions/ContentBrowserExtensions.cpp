// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentBrowserExtensions/ContentBrowserExtensions.h"
#include "Modules/ModuleManager.h"
#include "Misc/PackageName.h"
#include "Textures/SlateIcon.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "Engine/Texture2D.h"
#include "AssetRegistry/AssetData.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "Materials/MaterialInterface.h"
#include "CybertoothML/CybertoothMLSettings.h"
#include "CybertoothMLEditor.h"

#define LOCTEXT_NAMESPACE "Paper2D"

DECLARE_LOG_CATEGORY_EXTERN(LogPaperCBExtensions, Log, All);
DEFINE_LOG_CATEGORY(LogPaperCBExtensions);

//////////////////////////////////////////////////////////////////////////

static FContentBrowserMenuExtender_SelectedAssets ContentBrowserExtenderDelegate;
static FDelegateHandle ContentBrowserExtenderDelegateHandle;

//////////////////////////////////////////////////////////////////////////
// FContentBrowserSelectedAssetExtensionBase

struct FContentBrowserSelectedAssetExtensionBase
{
public:
	TArray<struct FAssetData> SelectedAssets;

public:
	virtual void Execute() {}
	virtual ~FContentBrowserSelectedAssetExtensionBase() {}
};

//////////////////////////////////////////////////////////////////////////
// FCreateSpriteFromTextureExtension

#include "IAssetTools.h"
#include "AssetToolsModule.h"


struct FCreateCPUTextureAndStoreExtension : public FContentBrowserSelectedAssetExtensionBase
{
	bool bExtractSprites;

	FCreateCPUTextureAndStoreExtension()
		: bExtractSprites(false)
	{
	}
	
	void CreateCPUTexturesFromMaterials(TArray<UMaterialInterface*>& Materials)
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

		TArray<UObject*> ObjectsToSync;

		for (auto MaterialsIt = Materials.CreateConstIterator(); MaterialsIt; ++MaterialsIt)
		{
			UMaterialInterface* Material = *MaterialsIt;

			const UCybertoothMLSettings* CoreSettings = GetDefault<UCybertoothMLSettings>();
			check(CoreSettings);
			check(CoreSettings->CPUTexturesDB.LoadSynchronous());

			// RENDER MATERIAL TO DATABASE
			FCPUTextureData Arr = FCPUTextureData();
			FCybertoothMLEditorModule::CreateTextureFromMaterial(Material, Arr);

			CoreSettings->CPUTexturesDB->Data.Add(Material, Arr);

			UPackage* Package = CoreSettings->CPUTexturesDB->GetOutermost();
			if (Package)
			{
				Package->MarkPackageDirty();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to mark the package as dirty"));
			}

			
		}

		if (ObjectsToSync.Num() > 0)
		{
			ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
		}
	}

	virtual void Execute() override
	{
		// Create sprites for any selected textures
		TArray<UMaterialInterface*> Materials;
		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& AssetData = *AssetIt;
			if (UMaterialInterface* Material = Cast<UMaterialInterface>(AssetData.GetAsset()))
			{
				Materials.Add(Material);
			}
		}

		CreateCPUTexturesFromMaterials(Materials);
	}
};

//////////////////////////////////////////////////////////////////////////
// FMLContentBrowserExtensions_Impl

class FMLContentBrowserExtensions_Impl
{
public:
	static void ExecuteSelectedContentFunctor(TSharedPtr<FContentBrowserSelectedAssetExtensionBase> SelectedAssetFunctor)
	{
		SelectedAssetFunctor->Execute();
	}

	static void CreateCPUTextureActionsSubMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
	{
		MenuBuilder.AddSubMenu(
			LOCTEXT("SpriteActionsSubMenuLabel", "ML Actions"),
			LOCTEXT("SpriteActionsSubMenuToolTip", "Machine Learning-related actions for this material."),
			FNewMenuDelegate::CreateStatic(&FMLContentBrowserExtensions_Impl::PopulateTextureActionsMenu, SelectedAssets),
			false,
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Material")
		);
	}

	static void PopulateTextureActionsMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
	{
		// Create sprites
		TSharedPtr<FCreateCPUTextureAndStoreExtension> CPUTextureCreator = MakeShareable(new FCreateCPUTextureAndStoreExtension());
		CPUTextureCreator->SelectedAssets = SelectedAssets;

		FUIAction Action_CreateCPUTextureAndStore(
			FExecuteAction::CreateStatic(&FMLContentBrowserExtensions_Impl::ExecuteSelectedContentFunctor, StaticCastSharedPtr<FContentBrowserSelectedAssetExtensionBase>(CPUTextureCreator)));
		

		MenuBuilder.AddMenuEntry(
			LOCTEXT("CB_Extension_Material_CPUTexture", "Create CPU Material"),
			LOCTEXT("CB_Extension_Materual_CPUTexture_Tooltip", "Creates a CPU texture and stores it in a DB"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Material"),
			Action_CreateCPUTextureAndStore,
			NAME_None,
			EUserInterfaceActionType::Button);

	}

	static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
	{
		TSharedRef<FExtender> Extender(new FExtender());

		// Run thru the assets to determine if any meet our criteria
		bool bAnyMaterials = false;
		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& Asset = *AssetIt;
			bAnyMaterials = bAnyMaterials || (Asset.GetClass()->IsChildOf(UMaterialInterface::StaticClass()));
		}


		if (bAnyMaterials)
		{
			// Add the sprite actions sub-menu extender
			Extender->AddMenuExtension(
				"GetAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateStatic(&FMLContentBrowserExtensions_Impl::CreateCPUTextureActionsSubMenu, SelectedAssets));
		}
		return Extender;
	}

	static TArray<FContentBrowserMenuExtender_SelectedAssets>& GetExtenderDelegates()
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		return ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	}
};

//////////////////////////////////////////////////////////////////////////
// FPaperContentBrowserExtensions

void FMLContentBrowserExtensions::InstallHooks()
{
	ContentBrowserExtenderDelegate = FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FMLContentBrowserExtensions_Impl::OnExtendContentBrowserAssetSelectionMenu);

	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = FMLContentBrowserExtensions_Impl::GetExtenderDelegates();
	CBMenuExtenderDelegates.Add(ContentBrowserExtenderDelegate);
	ContentBrowserExtenderDelegateHandle = CBMenuExtenderDelegates.Last().GetHandle();
}

void FMLContentBrowserExtensions::RemoveHooks()
{
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = FMLContentBrowserExtensions_Impl::GetExtenderDelegates();
	CBMenuExtenderDelegates.RemoveAll([](const FContentBrowserMenuExtender_SelectedAssets& Delegate){ return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle; });
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
