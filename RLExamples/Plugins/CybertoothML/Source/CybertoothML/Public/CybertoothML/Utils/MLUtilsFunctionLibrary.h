// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MLUtilsFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class CYBERTOOTHML_API UMLUtilsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Game", meta = (WorldContext = "WorldContextObject"))
	static void SetPlayerScore(APlayerState* PlayerState, float Score);

	UFUNCTION(BlueprintCallable, Category = "Game")
	static FString GenerateRandomAgentName();

	UFUNCTION(BlueprintCallable, Category = "Game")
	static TSubclassOf<AActor> GetActorFromString(const FString InName);

	static bool ConvertFStringToTraceType(const FString& TraceTypeName, TEnumAsByte<ETraceTypeQuery>& OutType);

	static bool ExtractFColor(const TSharedPtr<FJsonObject> JsonObject, FColor& OutColor);

	UFUNCTION(BlueprintCallable, Category = "Game")
	static USceneComponent* FindComponentByName(AActor* Actor, const FString& ComponentName);

	UFUNCTION(BlueprintCallable, Category = "Game", meta = (WorldContext = "WorldContextObject"))
	static AActor* GetActorByName(UObject* WorldContextObject, const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Game")
	static UClass* TryGetUClassFromPath(FString InPath);

	UFUNCTION(BlueprintCallable, Category = "Game", meta = (WorldContext = "WorldContextObject"))
	static FTransform GetStartingTransform(UObject* WorldContextObject, FString AgentName);		
	
	UFUNCTION(BlueprintCallable, Category = "Game")
	static void ConvertToGrayscale(const TArray<FColor>& Colors, TArray<uint8>& OutGrayscale);

	UFUNCTION(BlueprintCallable, Category = "Game")
	static float CalculateDistanceAlongAxis(const FVector& PositionA, const FVector& PositionB, const FVector& Axis);


};