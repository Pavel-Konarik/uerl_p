// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Terminators/OnHitTerminatorComp.h"

#include "JsonObjectConverter.h"

// Sets default values for this component's properties
UOnHitTerminatorComp::UOnHitTerminatorComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;

}


// Called every frame
void UOnHitTerminatorComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

FString UOnHitTerminatorComp::GetMLName_Implementation()
{
	return "on_hit_terminator";
}

void UOnHitTerminatorComp::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FOnHitTerminatorConfig StepCounterConfig;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &StepCounterConfig))
	{
	}

	// Get owner and attach the OnHit event (not OnTakeAnyDamage)
	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->OnActorHit.AddDynamic(this, &UOnHitTerminatorComp::OnHit);
	}
	
}

bool UOnHitTerminatorComp::IsTerminated_Implementation()
{
	return bIsTerminated;
}

bool UOnHitTerminatorComp::IsTruncated_Implementation()
{
	return false;
}

void UOnHitTerminatorComp::Reset_Implementation()
{
	bIsTerminated = false;
}

FString UOnHitTerminatorComp::GetDebugString_Implementation()
{
	return FString::Printf(TEXT("Was terminated: %s"), bIsTerminated ? TEXT("True") : TEXT("False"));
}

void UOnHitTerminatorComp::OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	bIsTerminated = true;
}