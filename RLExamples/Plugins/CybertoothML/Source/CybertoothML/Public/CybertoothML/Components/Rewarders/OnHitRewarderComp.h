// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/Components/MLRewarderInterface.h"
#include "OnHitRewarderComp.generated.h"


USTRUCT()
struct FOnHitRewarderConfig
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float on_hit_reward;

	FOnHitRewarderConfig()
	{
		on_hit_reward = -10.0f;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API UOnHitRewarderComp : public UActorComponent, public IMLRewarderInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOnHitRewarderComp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY()
	float CurrentReward = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Defaults")
	float OnHitReward = -10.0f;


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
