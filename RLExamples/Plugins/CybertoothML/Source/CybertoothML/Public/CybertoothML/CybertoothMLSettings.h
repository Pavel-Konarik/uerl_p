// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CPUTexturesDB.h"
#include "UObject/NoExportTypes.h"
#include "CybertoothMLSettings.generated.h"

class UUserWidget;

/**
 * 
UCLASS(Config = Game,  meta = (DisplayName = "CybertoothML Settings"))
class CYBERTOOTHML_API UCybertoothMLSettings : public UDeveloperSettings
 */

UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "CybertoothML Settings"))
class CYBERTOOTHML_API UCybertoothMLSettings : public UObject
{
	GENERATED_BODY()

public:
	UCybertoothMLSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Textures")
	TSoftObjectPtr<UCPUTexturesDB> CPUTexturesDB;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Textures")
	TArray<float> LODsDistances;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Textures")
	TSoftObjectPtr<UMaterialInterface> AgentGPUCameraPostProcessMat; 

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Rendering")
	FName ActorIgnoreRenderTag;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Rendering")
	TArray<TSoftClassPtr<AActor>> IgnoreRenderClasses;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Rendering")
	TSoftClassPtr<UUserWidget> HumanModeWidget;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Rendering")
	TMap<FString, TEnumAsByte<ETraceTypeQuery>> TraceChannelsMap;

};
