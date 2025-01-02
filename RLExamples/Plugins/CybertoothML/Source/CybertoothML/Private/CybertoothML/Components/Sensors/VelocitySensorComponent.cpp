// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Sensors/VelocitySensorComponent.h"

#include "GameFramework/PawnMovementComponent.h"

// Sets default values for this component's properties
UVelocitySensorComponent::UVelocitySensorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
}

// Called every frame
void UVelocitySensorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ActorVelocity = GetOwnerVelocity();
}

FVector UVelocitySensorComponent::GetOwnerVelocity()
{
	FVector Velocity = FVector::ZeroVector;
	float MaxVelocityLocal = MaxVelocity;

	// If this actor is a Pawn
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->GetMovementComponent())
	{
		// Return the velocity from the movement component
		Velocity = Pawn->GetMovementComponent()->Velocity;

		if (bMaxVelocityFromCMC)
		{
			MaxVelocityLocal = Pawn->GetMovementComponent()->GetMaxSpeed();
		}
	}
	else
	{
		// Return the velocity from the actor
		Velocity = GetOwner()->GetVelocity();
	}

	if (bShouldNormalise)
	{
		Velocity /= MaxVelocityLocal;
		// Clamp the velocity to the range [-1, 1]
		Velocity.X = FMath::Clamp(Velocity.X, -1.0f, 1.0f);
		Velocity.Y = FMath::Clamp(Velocity.Y, -1.0f, 1.0f);
		Velocity.Z = FMath::Clamp(Velocity.Z, -1.0f, 1.0f);
	}

	if (bShouldNormalise && !bZeroCentered)
	{
		Velocity.X = (Velocity.X + 1.0f) / 2.0f;
		Velocity.Y = (Velocity.Y + 1.0f) / 2.0f;
		Velocity.Z = (Velocity.Z + 1.0f) / 2.0f;
	}

	return Velocity;
}


FString UVelocitySensorComponent::GetMLName_Implementation()
{
	return "velocity_sensor";
}

void UVelocitySensorComponent::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FVelocitySensorConfig Config;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &Config))
	{
		bZeroCentered = Config.zero_centered;
		bShouldNormalise = Config.should_normalise;
		MaxVelocity = Config.max_velocity;
		//bMaxVelocityFromCMC = bMaxVelocityFromCMC;
	}

	check(bMaxVelocityFromCMC == false || MaxVelocity > 0.0f);

}


void UVelocitySensorComponent::GetObservationFloatArray(TArray<float>& OutFloatArray)
{
	OutFloatArray.Empty();
	OutFloatArray.AddUninitialized(3);

	OutFloatArray[0] = ActorVelocity.X;
	OutFloatArray[1] = ActorVelocity.Y;
	OutFloatArray[2] = ActorVelocity.Z;
}

void UVelocitySensorComponent::GetObservation_Implementation(FSensorObservation& OutData)
{
	TArray<float> ObservationFloatArr;
	GetObservationFloatArray(ObservationFloatArr);

	check(ObservationFloatArr.Num() == 3);

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

void UVelocitySensorComponent::OnPostReset_Implementation()
{
	ActorVelocity = GetOwnerVelocity();
}

FString UVelocitySensorComponent::GetDebugString_Implementation()
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

