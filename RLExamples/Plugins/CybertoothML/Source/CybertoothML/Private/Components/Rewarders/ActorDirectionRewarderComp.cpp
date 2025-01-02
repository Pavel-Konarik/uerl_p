// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Rewarders/ActorDirectionRewarderComp.h"
#include "Kismet/KismetMathLibrary.h"
#include "CybertoothML/Utils/MLUtilsFunctionLibrary.h"
#include "JsonObjectConverter.h"

// Sets default values for this component's properties
UActorDirectionRewarderComp::UActorDirectionRewarderComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
}


// Called when the game starts
void UActorDirectionRewarderComp::BeginPlay()
{
	Super::BeginPlay();
	
	LastLocation = GetOwner()->GetActorLocation();
}


// Called every frame
void UActorDirectionRewarderComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	AActor* Actor = GetOwner(); 
	FVector CurrentLocation = Actor->GetActorLocation(); 


	FVector TargetDirection;
	switch (RewardDirection)
	{
	case EActorDirection::Forward:
		TargetDirection = Actor->GetActorForwardVector();
		break;
	case EActorDirection::Backwards:
		TargetDirection = Actor->GetActorForwardVector() * -1.0f;
		break;
	case EActorDirection::Right:
		TargetDirection = Actor->GetActorRightVector();
		break;
	case EActorDirection::Left:
		TargetDirection = Actor->GetActorRightVector() * -1.0f;
		break;
	default:
		check(false && "Unknown EActorDirection received");
		break;
	}

	float DistanceAlongAxis = UMLUtilsFunctionLibrary::CalculateDistanceAlongAxis(LastLocation, CurrentLocation, TargetDirection);
	// If we are going in the opposite way, should we allow inverse reward?
	if (!bAllowOpposite && DistanceAlongAxis <= 0.0f)
	{
		DistanceAlongAxis = 0.0f;
	}
	CurrentReward = DistanceAlongAxis * RewardPerWorldUnit;

	LastLocation = CurrentLocation;
}


#pragma region IMLRewarderInterface

FString UActorDirectionRewarderComp::GetMLName_Implementation()
{
	return "actor_direction";
}


void UActorDirectionRewarderComp::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FActorDirectionRewarderConfig RewarderConfig;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &RewarderConfig))
	{
		RewardPerWorldUnit = RewarderConfig.reward_per_unit;
		RewardDirection = RewarderConfig.direction;
		bAllowOpposite = RewarderConfig.allow_opposite;
	}
}

float UActorDirectionRewarderComp::GetReward_Implementation()
{
	return CurrentReward;
}

float UActorDirectionRewarderComp::ConsumeReward_Implementation()
{
	float Reward = CurrentReward;
	CurrentReward = 0.0f;

	return Reward;
}

void UActorDirectionRewarderComp::Reset_Implementation()
{
	CurrentReward = 0.0f;
}

void UActorDirectionRewarderComp::OnPostReset_Implementation()
{
	// User is now in new location, so we just reset it to start counting from there
	LastLocation = GetOwner()->GetActorLocation();
}

FString UActorDirectionRewarderComp::GetDebugString_Implementation()
{
	return FString::Printf(TEXT("Current Reward: %f"), CurrentReward);;
}

#pragma endregion IMLRewarderInterface