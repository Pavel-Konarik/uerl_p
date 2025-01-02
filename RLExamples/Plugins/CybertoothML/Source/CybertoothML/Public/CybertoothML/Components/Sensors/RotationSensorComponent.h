// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/Components/MLSensorInterface.h"
#include "RotationSensorComponent.generated.h"

USTRUCT()
struct FRotationSensorConfig
{
    GENERATED_BODY()

public:
    UPROPERTY()
    bool cyclic;

	UPROPERTY()
	bool only_z;

	UPROPERTY()
	bool zero_centered;

	FRotationSensorConfig()
	{
		cyclic = true;
		only_z = true;
		zero_centered = false;
	}

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API URotationSensorComponent : public UActorComponent, public IMLSensorInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URotationSensorComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	FRotator ActorRotation;

	UPROPERTY()
	bool bUseCyclic;

	UPROPERTY()
	bool bOnlyZ;

	UPROPERTY()
	bool bZeroCentered;

	void LogSineCosine(float Degrees, float& OutSine, float& OutCos);

	void GetObservationFloatArray(TArray<float>& OutFloatArray);

#pragma region IMLSensorInterface
	FString GetMLName_Implementation() override;
	void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	void GetObservation_Implementation(FSensorObservation& OutData) override;
	void OnPostReset_Implementation() override;
	FString GetDebugString_Implementation() override;
#pragma endregion IMLSensorInterface
		
};
