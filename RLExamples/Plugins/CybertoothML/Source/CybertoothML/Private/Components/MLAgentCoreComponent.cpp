// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/MLAgentCoreComponent.h"
#include "CybertoothML/Components/MLSensorInterface.h"
#include "CybertoothML/Components/MLActuatorInterface.h"
#include "CybertoothML/Components/MLRewarderInterface.h"
#include "CybertoothML/Components/MLInfoProviderInterface.h"
#include "CybertoothML/Components/MLTerminatorInterface.h"

#include "CybertoothML/RLManager.h"
#include "CybertoothML/Utils/MLUtilsFunctionLibrary.h"
#include "CybertoothML/MLTypes.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"

// Sets default values for this component's properties
UMLAgentCoreComponent::UMLAgentCoreComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	AgentName = "UnsetName";
}


void UMLAgentCoreComponent::Configure(const FString InAgentName, const TSharedPtr<FJsonObject> AgentConfigJson)
{
	SetAgentName(InAgentName);

	// Spawn and configure components

	// Configure Sensors
	CreateAndConfigureComponents<UMLSensorInterface, IMLSensorInterface>("sensor_configs", AgentConfigJson, Sensors);
	// Configure Actuators
	CreateAndConfigureComponents<UMLActuatorInterface, IMLActuatorInterface>("actuator_configs", AgentConfigJson, Actuators);
	// Configure Rewarders
	CreateAndConfigureComponents<UMLRewarderInterface, IMLRewarderInterface>("rewarder_configs", AgentConfigJson, Rewarders);
	// Configure Terminators
	CreateAndConfigureComponents<UMLTerminatorInterface, IMLTerminatorInterface>("terminator_configs", AgentConfigJson, Terminators);
	// Configure Info Providers
	CreateAndConfigureComponents<UMLInfoProviderInterface, IMLInfoProviderInterface>("infos_configs", AgentConfigJson, InfoProviders);
}

template<typename TUInterface, typename TIInterface>
void UMLAgentCoreComponent::CreateAndConfigureComponents(const FString KeyName, const TSharedPtr<FJsonObject> AgentConfigJson, TMap<FString, UActorComponent*>& ComponentsMap)
{
	// Construct actuators for this agent
	const TArray<TSharedPtr<FJsonValue>>* ConfigJson;
	bool bConfigFound = AgentConfigJson->TryGetArrayField(KeyName, ConfigJson);
	if (bConfigFound)
	{
		URLManager* RLManager = URLManager::Get(this);
		check(RLManager);

		for (TSharedPtr<FJsonValue> ComponentValueData : *ConfigJson)
		{
			TSharedPtr<FJsonObject> ComponentData = ComponentValueData->AsObject();

			// Get unique component name
			FString ComponentName;
			check(ComponentData->TryGetStringField(TEXT("name"), ComponentName));

			// Get the class name to construct the component from
			FString ComponentClassName;
			check(ComponentData->TryGetStringField(TEXT("ue_class_name"), ComponentClassName));

			// Get the actual class
			UClass* ComponentClass = RLManager->GetClassByInterface<TUInterface>(ComponentClassName);
			check(ComponentClass);

			// Spawn the component
			UActorComponent* ComponentComp = NewObject<UActorComponent>(GetOwner(), ComponentClass);
			check(ComponentComp);
			ComponentComp->RegisterComponent();

			// This component MUST implement the specified interface
			check(ComponentComp->GetClass()->ImplementsInterface(TUInterface::StaticClass()));

			// To be able to use this config in Blueprints, we wrap it in blueprint wrapper
			FJsonObjectWrapper ComponentConfigWrapper;
			ComponentConfigWrapper.JsonObject = ComponentData;

			// Actually configure the component
			TIInterface::Execute_Configure(ComponentComp, ComponentConfigWrapper);

			// Keep track of this component
			ComponentsMap.Add(ComponentName, ComponentComp);
		}
	}
}

FString UMLAgentCoreComponent::GetAgentName() const
{
	return AgentName;
}

void UMLAgentCoreComponent::SetAgentName(FString InAgentName)
{
	AgentName = InAgentName;
}

// Called when the game starts
void UMLAgentCoreComponent::BeginPlay()
{
	Super::BeginPlay();

	StartingTransform = GetOwner()->GetActorTransform();
}


// Called every frame
void UMLAgentCoreComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}



FTransform UMLAgentCoreComponent::GetNewStartingTransform()
{
	return UMLUtilsFunctionLibrary::GetStartingTransform(this, AgentName);
}

