// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/MLTypes.h"
#include "MLAgentCoreComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API UMLAgentCoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMLAgentCoreComponent();


	void Configure(const FString InAgentName, const TSharedPtr<FJsonObject> AgentConfigJson);
	template<typename TUInterface, typename TIInterface> void CreateAndConfigureComponents(const FString KeyName, const TSharedPtr<FJsonObject> AgentConfigJson, TMap<FString, UActorComponent*>& ComponentsMap);

	UFUNCTION(BlueprintCallable, Category = "Agent")
	FString GetAgentName() const;

	UFUNCTION(BlueprintCallable, Category = "Agent")
	void SetAgentName(FString InAgentName);

	

private:
	UPROPERTY()
	FString AgentName;

	UPROPERTY()
	TMap<FString, UActorComponent*> Actuators;

	UPROPERTY()
	TMap<FString, UActorComponent*> Sensors;

	UPROPERTY()
	TMap<FString, UActorComponent*> Rewarders;

	UPROPERTY()
	TMap<FString, UActorComponent*> Terminators;

	UPROPERTY()
	TMap<FString, UActorComponent*> InfoProviders;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
public:
	// Called when the environment gets resetted
	UFUNCTION(BlueprintCallable, Category = "Reset")
	virtual FTransform GetNewStartingTransform();
	UFUNCTION(BlueprintCallable, Category="Reset")
	virtual void TeleportToStart();
	virtual void Reset();
	FTransform StartingTransform;

	// Called after the Reset has finished (usually to gather first observation)
	virtual void OnPostReset();

	virtual void PrepStepHuman();

	virtual void PrepStep(const FAgentActuators& AgentActions);

	
	void CollectObservations(FAgentObservations& AgentObservations);

	void CollectAndConsumeReward(FAgentRewards& AgentRewards);

	void CollectInfos(FAgentInfos& AgentInfos);

	void CollectTerminations(FAgentTerminations& AgentTerminations);

#pragma region Debug
	UFUNCTION(BlueprintCallable, Category="ML|Debug")
	FString GetDebugString();

	UFUNCTION(BlueprintCallable, Category = "ML|Debug")
	FString GetDebugString_Sensors();

	UFUNCTION(BlueprintCallable, Category = "ML|Debug")
	TArray<class UTexture*> GetDebugTextures_Sensors();

	UFUNCTION(BlueprintCallable, Category = "ML|Debug")
	FString GetDebugString_Actuators();

	UFUNCTION(BlueprintCallable, Category = "ML|Debug")
	FString GetDebugString_Rewarders();

	UFUNCTION(BlueprintCallable, Category = "ML|Debug")
	FString GetDebugString_Terminators();

	UFUNCTION(BlueprintCallable, Category = "ML|Debug")
	FString GetDebugString_Infos();
#pragma endregion Debug
};

template <typename InterfaceType>
FString GetFormattedComponentDebugStrings(const FString& Title, const TMap<FString, UActorComponent*>& Components)
{
	FString Result = Title + TEXT("\n");
	const FString Indent = TEXT("\t");

	for (const auto& Pair : Components)
	{
		FString ComponentKey = Pair.Key;
		UActorComponent* Component = Pair.Value;

		if (Component)
		{
			FString DebugString = InterfaceType::Execute_GetDebugString(Component);
			Result += Indent + ComponentKey + TEXT(": ") + DebugString + TEXT("\n");
		}
	}

	return Result;
}
