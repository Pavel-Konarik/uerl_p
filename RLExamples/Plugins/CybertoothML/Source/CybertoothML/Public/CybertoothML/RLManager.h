// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CybertoothML/Server/ServerWebSocketSubsystem.h"
#include "CybertoothML/Components/MLSensorInterface.h"
#include "CybertoothML/Components/MLActuatorInterface.h"
#include "CybertoothML/Components/MLTerminatorInterface.h"
#include "CybertoothML/Components/MLInfoProviderInterface.h"
#include "CybertoothML/Components/MLRewarderInterface.h"
#include "MLTypes.h"
#include "RLManager.generated.h"

UENUM()
enum class ERLStepState : uint8
{
	WaitingForCommand = 0,
	PerformingTick = 1,
	TickFinished = 2,
};

USTRUCT(BlueprintType)
struct FActorTransformData
{
    GENERATED_BODY()

    UPROPERTY()
    AActor* Actor;

    UPROPERTY()
    FTransform OriginalTransform;
};

USTRUCT()
struct FTestUstruct
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SampleInt32;

	UPROPERTY()
	FVector MyVector;

	FTestUstruct()
	{
		SampleInt32 = 5;
		MyVector = FVector(1, 2, 3);
	}
};

class UMLAgentCoreComponent;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPreEpisodeResetDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPostEpisodeResetDelegate);

    

/**
 * The manager of the UERL (reinforcement learning) system.
 * A singleton instance is setup automatically during OnPostEngineInit if this plugin is included.
 */
UCLASS(Transient)
class CYBERTOOTHML_API URLManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	URLManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static URLManager* Get(UObject* WorldContext);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	UFUNCTION()
	void HumanStart(const TArray<FString>& Args);

	UFUNCTION()
	void DebugAgent(const TArray<FString>& Args);

	UPROPERTY()
	UMLAgentCoreComponent* DebuggedAgent;

	UFUNCTION(BlueprintCallable, Category="Debug")
	UMLAgentCoreComponent* GetDebuggedAgent();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	bool IsHumanModeEnabled() const;
	UUserWidget* CreateHumanModeWidgetInstance();

	UPROPERTY()
	UUserWidget* DebugWidgetInstance;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void OnPostWorldInit(UWorld* World, const UWorld::InitializationValues);

	void OnPostWorldInit();

	void OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaTime);
	void OnWorldTickEnd(UWorld* World, ELevelTick TickType, float DeltaTime);
	void SoftPauseGame();
	void SoftUnpauseGame();
	
	UPROPERTY()
	class APlayerState* PauserPS;

	


	/** At the beginning of the world tick, we should wait for a command (step/reset) from Server. But 
	this would also lock the Editor thread. We can have a soft world locking in editor to skip the update 
	for the game world, but still tick the editor. This is slightly less efficient than Hard locking, which
	is recommended in the actual production. */
	bool IsHardLockingWait();

	void CollectClasses();
	
	template<typename TInterface>
	UClass* GetClassByInterface(FString ComponentClassName) const;

	UClass* GetSensorClass(FString SensorName) const;
	UClass* GetActuatorClass(FString SensorName) const;
	UClass* GetRewarderClass(FString RewarderName) const;
	UClass* GetTerminatorClass(FString TerminatorName) const;
	UClass* GetInfoProviderClass(FString InfoProviderName) const;

	UPROPERTY()
	TMap<FString, UClass*> SensorsNamesMap;

	UPROPERTY()
	TMap<FString, UClass*> ActuatorsNamesMap;

	UPROPERTY()
	TMap<FString, UClass*> RewardersNamesMap;

	UPROPERTY()
	TMap<FString, UClass*> TerminatorsNamesMap;

	UPROPERTY()
	TMap<FString, UClass*> InfoProvidersNamesMap;


	void StopServer();

	void StartServer(uint16 Port, uint16 ServerThreads);

	void Configure(const TSharedPtr<FJsonObject>& JsonObject);

	void StopExperiment();

	// Create an instance of the delegate
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FPreEpisodeResetDelegate OnPreEpisodeReset;

	// Create an instance of the delegate
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FPostEpisodeResetDelegate OnPostEpisodeReset;

	void HandleResetRequest(FAction_ResetRequest& ResetData);
	void PrepHumanStep();
	void HandleStepRequest(FAction_StepRequest& StepRequest);


	bool bTickWorld = false;
	bool bTickFinished = false;


	void GatherObservations(TMap<FString, FAgentObservations>& Observations);
	void GatherRewards(TMap<FString, FAgentRewards>& Rewards);
	void GatherTerminators(TMap<FString, FAgentTerminations>& Terminators);
	/*void GatherTerminated(TMap<FString, bool>& Terminated);
	void GatherTruncated(TMap<FString, bool>& Truncated);*/
	void GatherInfos(TMap<FString, FAgentInfos>& Infos);


	// Store original transforms of actors with MLReset tag
	void StoreMLResetActorsTransforms();

	// Reset actors with MLReset tag to their original positions and reset physics
	void ResetMLResetActorsTransforms();

	
	ERLStepState StepState;
	void WaitForStep();
	void StartStep();
	void FinishStep();

	FTSTicker::FDelegateHandle DisableRenderingTicker;


	UPROPERTY()
	TArray<FActorTransformData> StoredTransforms;

#pragma region Agents

	UFUNCTION(BlueprintCallable, Category = "ML|Agents")
	bool RegisterAgent(AActor* Agent);

	UFUNCTION(BlueprintCallable, Category = "ML|Agents")
	void UnregisterAgent(FString AgentName);
	
	UFUNCTION(BlueprintCallable, Category="ML|Agents")
	AActor* GetAgent(FString AgentName);

	UFUNCTION(BlueprintCallable, Category = "ML|Agents")
	UMLAgentCoreComponent* GetAgentCoreComp(FString AgentName);

	UFUNCTION()
	const TMap<FString, UMLAgentCoreComponent*>& GetAgents() const;


private:
	/** Most of our interactions will be with the agent core component, so we store that. */
	UPROPERTY()
	TMap<FString, UMLAgentCoreComponent*> Agents;

#pragma endregion Agents

};

template<typename TInterface>
UClass* URLManager::GetClassByInterface(FString ComponentClassName) const
{
	if (TInterface::StaticClass() == UMLSensorInterface::StaticClass())
	{
		return GetSensorClass(ComponentClassName);
	}

	if (TInterface::StaticClass() == UMLActuatorInterface::StaticClass())
	{
		return GetActuatorClass(ComponentClassName);
	}

	if (TInterface::StaticClass() == UMLRewarderInterface::StaticClass())
	{
		return GetRewarderClass(ComponentClassName);
	}

	if (TInterface::StaticClass() == UMLTerminatorInterface::StaticClass())
	{
		return GetTerminatorClass(ComponentClassName);
	}

	if (TInterface::StaticClass() == UMLInfoProviderInterface::StaticClass())
	{
		return GetInfoProviderClass(ComponentClassName);
	}

	return nullptr;
}

