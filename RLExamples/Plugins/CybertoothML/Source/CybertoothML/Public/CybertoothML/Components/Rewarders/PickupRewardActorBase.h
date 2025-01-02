// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupRewardActorBase.generated.h"

UCLASS()
class CYBERTOOTHML_API APickupRewardActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupRewardActorBase();

	/** This ensures this reward tiggers only for a specific reward component. For example, if we have a reward component for blue apples and red apples,
	we can ensure that red apples trigger only for red apples component. In Python this would be defined like so rewarder_conf = PickupRewarderConfig(name="red_apples_reward") */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Reward")
	FString PickupComponentName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Reward")
	float RewardAmount;


	UFUNCTION(BlueprintCallable, Category="Reward")
	bool AttemptToPickup(AActor* Picker);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
