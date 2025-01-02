// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "CybertoothML/Components/MLSensorInterface.h"
#include "MLCameraGPUComp.generated.h"

/**
 * 
 */
UCLASS(hidecategories = (Collision, Object, Physics, SceneComponent), ClassGroup = Rendering, editinlinenew, meta = (BlueprintSpawnableComponent))
class CYBERTOOTHML_API UMLCameraGPUComp : public USceneCaptureComponent2D, public IMLSensorInterface
{
	GENERATED_BODY()

public:
	UMLCameraGPUComp(const FObjectInitializer& ObjectInitializer);
	
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	void EnsureTextureExists();

#pragma region IMLSensorInterface
	FString GetMLName_Implementation() override;
	void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	void GetObservation_Implementation(FSensorObservation& OutData) override;
	void Reset_Implementation() override;
	void OnPostReset_Implementation() override;
	virtual class UTexture* GetDebugTexture_Implementation() override;
#pragma endregion IMLSensorInterface

	
#pragma region Camera
	/** Manually triggers the rendering at the end of the tick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPURender")
	int32 bPreciseMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPURender")
	int32 ResolutionWidth = 84;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPURender")
	int32 ResolutionHeight = 84;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPURender")
	float MaxTraceDistance = 15000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPURender")
	bool bUseGPU = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPURender")
	bool bSRGB = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPURender")
	bool bGrayscale = false;

	UPROPERTY(EditDefaultsOnly, Category = "CPURender")
	FColor NoTraceColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPURender")
	bool bRenderOwner = true;


#pragma endregion

protected:
	UPROPERTY()
	UMaterialInstanceDynamic* PostProcessMat;
};
