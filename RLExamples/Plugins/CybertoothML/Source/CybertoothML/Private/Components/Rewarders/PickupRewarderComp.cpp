// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Rewarders/PickupRewarderComp.h"

#include "JsonObjectConverter.h"

// Sets default values for this component's properties
UPickupRewarderComp::UPickupRewarderComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
}


// Called when the game starts
void UPickupRewarderComp::BeginPlay()
{
	Super::BeginPlay();

	
}


// Called every frame
void UPickupRewarderComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}

#pragma region IMLRewarderInterface

FString UPickupRewarderComp::GetMLName_Implementation()
{
	return "pickup_reward";
}


void UPickupRewarderComp::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FPickupRewarderConfig RewarderConfig;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &RewarderConfig))
	{
		RewardInstanceName = RewarderConfig.name;
	}
}

float UPickupRewarderComp::GetReward_Implementation()
{
	return CurrentReward;
}

float UPickupRewarderComp::ConsumeReward_Implementation()
{
	float Reward = CurrentReward;
	CurrentReward = 0.0f;

	return Reward;
}

void UPickupRewarderComp::Reset_Implementation()
{
	CurrentReward = 0.0f;
}

void UPickupRewarderComp::OnPostReset_Implementation()
{

}

FString UPickupRewarderComp::GetDebugString_Implementation()
{
	return FString::Printf(TEXT("Current Reward: %f"), CurrentReward);;
}

#pragma endregion IMLRewarderInterface