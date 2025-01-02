// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct FCPUTextureData;

class FCybertoothMLEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	void OnPostEngineInit();
	virtual void ShutdownModule() override;

	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();


	static bool CreateTextureFromMaterial(UMaterialInterface* Material, FCPUTextureData& CPUTexture);

	// Capture high-res 
	void RegisterCaptureCameraInfoConsoleCommand();
	void CaptureCameraInfo(int32 Width, int32 Height, const FString& OutputPath);
	
	void RegisterSettings();
	void UnregisterSettings();
private:
	void RegisterMenus();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
