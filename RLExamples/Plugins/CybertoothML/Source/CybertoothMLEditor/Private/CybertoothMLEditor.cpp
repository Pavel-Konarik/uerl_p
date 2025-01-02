// Copyright Epic Games, Inc. All Rights Reserved.

#include "CybertoothMLEditor.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "CybertoothMLPluginStyle.h"
#include "ContentBrowserExtensions/ContentBrowserExtensions.h"
#include "CybertoothMLCommands.h"
#include "SMLUtilsWindow.h"
#include "CPUTexturesDBActions.h"
#include "SceneTypes.h"

#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Blueprint/BlueprintSupport.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "IMaterialBakingModule.h"
#include "MaterialOptions.h"
#include "MaterialBakingStructures.h"
#include "Engine/SkeletalMesh.h"
#include "CybertoothML/CPUTexturesDB.h"
#include "HAL/IConsoleManager.h"
#include "CybertoothML/Components/Sensors/CPURendererComponent.h"
#include "ISettingsModule.h"
#include "CybertoothML/CybertoothMLSettings.h"

static const FName CybertoothMLUtilsTabName("AI Utils");

#define LOCTEXT_NAMESPACE "FCybertoothMLEditorModule"

void FCybertoothMLEditorModule::StartupModule()
{

	FCybertoothMLPluginStyle::Initialize();
	FCybertoothMLPluginStyle::ReloadTextures();
	
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	EAssetTypeCategories::Type gameAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("MachineLearning")), FText::FromName(TEXT("MachineLearning")));

	TSharedPtr<IAssetTypeActions> actionType2 = MakeShareable(new FCPUTexturesDBActions(gameAssetCategory));
	AssetTools.RegisterAssetTypeActions(actionType2.ToSharedRef());


	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FCybertoothMLEditorModule::OnPostEngineInit);




	FCybertoothMLCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FCybertoothMLCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FCybertoothMLEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCybertoothMLEditorModule::RegisterMenus));
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(CybertoothMLUtilsTabName, FOnSpawnTab::CreateRaw(this, &FCybertoothMLEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FCybertoothMLEditorModule", "UtilsWindowPlugin"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	
}

void FCybertoothMLEditorModule::OnPostEngineInit()
{
	if (!IsRunningCommandlet())
	{
		FMLContentBrowserExtensions::InstallHooks();
		RegisterCaptureCameraInfoConsoleCommand();
		RegisterSettings();
	}
}

void FCybertoothMLEditorModule::ShutdownModule()
{
	FCoreDelegates::OnPostEngineInit.RemoveAll(this);

	FMLContentBrowserExtensions::RemoveHooks();

	if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}


TSharedRef<SDockTab> FCybertoothMLEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SMLUtilsWindow)
		];
}

void FCybertoothMLEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(CybertoothMLUtilsTabName);
}

void FCybertoothMLEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FCybertoothMLCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FCybertoothMLCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}




