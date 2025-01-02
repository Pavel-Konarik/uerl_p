// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/Components/MLTerminatorInterface.h"
#include "OnHitTerminatorComp.generated.h"

USTRUCT()
struct FOnHitTerminatorConfig
{
	GENERATED_BODY()

public:

	FOnHitTerminatorConfig()
	{
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API UOnHitTerminatorComp : public UActorComponent, public IMLTerminatorInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOnHitTerminatorComp();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region TerminatorInterface
	virtual FString GetMLName_Implementation() override;
	virtual void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	virtual bool IsTerminated_Implementation() override;
	virtual bool IsTruncated_Implementation() override;
	virtual void Reset_Implementation() override;
	virtual FString GetDebugString_Implementation() override;
#pragma endregion TerminatorInterface
		
	UFUNCTION()
	void OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY()
	bool bIsTerminated = false;
};
