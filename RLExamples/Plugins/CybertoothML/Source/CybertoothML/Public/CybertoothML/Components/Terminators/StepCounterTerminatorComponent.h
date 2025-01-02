// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/Components/MLTerminatorInterface.h"
#include "StepCounterTerminatorComponent.generated.h"

USTRUCT()
struct FStepCounterTerminatorConfig
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 max_step_count;

	FStepCounterTerminatorConfig()
	{
		max_step_count = 2000;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API UStepCounterTerminatorComponent : public UActorComponent, public IMLTerminatorInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStepCounterTerminatorComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadWrite, Category = "Defaults")
	int32 CurrentStepCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	int32 MaxStepCount = 1000;

#pragma region TerminatorInterface
	virtual FString GetMLName_Implementation() override;
	virtual void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	virtual bool IsTerminated_Implementation() override;
	virtual bool IsTruncated_Implementation() override;
	virtual void Reset_Implementation() override;
	virtual FString GetDebugString_Implementation() override;
#pragma endregion TerminatorInterface

		
};
