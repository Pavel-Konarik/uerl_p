// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Sensors/LocationSensorComponent.h"

// Sets default values for this component's properties
ULocationSensorComponent::ULocationSensorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
	// ...
}


void ULocationSensorComponent::BeginPlay()
{
	Super::BeginPlay();

	LastLocation = GetOwner()->GetActorLocation();
}

// Called every frame
void ULocationSensorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	LastLocation = GetOwner()->GetActorLocation();
}



FString ULocationSensorComponent::GetMLName_Implementation()
{
	return "location";
}

void ULocationSensorComponent::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FLocationSensorConfig Config;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &Config))
	{
		bNormalise = Config.normalise;
		MaximumLocation = Config.maximum_location;
		MinimumLocation = Config.minimum_location;
	}
}

void ULocationSensorComponent::GetObservation_Implementation(FSensorObservation& OutData)
{
	FVector OutVector = GetProcessedLocation();
	TArray<float> ObservationFloatArr = { static_cast<float>(OutVector.X), static_cast<float>(OutVector.Y), static_cast<float>(OutVector.Z) };

	// Calculate the number of bytes to copy
	int32 NumBytes = ObservationFloatArr.Num() * sizeof(float);

	// Resize the Data array to match the required size
	OutData.Data.SetNumUninitialized(NumBytes);

	// Copy the data
	FMemory::Memcpy(OutData.Data.GetData(), ObservationFloatArr.GetData(), NumBytes);

}

FVector ULocationSensorComponent::GetProcessedLocation()
{
	FVector OutVector = LastLocation;
	if (bNormalise)
	{
		OutVector = (OutVector - MinimumLocation) / (MaximumLocation - MinimumLocation);

		// Clamp the FVector components between 0 and 1
		OutVector.X = FMath::Clamp(OutVector.X, 0.0f, 1.0f);
		OutVector.Y = FMath::Clamp(OutVector.Y, 0.0f, 1.0f);
		OutVector.Z = FMath::Clamp(OutVector.Z, 0.0f, 1.0f);
	}
	return OutVector;
}

void ULocationSensorComponent::OnPostReset_Implementation()
{
	LastLocation = GetOwner()->GetActorLocation();
}

FString ULocationSensorComponent::GetDebugString_Implementation()
{
	FVector OutVector = GetProcessedLocation();

	return OutVector.ToString();
}

