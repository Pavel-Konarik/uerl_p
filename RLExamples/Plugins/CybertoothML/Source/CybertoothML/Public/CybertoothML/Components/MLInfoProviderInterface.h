// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CybertoothML/MLTypes.h"
#include "MLInfoProviderInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMLInfoProviderInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CYBERTOOTHML_API IMLInfoProviderInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	FString GetMLName();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	void Configure(const FJsonObjectWrapper& JsonConfig);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	void GetInfo(FSensorObservation& OutInfo);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	void Reset();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	void OnPostReset();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	FString GetDebugString();
};
