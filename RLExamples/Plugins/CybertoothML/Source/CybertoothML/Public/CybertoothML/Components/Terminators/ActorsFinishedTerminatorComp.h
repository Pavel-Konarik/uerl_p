// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/Components/MLTerminatorInterface.h"
#include "ActorsFinishedTerminatorComp.generated.h"

USTRUCT()
struct FActorsFinishedTerminatorConfig
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString actors_class_to_check;

	FActorsFinishedTerminatorConfig()
	{
		actors_class_to_check = "";
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API UActorsFinishedTerminatorComp : public UActorComponent, public IMLTerminatorInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActorsFinishedTerminatorComp();

	UPROPERTY(BlueprintReadOnly, Category = "Defaults")
	UClass* ActorClassToCheck;

	UPROPERTY(BlueprintReadOnly, Category = "Defaults")
	int32 AllActorsCount;

	UPROPERTY(BlueprintReadOnly, Category = "Defaults")
	int32 FinishedActorsCount;

#pragma region TerminatorInterface
	virtual FString GetMLName_Implementation() override;
	virtual void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	virtual bool IsTerminated_Implementation() override;
	virtual bool IsTruncated_Implementation() override;
	virtual void Reset_Implementation() override;
	virtual FString GetDebugString_Implementation() override;
#pragma endregion TerminatorInterface


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
	void UpdateHasFinished(bool ExitOnFirstNotFinished);
};
