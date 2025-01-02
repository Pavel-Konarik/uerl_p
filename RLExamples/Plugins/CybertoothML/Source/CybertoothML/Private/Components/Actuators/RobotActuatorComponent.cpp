// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Actuators/RobotActuatorComponent.h"
#include "CybertoothML/Robot/RobotAgent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

// Sets default values for this component's properties
URobotActuatorComponent::URobotActuatorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
}


// Called when the game starts
void URobotActuatorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void URobotActuatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ARobotAgent* OwningCharacter = Cast<ARobotAgent>(GetOwner());
	check(OwningCharacter);
	UCharacterMovementComponent* MovementComp = OwningCharacter->GetCharacterMovement();
	check(MovementComp);
	
	if (ActionMoveForward != 0)
	{
		OwningCharacter->MoveForward(ActionMoveForward);
	}

	if (ActionTurnRight != 0)
	{
		//OwningCharacter->TurnAtRate(ActionTurnRight);
		FRotator DeltaRotator = FRotator(0.0f, ActionTurnRight * OwningCharacter->BaseTurnRate * GetWorld()->GetDeltaSeconds(), 0.0f);
		OwningCharacter->AddActorWorldRotation(DeltaRotator);
	}
	
	if (ActionJump == 1)
	{
		if (MovementComp->IsJumpAllowed())
		{
			OwningCharacter->Jump();
		}
	}
	else {
		OwningCharacter->StopJumping();
	}
	
}


#pragma region IMLActuatorInterface
FString URobotActuatorComponent::GetMLName_Implementation()
{
	return "robot_movement";
}

void URobotActuatorComponent::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{

}

void URobotActuatorComponent::PrepActions_Implementation(const FActuatorData& Actions)
{
	// // Make sure the source TArray has a size that is a multiple of sizeof(float)
	// check(Actions.Data.Num() % sizeof(int64) == 0);
	//
	// // This is a bit awkward, but we know that Actions are actually a list of floats. So we just say, "treat Actions as floats"
	// // This way is better than copying it into a new array
	// TArrayView<const int64> FloatActions(reinterpret_cast<const int64*>(Actions.Data.GetData()), Actions.Data.Num() / sizeof(int64));
	//
	// // Forward - Back, Right - Left
	// ActionMoveForward = FloatActions[0] - FloatActions[1];
	// ActionTurnRight = FloatActions[2] - FloatActions[3];
	// ActionJump = FloatActions[4];



	check(Actions.Data.Num() % sizeof(int32) == 0);
	TArrayView<const int32> ActionArray(reinterpret_cast<const int32*>(Actions.Data.GetData()), Actions.Data.Num() / sizeof(int32));
	ActionMoveForward = ActionArray[0] - ActionArray[1];
	ActionTurnRight = ActionArray[2] - ActionArray[3];
	ActionJump = ActionArray[4];
}

void URobotActuatorComponent::PrepActionsHumanInput_Implementation()
{
	// Get human PC
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!IsValid(PawnOwner))
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!IsValid(PC))
	{
		return;
	}

	float ForwardValue = PC->IsInputKeyDown(EKeys::W) ? 1.0f : 0.0f;
	float BackwardsValue = PC->IsInputKeyDown(EKeys::S) ? 1.0f : 0.0f;
	float RightValue = PC->IsInputKeyDown(EKeys::D) ? 1.0f : 0.0f;
	float LeftValue = PC->IsInputKeyDown(EKeys::A) ? 1.0f : 0.0f;
	float SpaceBarValue = PC->IsInputKeyDown(EKeys::SpaceBar) ? 1.0f : 0.0f;

	// Forward - Back, Right - Left
	ActionMoveForward = ForwardValue - BackwardsValue;
	ActionTurnRight = RightValue - LeftValue;
	ActionJump = SpaceBarValue;
}

void URobotActuatorComponent::Reset_Implementation()
{
	ActionMoveForward = 0;
	ActionTurnRight = 0;
	ActionJump = 0;
}

FString URobotActuatorComponent::GetDebugString_Implementation()
{
	return FString::Printf(TEXT("Move: %d, Turn: %d, Jump: %d"), ActionMoveForward, ActionTurnRight, ActionJump);
}

#pragma endregion IMLActuatorInterface

