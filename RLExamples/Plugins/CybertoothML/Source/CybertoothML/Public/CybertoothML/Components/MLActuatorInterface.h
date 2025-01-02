// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CybertoothML/MLTypes.h"
#include "MLActuatorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMLActuatorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CYBERTOOTHML_API IMLActuatorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	FString GetMLName();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	void Configure(const FJsonObjectWrapper& JsonConfig);

	/* Prep actions before tick, so the correct action can be performed during component tick function */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	void PrepActions(const FActuatorData& Actions);

	/* Same functionality as PrepActions, but it should use human player inputs. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	void PrepActionsHumanInput();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	void Reset();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	void OnPostReset();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	FString GetDebugString();
};
