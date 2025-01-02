// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CPUTexturesDB.generated.h"

class UMaterial;
class UMaterialInterface;



USTRUCT()
struct FTextureLOD
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Width;

	UPROPERTY()
	int32 Height;

	UPROPERTY()
	TArray<FColor> Colors;

	FTextureLOD()
	{
		Width = 0;
		Height = 0;
		Colors = TArray<FColor>();
	}
};

USTRUCT()
struct FCPUTextureData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FTextureLOD> LODs;

	FCPUTextureData()
	{
		LODs = TArray<FTextureLOD>();
	}
};

/**
 * 
 */
UCLASS(BlueprintType)
class CYBERTOOTHML_API UCPUTexturesDB : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<TSoftObjectPtr<UMaterialInterface>, FCPUTextureData> Data;

	const FCPUTextureData* GetCPUTexture(UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "CPU_Render")
	FColor GetPixelFromUV(UMaterialInterface* Material, float Distance, float U, float V, bool bUseBestLOD);

	int32 GetLODLevelFromDistance(float Distance) const;
	// To avoid iterating over the Material Inheritance Chain, we cache results if found.
	// Material from world -> Material we have in DB (could be null)
	UPROPERTY()
	TMap<const UMaterialInterface*, const UMaterialInterface*> MaterialCache;
	//TMap<TWeakObjectPtr<UMaterialInterface>, TWeakObjectPtr<UMaterialInterface>> MaterialCache;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
	void PrintDebug();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
	void ClearDatabase();
};

