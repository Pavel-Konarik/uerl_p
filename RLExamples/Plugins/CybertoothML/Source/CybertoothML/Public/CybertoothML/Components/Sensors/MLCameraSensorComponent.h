// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/Components/MLSensorInterface.h"
#include "MLCameraSensorComponent.generated.h"


USTRUCT(BlueprintType)
struct FCameraSensorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 height;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float max_distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool use_gpu;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool grayscale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool render_owner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool srgb;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString attach_to_comp_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString attach_to_actor_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fov;

	/*
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Offset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation;
	*/

	FCameraSensorConfig()
	{
		width = 72;
		height = 72;
		max_distance = 8000.0f;
		use_gpu = false;
		grayscale = false;
		render_owner = true;
		srgb = false;
		attach_to_comp_name = "";
		attach_to_actor_name = "";
		fov = 90.0f;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API UMLCameraSensorComponent : public UActorComponent, public IMLSensorInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMLCameraSensorComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UActorComponent* CameraSensor;

#pragma region IMLSensorInterface
	FString GetMLName_Implementation() override;
	void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	void GetObservation_Implementation(FSensorObservation& OutData) override;
	void Reset_Implementation() override;
	void OnPostReset_Implementation() override;
	FString GetDebugString_Implementation() override;
	UTexture* GetDebugTexture_Implementation() override;
#pragma endregion IMLSensorInterface
	
	
};
