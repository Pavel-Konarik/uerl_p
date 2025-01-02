// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MLTerminatorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMLTerminatorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CYBERTOOTHML_API IMLTerminatorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	FString GetMLName();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	void Configure(const FJsonObjectWrapper& JsonConfig);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	bool IsTerminated();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	bool IsTruncated();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	void Reset();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	void OnPostReset();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ML")
	FString GetDebugString();
};
