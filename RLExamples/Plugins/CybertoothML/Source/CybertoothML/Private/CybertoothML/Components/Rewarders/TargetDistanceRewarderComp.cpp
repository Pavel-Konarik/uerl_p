// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Rewarders/TargetDistanceRewarderComp.h"

#include "JsonObjectConverter.h"


// Sets default values for this component's properties
UTargetDistanceRewarderComp::UTargetDistanceRewarderComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;

	LastLocation = FVector::ZeroVector;
}


// Called when the game starts
void UTargetDistanceRewarderComp::BeginPlay()
{
	Super::BeginPlay();

	LastLocation = GetOwner()->GetActorLocation();
	
}


// Called every frame
void UTargetDistanceRewarderComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector NewLoc = GetOwner()->GetActorLocation();
	
	FVector DisplacementToTargetFromNewLoc = TargetLocation - NewLoc;
	FVector DisplacementToTargetFromLastLoc = TargetLocation - LastLocation;

	// Calculate the change in distance to the target (negative value means moving closer)
	float DistanceChange = DisplacementToTargetFromNewLoc.Size() - DisplacementToTargetFromLastLoc.Size();

	// Reward is positive if moving closer (DistanceChange is negative), negative if moving away
	CurrentReward = -DistanceChange * RewardPerWorldUnit;

	// Update LastLocation for the next tick
	LastLocation = NewLoc;
}


#pragma region IMLRewarderInterface

FString UTargetDistanceRewarderComp::GetMLName_Implementation()
{
	return "target_distance";
}


void UTargetDistanceRewarderComp::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FTargetDistanceRewarderConfig TargetLocationConfig;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &TargetLocationConfig))
	{
		TargetLocation = TargetLocationConfig.target_location;
	}
}

float UTargetDistanceRewarderComp::GetReward_Implementation()
{
	return CurrentReward;
}

float UTargetDistanceRewarderComp::ConsumeReward_Implementation()
{
	float Reward = CurrentReward;
	CurrentReward = 0.0f;

	LastReward_Debug = Reward;
	TotalReward_Debug += Reward;

	return Reward;
}

void UTargetDistanceRewarderComp::Reset_Implementation()
{
	CurrentReward = 0.0f;

	LastReward_Debug = 0.0f;
	TotalReward_Debug = 0.0f;
}

void UTargetDistanceRewarderComp::OnPostReset_Implementation()
{
	// User is now in new location, so we just reset it to start counting from there
	LastLocation = GetOwner()->GetActorLocation();
}

FString UTargetDistanceRewarderComp::GetDebugString_Implementation()
{
	return FString::Printf(TEXT("Last Reward: %f (total: %f)"), LastReward_Debug, TotalReward_Debug);;
}

#pragma endregion IMLRewarderInterface