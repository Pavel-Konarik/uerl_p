// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Terminators/StepCounterTerminatorComponent.h"
#include "JsonObjectConverter.h"

UStepCounterTerminatorComponent::UStepCounterTerminatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
}


// Called every frame
void UStepCounterTerminatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Increase the counter
	CurrentStepCount = FMath::Min(CurrentStepCount + 1, MaxStepCount);
}

FString UStepCounterTerminatorComponent::GetMLName_Implementation()
{
	return "step_counter";
}

void UStepCounterTerminatorComponent::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FStepCounterTerminatorConfig StepCounterConfig;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &StepCounterConfig))
	{
		MaxStepCount = StepCounterConfig.max_step_count;
		CurrentStepCount = 0;
	}
}

bool UStepCounterTerminatorComponent::IsTerminated_Implementation()
{
	return false;
}

bool UStepCounterTerminatorComponent::IsTruncated_Implementation()
{
	return CurrentStepCount >= MaxStepCount;
}

void UStepCounterTerminatorComponent::Reset_Implementation()
{
	CurrentStepCount = 0;
}

FString UStepCounterTerminatorComponent::GetDebugString_Implementation()
{
	return FString::Printf(TEXT("Steps: %d/%d (%s)"), CurrentStepCount, MaxStepCount, (CurrentStepCount >= MaxStepCount) ? TEXT("done") : TEXT("not done"));
}

