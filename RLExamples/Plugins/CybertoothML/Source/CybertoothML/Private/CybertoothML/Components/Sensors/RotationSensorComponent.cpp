// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Sensors/RotationSensorComponent.h"

// Sets default values for this component's properties
URotationSensorComponent::URotationSensorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
}


// Called when the game starts
void URotationSensorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void URotationSensorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ActorRotation = GetOwner()->GetActorRotation();
}

FString URotationSensorComponent::GetMLName_Implementation()
{
	return "rotation";
}

void URotationSensorComponent::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FRotationSensorConfig Config;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &Config))
	{
		bUseCyclic = Config.cyclic;
		bOnlyZ = Config.only_z;
		bZeroCentered = Config.zero_centered;
	}
}


// Function to convert degrees to radians, calculate sine and cosine, and log the results
void URotationSensorComponent::LogSineCosine(float Degrees, float& OutSine, float& OutCos)
{
	float Radians = FMath::DegreesToRadians(Degrees);
	OutSine = FMath::Sin(Radians);
	OutCos = FMath::Cos(Radians);

	if (bZeroCentered == false)
	{
		OutSine = (OutSine + 1.0f) / 2.0f;
		OutCos = (OutCos + 1.0f) / 2.0f;
	}
}

void URotationSensorComponent::GetObservationFloatArray(TArray<float>& OutFloatArray)
{
	OutFloatArray.Empty();
	OutFloatArray.AddUninitialized(bOnlyZ ? 2 : 6);

	if (bOnlyZ)
	{
		LogSineCosine(ActorRotation.Yaw, OutFloatArray[0], OutFloatArray[1]);
	}
	else {
		LogSineCosine(ActorRotation.Pitch, OutFloatArray[0], OutFloatArray[1]);
		LogSineCosine(ActorRotation.Yaw, OutFloatArray[2], OutFloatArray[3]);
		LogSineCosine(ActorRotation.Roll, OutFloatArray[4], OutFloatArray[5]);
	}
}

void URotationSensorComponent::GetObservation_Implementation(FSensorObservation& OutData)
{
	TArray<float> ObservationFloatArr;
	GetObservationFloatArray(ObservationFloatArr);

	check(ObservationFloatArr.Num() == 2 || ObservationFloatArr.Num() == 6);

	// Ensure OutFloatArray has elements
	if (ObservationFloatArr.Num() > 0)
	{
		// Calculate the number of bytes to copy
		int32 NumBytes = ObservationFloatArr.Num() * sizeof(float);

		// Resize the Data array to match the required size
		OutData.Data.SetNumUninitialized(NumBytes);

		// Copy the data
		FMemory::Memcpy(OutData.Data.GetData(), ObservationFloatArr.GetData(), NumBytes);
	}

}

void URotationSensorComponent::OnPostReset_Implementation()
{
	ActorRotation = GetOwner()->GetActorRotation();
}

FString URotationSensorComponent::GetDebugString_Implementation()
{
	FString DebugString;
	TArray<float> ObservationFloatArr;
	GetObservationFloatArray(ObservationFloatArr);

	for (int32 Index = 0; Index < ObservationFloatArr.Num(); ++Index)
	{
		DebugString += FString::Printf(TEXT("%d: %f"), Index, ObservationFloatArr[Index]);

		if (Index < ObservationFloatArr.Num() - 1)
		{
			DebugString += TEXT(", ");
		}
	}
	return DebugString;
}

