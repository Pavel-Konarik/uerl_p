// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CybertoothML/Components/MLSensorInterface.h"
#include "CybertoothML/MLTypes.h"
#include "DistanceTracerComponent.generated.h"


USTRUCT(BlueprintType)
struct FDistanceTracerConfig
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 num_traces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float half_angle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float trace_length;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool should_normalise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString trace_channel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool invert_values;

	FDistanceTracerConfig()
	{
		num_traces = 10;
		half_angle = 90.0f;
		trace_length = 8000.0f;
		should_normalise = true;
		trace_channel = "Visibility";
		rotation = FRotator::ZeroRotator;
		location = FVector::ZeroVector;
		invert_values = true;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API UDistanceTracerComponent : public USceneComponent, public IMLSensorInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDistanceTracerComponent();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
#pragma region IMLSensorInterface
	FString GetMLName_Implementation() override;
	void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	void GetObservation_Implementation(FSensorObservation& OutData) override;
	void OnPostReset_Implementation() override;
	class UTexture2D* GetDebugTexture_Implementation() override;
#pragma endregion IMLSensorInterface

	/** Last frame of traces. Each value is the distance to a hit */
	UPROPERTY()
	TArray<float> TracesBuffer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traces")
	int32 NumTraces = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traces")
	float HalfAngle = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traces")
	float TraceLength = 6000.0f; // Length of traces

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traces")
	bool bShouldNormalise = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traces")
	FRotator InitRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traces")
	FVector InitLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traces")
	bool bInvertValues;

	UFUNCTION(BlueprintCallable, Category="Trace")
	void TraceLines();

	UPROPERTY(EditDefaultsOnly, Category="Trace")
	TEnumAsByte<ETraceTypeQuery> TraceType;

	// DEBUG
	void UpdateTextureFromTraces(int width, int height);

	UPROPERTY(BlueprintReadOnly, Category = "CPURender")
	UTexture2D* DebugRenderTexture;
};
