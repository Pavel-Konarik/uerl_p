// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/Components/MLSensorInterface.h"
#include "LocationSensorComponent.generated.h"



USTRUCT()
struct FLocationSensorConfig
{
    GENERATED_BODY()

public:
    UPROPERTY()
    bool normalise;

	UPROPERTY()
	FVector minimum_location;

	UPROPERTY()
	FVector maximum_location;


	FLocationSensorConfig()
	{
		normalise = false;
		minimum_location = FVector::ZeroVector;
		maximum_location = FVector::ZeroVector;
	}

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API ULocationSensorComponent : public UActorComponent, public IMLSensorInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULocationSensorComponent();

	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	FVector LastLocation;

	UPROPERTY()
	bool bNormalise = false;

	UPROPERTY()
	FVector MinimumLocation;

	UPROPERTY()
	FVector MaximumLocation;

	FVector GetProcessedLocation();

		
#pragma region IMLSensorInterface
	FString GetMLName_Implementation() override;
	void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	void GetObservation_Implementation(FSensorObservation& OutData) override;
	void OnPostReset_Implementation() override;
	FString GetDebugString_Implementation() override;
#pragma endregion IMLSensorInterface
};
