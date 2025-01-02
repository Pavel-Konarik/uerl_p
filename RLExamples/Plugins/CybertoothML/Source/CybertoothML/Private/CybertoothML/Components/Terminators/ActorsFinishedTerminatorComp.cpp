// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Terminators/ActorsFinishedTerminatorComp.h"
#include "CybertoothML/Components/Terminators/CanBeFinishedInterface.h"
#include "EngineUtils.h"
#include "CybertoothML/RLManager.h"
#include "CybertoothML/Utils/MLUtilsFunctionLibrary.h"

// Sets default values for this component's properties
UActorsFinishedTerminatorComp::UActorsFinishedTerminatorComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;

	AllActorsCount = 0;
	FinishedActorsCount = 0;
}


// Called when the game starts
void UActorsFinishedTerminatorComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UActorsFinishedTerminatorComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	URLManager* RLManager = URLManager::Get(this);
	check(RLManager);

	// In Human mode we want to loop over all actors to generate valid debug string.
	// Otherwise it's more efficient to stop with the first one
	UpdateHasFinished(!RLManager->IsHumanModeEnabled());
}

void UActorsFinishedTerminatorComp::UpdateHasFinished(bool ExitOnFirstNotFinished)
{
	// Iterate over all actors of class UClass* ActorClassToCheck;
	// Check that each actor implements ICheckFinishedInterface
	// And get IsFinished from it. If any is not terminated return false

	AllActorsCount = 0;
	FinishedActorsCount = 0;

	// Ensure that ActorClassToCheck is valid
	if (!ActorClassToCheck)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActorClassToCheck is not set."));
		return;
	}
	
	

	// Iterate over all actors of class ActorClassToCheck
	for (TActorIterator<AActor> It(GetWorld(), ActorClassToCheck); It; ++It)
	{
		AActor* Actor = *It;

		AllActorsCount++;

		// Check that the actor implements ICheckFinishedInterface
		if (Actor && Actor->Implements<UCanBeFinishedInterface>())
		{
			bool bIsFinished = ICanBeFinishedInterface::Execute_IsFinished(Actor);
			if (bIsFinished)
			{
				FinishedActorsCount++;
			}
			else if(ExitOnFirstNotFinished && !bIsFinished)
			{
				return;
			}
		}
		else
		{
			// If the actor does not implement ICheckFinishedInterface, log a warning
			UE_LOG(LogTemp, Warning, TEXT("%s does not implement ICanBeFinishedInterface."), *Actor->GetName());
		}
	}
}

FString UActorsFinishedTerminatorComp::GetMLName_Implementation()
{
	return "actors_finished";
}

void UActorsFinishedTerminatorComp::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FActorsFinishedTerminatorConfig Config;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &Config))
	{
		// Find matching class reference
		ActorClassToCheck = UMLUtilsFunctionLibrary::TryGetUClassFromPath(*Config.actors_class_to_check);
		check(ActorClassToCheck);
	}
}



bool UActorsFinishedTerminatorComp::IsTerminated_Implementation()
{
	return FinishedActorsCount == AllActorsCount;
}

bool UActorsFinishedTerminatorComp::IsTruncated_Implementation()
{
	return false;
}

void UActorsFinishedTerminatorComp::Reset_Implementation()
{
	
}

FString UActorsFinishedTerminatorComp::GetDebugString_Implementation()
{
	return FString::Printf(TEXT("Finished: %d/%d (%s)"), FinishedActorsCount, AllActorsCount, (FinishedActorsCount >= AllActorsCount) ? TEXT("done") : TEXT("not done"));
}