bool FCybertoothMLEditorModule::CreateTextureFromMaterial(UMaterialInterface* Material, FCPUTextureData& CPUTexture)
{
	IMaterialBakingModule& MaterialBakingModule = FModuleManager::Get().LoadModuleChecked<IMaterialBakingModule>("MaterialBaking");
	/*
	TArray<FMeshData> GlobalMeshSettings;
	TArray<FMaterialData> GlobalMaterialSettings;

	// Collect data in these to pass to baker module
	TArray<FMaterialData*> MaterialSettingPtrs;
	TArray<FMeshData*> MeshSettingPtrs;

	TArray<FIntPoint> LODsSizes{ FIntPoint(256, 256),  FIntPoint(128, 128),  FIntPoint(64, 64),  FIntPoint(32, 32),  FIntPoint(8, 8) };

	for (FIntPoint LODSize : LODsSizes)
	{
		FMeshData& MeshData = GlobalMeshSettings.AddDefaulted_GetRef();

		FMaterialData& MaterialSettings = GlobalMaterialSettings.AddDefaulted_GetRef();
		MaterialSettings.Material = Material;
		MaterialSettings.PropertySizes.Add(EMaterialProperty::MP_BaseColor, LODSize);

		MeshSettingPtrs.Add(&MeshData);
		MaterialSettingPtrs.Add(&MaterialSettings);
	}

	check(MaterialSettingPtrs[0]->PropertySizes.Num() > 0);

*/


	TArray<FMeshData> GlobalMeshSettings;
	TArray<FMaterialData> GlobalMaterialSettings;


	TArray<FIntPoint> LODsSizes { FIntPoint(512, 512), FIntPoint(256, 256),  FIntPoint(128, 128),  FIntPoint(64, 64),  FIntPoint(32, 32),  FIntPoint(8, 8) };

	for (FIntPoint LODSize : LODsSizes)
	{
		FMeshData MeshData = GlobalMeshSettings.AddDefaulted_GetRef();

		FMaterialData MaterialSettings; // = GlobalMaterialSettings.AddDefaulted_GetRef();
		MaterialSettings.Material = Material;
		MaterialSettings.PropertySizes.Add(EMaterialProperty::MP_BaseColor, LODSize);

		FMaterialData MaterialSettingsInGlobals = GlobalMaterialSettings.Add_GetRef(MaterialSettings);
	}

	// Collect data in these to pass to baker module

	TArray<FMeshData*> MeshSettingPtrs;
	for (int32 SettingsIndex = 0; SettingsIndex < GlobalMeshSettings.Num(); ++SettingsIndex)
	{
		MeshSettingPtrs.Add(&GlobalMeshSettings[SettingsIndex]);
	}

	TArray<FMaterialData*> MaterialSettingPtrs;
	for (int32 SettingsIndex = 0; SettingsIndex < GlobalMaterialSettings.Num(); ++SettingsIndex)
	{
		MaterialSettingPtrs.Add(&GlobalMaterialSettings[SettingsIndex]);
	}

	check(MaterialSettingPtrs[0]->PropertySizes.Num() > 0);

	IMaterialBakingModule& Module = FModuleManager::Get().LoadModuleChecked<IMaterialBakingModule>("MaterialBaking");

	TArray<FBakeOutput> BakeOutputs;
	Module.BakeMaterials(MaterialSettingPtrs, MeshSettingPtrs, BakeOutputs);

	// Get the Pixel data of the rendered texture
	for (FBakeOutput BakeOutput : BakeOutputs) {
		FTextureLOD TextureLOD;

		TArray<FColor> PixelData = BakeOutput.PropertyData[EMaterialProperty::MP_BaseColor];
		
		TextureLOD.Width = BakeOutput.PropertySizes[EMaterialProperty::MP_BaseColor].X;
		TextureLOD.Height = BakeOutput.PropertySizes[EMaterialProperty::MP_BaseColor].Y;

		// Copy the actual texture colors over to the new TextureLOD
		TextureLOD.Colors.Empty();
		TextureLOD.Colors.AddUninitialized(PixelData.Num());

		for (int32 i = 0; i < PixelData.Num(); i++)
		{
			// Ensure we ignore alpha
			PixelData[i].A = 255;
			TextureLOD.Colors[i] = PixelData[i];
		}
		
		CPUTexture.LODs.Add(TextureLOD);

		UE_LOG(LogTemp, Warning, TEXT("Generated texture for %s with size (%d, %d)"), *Material->GetName(), TextureLOD.Width, TextureLOD.Height);

	}

	return true;
}


void FCybertoothMLEditorModule::RegisterCaptureCameraInfoConsoleCommand()
{
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("CaptureCameraInfo"),
		TEXT("Capture camera location and rotation and process with given width, height, and output path. Usage: CaptureCameraInfo Width Height OutputPath"),
		FConsoleCommandWithArgsDelegate::CreateLambda([this](const TArray<FString>& Args)
	{
		if (Args.Num() == 3)
		{
			int32 Width = FCString::Atoi(*Args[0]);
			int32 Height = FCString::Atoi(*Args[1]);
			FString OutputPath = Args[2];

			CaptureCameraInfo(Width, Height, OutputPath);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid arguments. Usage: CaptureCameraInfo Width Height OutputPath"));
		}
	}),
		ECVF_Default);
}

void FCybertoothMLEditorModule::CaptureCameraInfo(int32 Width, int32 Height, const FString& OutputPath)
{
	if (GEditor)
	{
		for (FEditorViewportClient* ViewportClient : GEditor->GetAllViewportClients())
		{
			if (ViewportClient && ViewportClient->IsPerspective() && !ViewportClient->IsInGameView())
			{
				// Get camera location and rotation
				FVector OutCameraLocation = ViewportClient->GetViewLocation();
				FRotator OutCameraRotation = ViewportClient->GetViewRotation();
				float FieldOfView = 90.0f;

				UWorld* World = GEditor->GetEditorWorldContext(false).World();

				UCPURendererComponent::RenderFrameToFile(World, OutCameraLocation, OutCameraRotation, Width, Height, FieldOfView, true, OutputPath);
			}
		}
	}
	/*
		ULocalPlayer* LocalPlayer = GEngine->GetLocalPlayerFromControllerId(GEngine->GetWorld(), 0);
		if (LocalPlayer && LocalPlayer->PlayerController)
		{
			APlayerController* PlayerController = LocalPlayer->PlayerController;
			AActor* ViewTarget = PlayerController->GetViewTarget();
			if (ViewTarget)
			{
				FVector CameraLocation = ViewTarget->GetActorLocation();
				FRotator CameraRotation = ViewTarget->GetActorRotation();

				UE_LOG(LogTemp, Warning, TEXT("Camera Location: %s"), *CameraLocation.ToString());
				UE_LOG(LogTemp, Warning, TEXT("Camera Rotation: %s"), *CameraRotation.ToString());

				// TODO: Implement functionality for Width, Height, and OutputPath
			}
		}
	*/
}


void FCybertoothMLEditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "CybertoothML",
			LOCTEXT("RuntimeSettingsName", "Cybertooth ML"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the CybertoothML plugin"),
			GetMutableDefault<UCybertoothMLSettings>());
	}
}

void FCybertoothMLEditorModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "CybertoothML");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCybertoothMLEditorModule, CybertoothMLEditor)