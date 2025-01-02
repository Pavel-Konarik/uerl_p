// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/Components/MLSensorInterface.h"
#include "VelocitySensorComponent.generated.h"

USTRUCT()
struct FVelocitySensorConfig
{
    GENERATED_BODY()

public:
    UPROPERTY()
    bool should_normalise;

	UPROPERTY()
	bool zero_centered;

	UPROPERTY()
	float max_velocity;

	UPROPERTY()
	bool max_velocity_from_cmc;

	FVelocitySensorConfig()
	{
		should_normalise = true;
		zero_centered = true;
		max_velocity = 500.0f;
		max_velocity_from_cmc = true;
	}

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API UVelocitySensorComponent : public UActorComponent, public IMLSensorInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVelocitySensorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool bShouldNormalise;
	bool bZeroCentered;
	float MaxVelocity;
	bool bMaxVelocityFromCMC;

	FVector ActorVelocity;

	FVector GetOwnerVelocity();

	void GetObservationFloatArray(TArray<float>& OutFloatArray);

#pragma region IMLSensorInterface
	FString GetMLName_Implementation() override;
	void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	void GetObservation_Implementation(FSensorObservation& OutData) override;
	void OnPostReset_Implementation() override;
	FString GetDebugString_Implementation() override;
#pragma endregion IMLSensorInterface
		
};
