// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Rewarders/OnHitRewarderComp.h"

#include "JsonObjectConverter.h"

// Sets default values for this component's properties
UOnHitRewarderComp::UOnHitRewarderComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;

}


// Called when the game starts
void UOnHitRewarderComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UOnHitRewarderComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}



void UOnHitRewarderComp::OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	CurrentReward += OnHitReward;
}

#pragma region IMLRewarderInterface

FString UOnHitRewarderComp::GetMLName_Implementation()
{
	return "on_hit_rewarder";
}


void UOnHitRewarderComp::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FOnHitRewarderConfig RewarderConfig;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &RewarderConfig))
	{
	}

	// Get owner and attach the OnHit event (not OnTakeAnyDamage)
	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->OnActorHit.AddDynamic(this, &UOnHitRewarderComp::OnHit);
	}
}


float UOnHitRewarderComp::GetReward_Implementation()
{
	return CurrentReward;
}

float UOnHitRewarderComp::ConsumeReward_Implementation()
{
	float Reward = CurrentReward;
	CurrentReward = 0.0f;

	return Reward;
}

void UOnHitRewarderComp::Reset_Implementation()
{
	CurrentReward = 0.0f;
}

void UOnHitRewarderComp::OnPostReset_Implementation()
{

}

FString UOnHitRewarderComp::GetDebugString_Implementation()
{
	return FString::Printf(TEXT("Current Reward: %f"), CurrentReward);;
}

#pragma endregion IMLRewarderInterface
