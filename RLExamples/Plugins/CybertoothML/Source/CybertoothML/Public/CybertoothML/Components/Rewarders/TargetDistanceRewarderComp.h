// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/Components/MLRewarderInterface.h"
#include "TargetDistanceRewarderComp.generated.h"

USTRUCT()
struct FTargetDistanceRewarderConfig
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVector target_location;

	FTargetDistanceRewarderConfig()
	{
		target_location = FVector(0.0f, 0.0f, 0.0f);
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API UTargetDistanceRewarderComp : public UActorComponent, public IMLRewarderInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTargetDistanceRewarderComp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	FVector LastLocation;

	UPROPERTY()
	float CurrentReward = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Defaults")
	float RewardPerWorldUnit = 0.03f;

	UPROPERTY()
	FVector TargetLocation = FVector(0.0f, 0.0f, 0.0f);

	// Debug
	UPROPERTY()
	float LastReward_Debug = 0.0f;

	UPROPERTY()
	float TotalReward_Debug = 0.0f;
		
#pragma region IMLRewarderInterface
	FString GetMLName_Implementation() override;
	void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	float GetReward_Implementation() override;
	float ConsumeReward_Implementation() override;
	void Reset_Implementation() override;
	void OnPostReset_Implementation() override;
	FString GetDebugString_Implementation() override;
#pragma endregion IMLRewarderInterface
};
