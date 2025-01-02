// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Rewarders/WorldDirectionRewarderComp.h"

#include "JsonObjectConverter.h"

// Sets default values for this component's properties
UWorldDirectionRewarderComp::UWorldDirectionRewarderComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;

	LastLocation = FVector::ZeroVector;
}


// Called when the game starts
void UWorldDirectionRewarderComp::BeginPlay()
{
	Super::BeginPlay();

	LastLocation = GetOwner()->GetActorLocation();
	
}


// Called every frame
void UWorldDirectionRewarderComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector NewLoc = GetOwner()->GetActorLocation();

	// Calculate the displacement vector
	FVector Displacement = NewLoc - LastLocation;

	// Normalize the TargetWorldDirection if it's not already normalized
	FVector NormalizedTargetDirection = TargetWorldDirection.GetSafeNormal();

	// Project the displacement onto the TargetWorldDirection
	float ProjectedDisplacement = FVector::DotProduct(Displacement, NormalizedTargetDirection);

	// Calculate the reward based on the projected displacement
	CurrentReward = ProjectedDisplacement * RewardPerWorldUnit;

	// Update the last location for the next tick
	LastLocation = NewLoc;
}

#pragma region IMLRewarderInterface

FString UWorldDirectionRewarderComp::GetMLName_Implementation()
{
	return "world_direction";
}


void UWorldDirectionRewarderComp::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FWorldDirectionRewarderConfig WorldDirectionConfig;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &WorldDirectionConfig))
	{
		TargetWorldDirection = WorldDirectionConfig.direction;
	}
}

float UWorldDirectionRewarderComp::GetReward_Implementation()
{
	return CurrentReward;
}

float UWorldDirectionRewarderComp::ConsumeReward_Implementation()
{
	float Reward = CurrentReward;
	CurrentReward = 0.0f;

	return Reward;
}

void UWorldDirectionRewarderComp::Reset_Implementation()
{
	CurrentReward = 0.0f;
}

void UWorldDirectionRewarderComp::OnPostReset_Implementation()
{
	// User is now in new location, so we just reset it to start counting from there
	LastLocation = GetOwner()->GetActorLocation();
}

FString UWorldDirectionRewarderComp::GetDebugString_Implementation()
{
	return FString::Printf(TEXT("Current Reward: %f"), CurrentReward);;
}

#pragma endregion IMLRewarderInterface