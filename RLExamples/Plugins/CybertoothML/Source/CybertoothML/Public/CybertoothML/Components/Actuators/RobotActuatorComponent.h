// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/Components/MLActuatorInterface.h"
#include "RobotActuatorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API URobotActuatorComponent : public UActorComponent, public IMLActuatorInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URobotActuatorComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


#pragma region IMLActuatorInterface
	FString GetMLName_Implementation() override;
	void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	void PrepActions_Implementation(const FActuatorData& Actions) override;
	void PrepActionsHumanInput_Implementation() override;
	void Reset_Implementation() override;
	FString GetDebugString_Implementation() override;
#pragma endregion IMLActuatorInterface


	int32 ActionMoveForward = 0;
	int32 ActionTurnRight = 0;
	int32 ActionJump = 0;
};
