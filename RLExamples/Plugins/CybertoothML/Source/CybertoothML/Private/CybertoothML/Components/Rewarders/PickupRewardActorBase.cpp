// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Rewarders/PickupRewardActorBase.h"
#include "CybertoothML/Components/Rewarders/PickupRewarderComp.h"

// Sets default values
APickupRewardActorBase::APickupRewardActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PickupComponentName = "";
    RewardAmount = 10.0f;
}

bool APickupRewardActorBase::AttemptToPickup(AActor* Picker)
{
    if (!IsValid(Picker)) // Check if Picker is valid
    {
        return false;
    }

    // Attempt to find the PickupRewarderComp component on the Picker actor
    UPickupRewarderComp* RewarderComp = Cast<UPickupRewarderComp>(Picker->GetComponentByClass(UPickupRewarderComp::StaticClass()));

    if (RewarderComp && RewarderComp->RewardInstanceName == PickupComponentName) // Check if the component exists and matches the required name
    {
        RewarderComp->CurrentReward += 10;
        return true; 
    }

    return false;
}

// Called when the game starts or when spawned
void APickupRewardActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APickupRewardActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

