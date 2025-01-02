// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../MLRewarderInterface.h"
#include "ActorDirectionRewarderComp.generated.h"


UENUM()
enum class EActorDirection : uint8
{
	Forward = 0,
	Backwards = 1,
	Left = 2, 
	Right = 3
};

USTRUCT()
struct FActorDirectionRewarderConfig
{
    GENERATED_BODY()

    UPROPERTY()
    float reward_per_unit;

	UPROPERTY()
	EActorDirection direction;

	UPROPERTY()
	bool allow_opposite;

	FActorDirectionRewarderConfig()
	{
		reward_per_unit = 1.0f;
		direction = EActorDirection::Forward;
		allow_opposite = false;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API UActorDirectionRewarderComp : public UActorComponent, public IMLRewarderInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActorDirectionRewarderComp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	FVector LastLocation;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	float CurrentReward = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Defaults")
	float RewardPerWorldUnit = 0.03f;
	
	UPROPERTY(EditAnywhere, Category = "Defaults")
	EActorDirection RewardDirection = EActorDirection::Forward;
	
	UPROPERTY(EditAnywhere, Category = "Defaults")
	bool bAllowOpposite = false;
		

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
