// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CybertoothML/Components/MLSensorInterface.h"
#include "CPURendererComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API UCPURendererComponent : public USceneComponent, public IMLSensorInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCPURendererComponent();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region IMLSensorInterface
	FString GetMLName_Implementation() override;
	void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	void GetObservation_Implementation(FSensorObservation& OutData) override;
	void Reset_Implementation() override;
	void OnPostReset_Implementation() override;
	FString GetDebugString_Implementation() override;
	UTexture* GetDebugTexture_Implementation() override;
#pragma endregion IMLSensorInterface
	
	void RenderTick();
	static void RenderSingleFrame(const UObject* WorldContextObject, FVector CameraLocation, FRotator CameraRotation, int32 Width, int32 Height, float MaxTraceDistance, float FieldOfView, bool bUseBestLOD, bool bSRGB, FColor NoTraceColor, TArray<AActor*> IgnoreActors, TArray<FColor>& OutColors);



#pragma region Camera
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

	/** Last frame of the cpu render */
	UPROPERTY()
	TArray<FColor> CPURender;

	UPROPERTY()
	TArray<uint8> CPURender_Grayscale;

	/** sRGB color of when the render trace does not hit any object. Useful for sky etc. */
	UPROPERTY(EditDefaultsOnly, Category = "CPURender")
	FColor NoTraceColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPURender")
	bool bRenderOwner = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPURender")
	float FieldOfView = 90.0f;

#pragma endregion

	


#pragma region Utils

	FString GetCPURenderBase64();

	UFUNCTION()
	static void RenderFrameToFile(const UObject* WorldContextObject, FVector CameraLocation, FRotator CameraRotation, int32 Width, int32 Height, float FOV, bool bUseBestLOD, const FString& Filename);


#pragma endregion Utils

#pragma region Debug
	UPROPERTY(BlueprintReadOnly, Category = "CPURender")
	UTexture2D* CpuRenderTexture;

	void UpdateTextureFromBGRA(FColor* data, int width, int height);
	void UpdateTextureFromGrayscale(TArray<uint8>& InBytesData, int width, int height);
#pragma endregion Debug
};