void UMLAgentCoreComponent::TeleportToStart()
{
	// Move actor to "starting" position
	GetOwner()->SetActorTransform(StartingTransform, false, nullptr, ETeleportType::ResetPhysics);
}

void UMLAgentCoreComponent::Reset()
{
	StartingTransform = GetNewStartingTransform();
	TeleportToStart();

	// Reset Sensors
	for (const TPair<FString, UActorComponent*>& SensorPair : Sensors)
	{
		UActorComponent* Sensor = SensorPair.Value;
		IMLSensorInterface::Execute_Reset(Sensor);
	}
	
	// Reset Actuators
	for (const TPair<FString, UActorComponent*>& ActuatorPair : Actuators)
	{
		UActorComponent* Actuator = ActuatorPair.Value;
		IMLActuatorInterface::Execute_Reset(Actuator);
	}
	// Reset Rewarders
	for (const TPair<FString, UActorComponent*>& RewarderPair : Rewarders)
	{
		UActorComponent* Rewarder = RewarderPair.Value;
		IMLRewarderInterface::Execute_Reset(Rewarder);
	}
	// Reset Terminators
	for (const TPair<FString, UActorComponent*>& TerminatorPair : Terminators)
	{
		UActorComponent* Terminator = TerminatorPair.Value;
		IMLTerminatorInterface::Execute_Reset(Terminator);
	}
	// Reset Actuators
	for (const TPair<FString, UActorComponent*>& ActuatorPair : Actuators)
	{
		UActorComponent* Actuator = ActuatorPair.Value;
		IMLActuatorInterface::Execute_Reset(Actuator);
	}
	// Reset InfoProviders
	for (const TPair<FString, UActorComponent*>& InfoPair : InfoProviders)
	{
		UActorComponent* InfoProvider = InfoPair.Value;
		IMLInfoProviderInterface::Execute_Reset(InfoProvider);
	}
}

void UMLAgentCoreComponent::OnPostReset()
{
	// Reset Sensors
	for (const TPair<FString, UActorComponent*>& SensorPair : Sensors)
	{
		UActorComponent* Sensor = SensorPair.Value;
		IMLSensorInterface::Execute_OnPostReset(Sensor);
	}

	// Reset Actuators
	for (const TPair<FString, UActorComponent*>& ActuatorPair : Actuators)
	{
		UActorComponent* Actuator = ActuatorPair.Value;
		IMLActuatorInterface::Execute_OnPostReset(Actuator);
	}
	// Reset Rewarders
	for (const TPair<FString, UActorComponent*>& RewarderPair : Rewarders)
	{
		UActorComponent* Rewarder = RewarderPair.Value;
		IMLRewarderInterface::Execute_OnPostReset(Rewarder);
	}
	// Reset Terminators
	for (const TPair<FString, UActorComponent*>& TerminatorPair : Terminators)
	{
		UActorComponent* Terminator = TerminatorPair.Value;
		IMLTerminatorInterface::Execute_OnPostReset(Terminator);
	}
	// Reset Actuators
	for (const TPair<FString, UActorComponent*>& ActuatorPair : Actuators)
	{
		UActorComponent* Actuator = ActuatorPair.Value;
		IMLActuatorInterface::Execute_OnPostReset(Actuator);
	}
	// Reset InfoProviders
	for (const TPair<FString, UActorComponent*>& InfoPair : InfoProviders)
	{
		UActorComponent* InfoProvider = InfoPair.Value;
		IMLInfoProviderInterface::Execute_OnPostReset(InfoProvider);
	}
}

void UMLAgentCoreComponent::PrepStepHuman()
{
	for (const auto& AgentActionPair : Actuators) {
		const FString& ComponentName = AgentActionPair.Key;
		UActorComponent* Actuator = AgentActionPair.Value;

		IMLActuatorInterface::Execute_PrepActionsHumanInput(Actuator);
	}
}

void UMLAgentCoreComponent::PrepStep(const FAgentActuators& AgentActions)
{
	for (const auto& AgentActionPair : AgentActions.Actuators) {
		const FString& ComponentName = AgentActionPair.Key;
		const FActuatorData& ActionValues = AgentActionPair.Value;

		UActorComponent* Actuator = Actuators.FindChecked(ComponentName);
		IMLActuatorInterface::Execute_PrepActions(Actuator, ActionValues);
	}
}

