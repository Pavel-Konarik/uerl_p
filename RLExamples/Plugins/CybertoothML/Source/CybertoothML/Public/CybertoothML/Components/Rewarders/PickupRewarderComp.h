// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/Components/MLRewarderInterface.h"
#include "PickupRewarderComp.generated.h"


USTRUCT()
struct FPickupRewarderConfig
{
    GENERATED_BODY()

    UPROPERTY()
    FString name;

	FPickupRewarderConfig()
	{
		name = "";
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API UPickupRewarderComp : public UActorComponent, public IMLRewarderInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPickupRewarderComp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	float CurrentReward = 0.0f;

	UPROPERTY()
	FString RewardInstanceName = "";

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