void UMLAgentCoreComponent::CollectObservations(FAgentObservations& AgentObservations)
{
	for (const TPair<FString, UActorComponent*>& SensorPair : Sensors)
	{
		FString SensorName = SensorPair.Key;
		UActorComponent* Sensor = SensorPair.Value;

		FSensorObservation SensorObservation;
		IMLSensorInterface::Execute_GetObservation(Sensor, SensorObservation);

		AgentObservations.Sensors.Add(SensorName, SensorObservation);
	}
}

void UMLAgentCoreComponent::CollectAndConsumeReward(FAgentRewards& AgentRewards)
{
	for (const TPair<FString, UActorComponent*>& RewarderPair : Rewarders)
	{
		FString RewarderName = RewarderPair.Key;
		UActorComponent* Rewarder = RewarderPair.Value;

		float RewarderReward = IMLRewarderInterface::Execute_ConsumeReward(Rewarder);
		AgentRewards.Rewarders.Add(RewarderName, FRewarderStepOutcome(RewarderReward));
	}
}

void UMLAgentCoreComponent::CollectInfos(FAgentInfos& AgentInfos)
{
	for (const TPair<FString, UActorComponent*>& InfoPair : InfoProviders)
	{
		FString InfoProviderName = InfoPair.Key;
		UActorComponent* InfoProvider = InfoPair.Value;

		FSensorObservation InfoData;
		IMLInfoProviderInterface::Execute_GetInfo(InfoProvider, InfoData);

		AgentInfos.InfoProviders.Add(InfoProviderName, InfoData);
	}
}

void UMLAgentCoreComponent::CollectTerminations(FAgentTerminations& AgentTerminations)
{
	for (const TPair<FString, UActorComponent*>& InfoPair : Terminators)
	{
		FString TerminatorName = InfoPair.Key;
		UActorComponent* Terminator = InfoPair.Value;

		bool bTerminated = IMLTerminatorInterface::Execute_IsTerminated(Terminator);
		bool bTruncated = IMLTerminatorInterface::Execute_IsTruncated(Terminator);

		AgentTerminations.Terminators.Add(TerminatorName, FTerminatorStepOutcome(bTerminated, bTruncated));
	}
}
//
//bool UMLAgentCoreComponent::CollectTruncated()
//{
//	for (const TPair<FString, UActorComponent*>& InfoPair : Terminators)
//	{
//		UActorComponent* Terminator = InfoPair.Value;
//
//		bool bTerminated = IMLTerminatorInterface::Execute_IsTruncated(Terminator);
//		if (bTerminated)
//		{
//			return true;
//		}
//	}
//	return false;
//}

#pragma region Debug
FString UMLAgentCoreComponent::GetDebugString()
{
	FString Result;

	Result += GetDebugString_Sensors() + TEXT("\n");
	Result += GetDebugString_Actuators() + TEXT("\n");
	Result += GetDebugString_Rewarders() + TEXT("\n");
	Result += GetDebugString_Terminators() + TEXT("\n");
	Result += GetDebugString_Infos() + TEXT("\n");

	return Result;
}

FString UMLAgentCoreComponent::GetDebugString_Sensors()
{
	return GetFormattedComponentDebugStrings<IMLSensorInterface>(TEXT("Sensors"), Sensors);
}

TArray<UTexture*> UMLAgentCoreComponent::GetDebugTextures_Sensors()
{
	TArray<UTexture*> DebugTextures;

	for (const auto& Pair : Sensors)
	{
		FString ComponentKey = Pair.Key;
		UActorComponent* Component = Pair.Value;
		if (Component)
		{
			UTexture* Texture = IMLSensorInterface::Execute_GetDebugTexture(Component);
			if (Texture)
			{
				DebugTextures.Add(Texture);
			}
		}
	}
	return DebugTextures;
}

FString UMLAgentCoreComponent::GetDebugString_Actuators()
{
	return GetFormattedComponentDebugStrings<IMLActuatorInterface>(TEXT("Actuators"), Actuators);
}

FString UMLAgentCoreComponent::GetDebugString_Rewarders()
{
	return GetFormattedComponentDebugStrings<IMLRewarderInterface>(TEXT("Rewarders"), Rewarders);
}

FString UMLAgentCoreComponent::GetDebugString_Terminators()
{
	return GetFormattedComponentDebugStrings<IMLTerminatorInterface>(TEXT("Terminators"), Terminators);
}

FString UMLAgentCoreComponent::GetDebugString_Infos()
{
	return GetFormattedComponentDebugStrings<IMLInfoProviderInterface>(TEXT("InfoProviders"), InfoProviders);
}
#pragma endregion Debug
