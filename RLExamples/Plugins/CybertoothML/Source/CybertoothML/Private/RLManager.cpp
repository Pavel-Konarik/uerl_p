// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/RLManager.h"
#include "CybertoothML/Utils/MLUtilsFunctionLibrary.h"
#include "CybertoothML/Components/MLAgentCoreComponent.h"
#include "CybertoothML/GymEnvironmentTimeStep.h"

#include "CybertoothML/Components/MLSensorInterface.h"
#include "CybertoothML/Components/MLActuatorInterface.h"
#include "CybertoothML/Components/MLRewarderInterface.h"
#include "CybertoothML/Components/MLTerminatorInterface.h"
#include "CybertoothML/Components/MLInfoProviderInterface.h"

#include "CybertoothML/Utils/AgentStart.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/BufferArchive.h"
#include "JsonObjectConverter.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "HAL/IConsoleManager.h"
#include "../Public/CybertoothML/CybertoothMLSettings.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformApplicationMisc.h"
#include "InputCoreTypes.h"
#include "GameFramework/InputSettings.h"


int32 bForceHardLockStep = 0;
FAutoConsoleVariableRef CVarForceHardLockStep(
	TEXT("ML.bForceHardLockStep"),
	bForceHardLockStep, TEXT("If we should lock every world tick."));


int32 bEnableRLMangerStep = 1;
FAutoConsoleVariableRef CVarEnableRLMangerStep(
	TEXT("ML.bEnableRLMangerStep"),
	bEnableRLMangerStep, TEXT("1 enables to play the game."));


int32 bHumanMode = 0;
FAutoConsoleVariableRef CVarHumanMode(
	TEXT("ML.bHumanMode"),
	bHumanMode, TEXT("1 enables to play the game."));


URLManager::URLManager(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{

}

URLManager* URLManager::Get(UObject* WorldContext)
{
	return UGameplayStatics::GetGameInstance(WorldContext)->GetSubsystem<URLManager>();
}

bool URLManager::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void URLManager::HumanStart(const TArray<FString>& Args)
{
	FString InputString = "";

	if (Args.Num() == 1)
	{
		InputString = Args[0];
	}
	else {
		// Iterate through each element in Args and concatenate it to InputString
		for (const FString& Arg : Args) {
			InputString += Arg;
			// Optionally, add a space or some delimiter between arguments
			InputString += TEXT(" "); // Remove or replace this line if you don't want spaces
		}

		// Optionally, remove the last added space (if any)
		if (!InputString.IsEmpty()) {
			InputString.RemoveAt(InputString.Len() - 1);
		}
	}


	// Parse the input string as a JSON object
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(InputString);

	if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
	{
		// Your custom function logic here
		UE_LOG(LogTemp, Log, TEXT("HumanStart function called with input: %s"), *InputString);
		Configure(JsonObject);

		// Create a debug widget to visualise what agent experiences
		CreateHumanModeWidgetInstance();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse the input string as a JSON object"));
	}
}


void URLManager::DebugAgent(const TArray<FString>& Args)
{
	if (Args.Num() != 1)
	{
		UE_LOG(LogTemp, Error, TEXT("You need to provide a valid name of agent to specatate."));
		return;
	}

	const FString AgentID = Args[0];
	DebuggedAgent = GetAgentCoreComp(AgentID);

	if (DebuggedAgent == nullptr || AgentID.Equals("None", ESearchCase::IgnoreCase))
	{
		if (DebugWidgetInstance)
		{
			DebugWidgetInstance->RemoveFromParent();
		}
		DebuggedAgent = nullptr;
	}
	else {
		// Create a debug widget to visualise what agent experiences
		CreateHumanModeWidgetInstance();
	}
}

UMLAgentCoreComponent* URLManager::GetDebuggedAgent()
{
	return DebuggedAgent;
}

bool URLManager::IsHumanModeEnabled() const
{
	return bHumanMode >= 1;
}


UUserWidget* URLManager::CreateHumanModeWidgetInstance()
{
	const UCybertoothMLSettings* CoreSettings = GetDefault<UCybertoothMLSettings>();

	// Get the first player controller
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get the first player controller."));
		return nullptr;
	}

	// Load the widget class asynchronously
	UClass* WidgetClass = CoreSettings->HumanModeWidget.LoadSynchronous();
	if (!WidgetClass || !WidgetClass->IsChildOf(UUserWidget::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load the HumanModeWidget class."));
		return nullptr;
	}

	// Create the widget instance
	DebugWidgetInstance = CreateWidget<UUserWidget>(PlayerController, WidgetClass);
	if (!DebugWidgetInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create the HumanModeWidget instance."));
		return nullptr;
	}

	// Add the widget to the viewport
	DebugWidgetInstance->AddToViewport();

	return DebugWidgetInstance;
}

void URLManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (IsHardLockingWait() == false)
	{
		UWorld* World = GetWorld();
		if (World != nullptr && World->bIsWorldInitialized)
		{
			OnPostWorldInit();
		}
		else
		{
			FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &URLManager::OnPostWorldInit);
		}

		// To pause the world in a non-blocking way, we need a dummy PS
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PauserPS = GetWorld()->SpawnActor<APlayerState>(APlayerState::StaticClass(), SpawnParams);
	}
	

	FWorldDelegates::OnWorldTickStart.AddUObject(this, &URLManager::OnWorldTickStart);
	FWorldDelegates::OnWorldTickEnd.AddUObject(this, &URLManager::OnWorldTickEnd);
	
	
	// Register the console command
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("ML.HumanStart"),
		TEXT("Triggers the HumanStart function with a string input"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &URLManager::HumanStart),
		ECVF_Default
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("ML.DebugAgent"),
		TEXT("Starts viewing debug info of a given agent"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &URLManager::DebugAgent),
		ECVF_Default
	);



	// Disable rendering
	//if (false) {
	//	DisableRenderingTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([=](float Time)
	//	{
	//		if (GEngine && GEngine->GameViewport)
	//		{
	//			GEngine->GameViewport->bDisableWorldRendering = true;
	//			FTSTicker::GetCoreTicker().RemoveTicker(DisableRenderingTicker);
	//		}
	//		return true;
	//	}));
	//}

	CollectClasses();

	// StartServer(35112, 1);

}



void URLManager::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(DisableRenderingTicker);

	Super::Deinitialize();

	StopServer();
}

void URLManager::OnPostWorldInit(UWorld* World, const UWorld::InitializationValues)
{
	OnPostWorldInit();
}

void URLManager::OnPostWorldInit()
{
	// Your code to be executed when a new map is loaded
	StoreMLResetActorsTransforms();
}



void URLManager::OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaTime)
{
	if (World != nullptr)
	{
		if (World->WorldType == EWorldType::Editor)
		{
			// If it's an editor world, just return.
			return;
		}
	}


	if (IsHumanModeEnabled())
	{
		// Iterate over a all agents and trigger human (debug) actuators
		PrepHumanStep();
	}
	else {
		if (bEnableRLMangerStep)
		{
			if (IsHardLockingWait())
			{
				WaitForStep();
			}
			else
			{
				UServerWebSocketSubsystem* WebsocketServer = GetWorld()->GetGameInstance()->GetSubsystem<UServerWebSocketSubsystem>();
				WebsocketServer->Tick();

				if (StepState == ERLStepState::PerformingTick)
				{
					// Unpause the game for this step
					SoftUnpauseGame();
				}
				else {
					// Ensure the game is paused
					SoftPauseGame();
				}
			}
		}
	}

	

}



void URLManager::OnWorldTickEnd(UWorld* World, ELevelTick TickType, float DeltaTime)
{
	FinishStep();
}

void URLManager::SoftPauseGame()
{
	GetWorld()->GetWorldSettings()->SetPauserPlayerState(PauserPS);
}

void URLManager::SoftUnpauseGame()
{
	GetWorld()->GetWorldSettings()->SetPauserPlayerState(nullptr);
}


bool URLManager::IsHardLockingWait()
{
	if (bForceHardLockStep)
	{
		return true;
	}

#if WITH_EDITOR
	return false;
#else
	return true;
#endif
}

void URLManager::CollectClasses()
{
	// Collect all sensors
	{
		for (TObjectIterator<UClass> It; It; ++It)
		{
			UClass* CurrentClass = *It;
			if (CurrentClass->ImplementsInterface(UMLSensorInterface::StaticClass()))
			{
				UObject* CDO = CurrentClass->GetDefaultObject<UObject>();
				check(CDO);
				FString SensorName = IMLSensorInterface::Execute_GetMLName(CDO);
				// Ensure that each sensor has an unique name
				check(SensorsNamesMap.Contains(SensorName) == false);
				SensorsNamesMap.Add(SensorName, CurrentClass);
			}
		}
	}

	// Collect all Actuators
	{
		for (TObjectIterator<UClass> It; It; ++It)
		{
			UClass* CurrentClass = *It;
			if (CurrentClass->ImplementsInterface(UMLActuatorInterface::StaticClass()))
			{
				UObject* CDO = CurrentClass->GetDefaultObject<UObject>();
				check(CDO);
				FString ActuatorName = IMLActuatorInterface::Execute_GetMLName(CDO);
				// Ensure that each Actuator has an unique name
				check(ActuatorsNamesMap.Contains(ActuatorName) == false);
				ActuatorsNamesMap.Add(ActuatorName, CurrentClass);
			}
		}
	}

	// Collect all Rewarders
	{
		for (TObjectIterator<UClass> It; It; ++It)
		{
			UClass* CurrentClass = *It;
			if (CurrentClass->ImplementsInterface(UMLRewarderInterface::StaticClass()))
			{
				UObject* CDO = CurrentClass->GetDefaultObject<UObject>();
				check(CDO);
				FString RewarderName = IMLRewarderInterface::Execute_GetMLName(CDO);
				// Ensure that each Actuator has an unique name
				check(RewardersNamesMap.Contains(RewarderName) == false);
				RewardersNamesMap.Add(RewarderName, CurrentClass);
			}
		}
	}

	// Collect all Terminators
	{
		for (TObjectIterator<UClass> It; It; ++It)
		{
			UClass* CurrentClass = *It;
			if (CurrentClass->ImplementsInterface(UMLTerminatorInterface::StaticClass()))
			{
				UObject* CDO = CurrentClass->GetDefaultObject<UObject>();
				check(CDO);
				FString TerminatorName = IMLTerminatorInterface::Execute_GetMLName(CDO);
				check(TerminatorsNamesMap.Contains(TerminatorName) == false);
				TerminatorsNamesMap.Add(TerminatorName, CurrentClass);
			}
		}
	}

	// Collect all Info providers
	{
		for (TObjectIterator<UClass> It; It; ++It)
		{
			UClass* CurrentClass = *It;
			if (CurrentClass->ImplementsInterface(UMLInfoProviderInterface::StaticClass()))
			{
				UObject* CDO = CurrentClass->GetDefaultObject<UObject>();
				check(CDO);
				FString InfoProviderName = IMLInfoProviderInterface::Execute_GetMLName(CDO);
				check(InfoProvidersNamesMap.Contains(InfoProviderName) == false);
				InfoProvidersNamesMap.Add(InfoProviderName, CurrentClass);
			}
		}
	}
	
}

UClass* URLManager::GetSensorClass(FString SensorName) const
{
	UClass* const * OutSensorClass = SensorsNamesMap.Find(SensorName);

	if (OutSensorClass == nullptr)
	{
		return nullptr;
	}
	return *OutSensorClass;
}


UClass* URLManager::GetActuatorClass(FString ActuatorName) const
{
	UClass* const * OutActuatorClass = ActuatorsNamesMap.Find(ActuatorName);

	if (OutActuatorClass == nullptr)
	{
		return nullptr;
	}
	return *OutActuatorClass;
}

UClass* URLManager::GetRewarderClass(FString RewarderName) const
{
	UClass* const* OutRewarderClass = RewardersNamesMap.Find(RewarderName);

	if (OutRewarderClass == nullptr)
	{
		return nullptr;
	}
	return *OutRewarderClass;
}

UClass* URLManager::GetTerminatorClass(FString TerminatorName) const
{
	UClass* const* OutTerminatorClass = TerminatorsNamesMap.Find(TerminatorName);

	if (OutTerminatorClass == nullptr)
	{
		return nullptr;
	}
	return *OutTerminatorClass;
}

UClass* URLManager::GetInfoProviderClass(FString InfoProviderName) const
{
	UE_LOG(LogTemp, Warning, TEXT("GetInfoProviderClass called with InfoProviderName: %s"), *InfoProviderName);

	UClass* const* OutInfoProviderClass = InfoProvidersNamesMap.Find(InfoProviderName);

	if (OutInfoProviderClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No class found for InfoProviderName: %s"), *InfoProviderName);
		return nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("Class found for InfoProviderName: %s"), *InfoProviderName);
	return *OutInfoProviderClass;
}

void URLManager::StopServer()
{
	
}

void URLManager::Configure(const TSharedPtr<FJsonObject>& JsonObject)
{
	// Cleanup 
	//StopExperiment();

	// Convert the launch_settings to a struct
	const TSharedPtr<FJsonObject>& LaunchSettingsJsonObject = JsonObject->GetObjectField(TEXT("launch_settings"));
	FUELaunchSettings LaunchSettings;
	bool bSuccess = FJsonObjectConverter::JsonObjectToUStruct<FUELaunchSettings>(LaunchSettingsJsonObject.ToSharedRef(), &LaunchSettings);
	check(bSuccess);

	// Custom Timestep
	{
		// We need to control the timestep to make it run
		// at super-realtime speed with constant delta time
		// We need custom timestep. If user is already using their own custom, throw error
		check(GEngine->GetCustomTimeStep() == nullptr || GEngine->GetCustomTimeStep()->IsA(UGymEnvironmentTimeStep::StaticClass()));

		// Create custom timestep
		UGymEnvironmentTimeStep* NewCustomTimeStep = NewObject<UGymEnvironmentTimeStep>(GEngine, UGymEnvironmentTimeStep::StaticClass());

		// Get from the command line the constant tickrate
		FFrameRate FrameRate = FFrameRate(LaunchSettings.TickFPS, 1);
		// Update constant tickrate 
		NewCustomTimeStep->FrameRate = FrameRate;
		if (!GEngine->SetCustomTimeStep(NewCustomTimeStep))
		{
			check(false && TEXT("Failed to set a UGymEnvironmentTimeStep as an engine timestep."));
		}
	}


	// Create all agents and their components
	const TArray<TSharedPtr<FJsonValue>>* AgentsJson;
	bool bAgentsFound = JsonObject->TryGetArrayField(TEXT("agent_configs"), AgentsJson);
	if (bAgentsFound)
	{
		for (TSharedPtr<FJsonValue> AgentData : *AgentsJson)
		{
			// Convert the agent data to json object
			const TSharedPtr<FJsonObject>* AgentJsonObject = nullptr;
			check(AgentData->TryGetObject(AgentJsonObject));


			const TSharedPtr<FJsonObject> AgentJsonObject2 = *AgentJsonObject;
			auto AgentConfigJson = AgentJsonObject2.ToSharedRef();

			// Create agent

			// Get Agent ID
			FString AgentId;
			check(AgentConfigJson->TryGetStringField(TEXT("agent_id"), AgentId));

			// Get Agent Class name
			FString AgentClassName;
			check(AgentConfigJson->TryGetStringField(TEXT("agent_ue_class"), AgentClassName));

			// Find matching class reference
			UClass* AvatarClass = UMLUtilsFunctionLibrary::TryGetUClassFromPath(*AgentClassName);
			check(AvatarClass);

			// Find the right spawn point
			FString SpawnPointName;
			check(AgentConfigJson->TryGetStringField(TEXT("spawn_point_name"), SpawnPointName));
			FTransform InstanceTransform = UMLUtilsFunctionLibrary::GetStartingTransform(this, SpawnPointName);

			// Spawn Agent
			AActor* NewAgent = GetWorld()->SpawnActorDeferred<AActor>(AvatarClass, InstanceTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (NewAgent)
			{
				NewAgent->FinishSpawning(InstanceTransform);
			}
			check(NewAgent);

			// If the agent is pawn, spawn a default AI controller and possess it
			APawn* PawnAgent = Cast<APawn>(NewAgent);
			if (PawnAgent)
			{
				PawnAgent->SpawnDefaultController();
			}

			UMLAgentCoreComponent* AgentCore = NewObject<UMLAgentCoreComponent>(NewAgent, UMLAgentCoreComponent::StaticClass());
			check(AgentCore);
			AgentCore->RegisterComponent();

			AgentCore->Configure(AgentId, AgentConfigJson);

			// Register the agent
			RegisterAgent(NewAgent);
		}
	}


	if (IsHumanModeEnabled())
	{
		// Possess the first MLAgent (if it's pawn)
		UMLAgentCoreComponent* FirstAgentComponent = nullptr;

		if (Agents.Num() > 0)
		{
			auto It = Agents.CreateConstIterator();
			FirstAgentComponent = It.Value();
		}

		if (FirstAgentComponent != nullptr)
		{
			// Get the owner of the component and check if it's a Pawn
			APawn* OwnerPawn = Cast<APawn>(FirstAgentComponent->GetOwner());
			if (OwnerPawn)
			{
				// Get the first player controller
				APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
				if (PlayerController)
				{
					// Possess the Pawn with the player controller
					PlayerController->Possess(OwnerPawn);
					OwnerPawn->DisableInput(PlayerController);
					//PlayerController->DisableInput()
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("No player controller found"));
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No Pawn agents found. Cannot possess any Agent"));
		}
	}

}

void URLManager::StopExperiment()
{
	StepState = ERLStepState::PerformingTick;

	GEngine->SetCustomTimeStep(nullptr);
	

	// Iterate over all agent cores and destroy their agents
	for (auto& AgentPair : Agents)
	{
		UMLAgentCoreComponent* AgentCore = AgentPair.Value;
		if (AgentCore && AgentCore->GetOwner())
		{
			AgentCore->GetOwner()->Destroy();
		}
	}
	Agents.Empty();
	
}

void URLManager::StartServer(uint16 Port, uint16 ServerThreads)
{
	/*StopServer();

	ServerThreads = FMath::Max< uint16>(1, ServerThreads);

	UE_LOG(LogTemp, Warning, TEXT("Starting RPC server on port %d."), Port);
	RPCServerInstance = new FRPCServer(Port);
	check(RPCServerInstance);

	ConfigureAsServer(*RPCServerInstance);

	RPCServerInstance->async_run(ServerThreads);*/
}



/*

void URLManager::ConfigureAsServer(FRPCServer& Server)
{
	UE_LOG(LogTemp, Warning, TEXT("\tconfiguring as server"));

	

#if WITH_RPCLIB
	Server.bind("ping", []() { return "pong"; });
	Librarian.AddRPCFunctionDescription(TEXT("ping"), TEXT("(), Checks if the RPC server is still alive and responding."));

	Server.bind("step", [this](std::map<std::string, std::map<std::string, std::vector<uint8>>> Actions) {

		// Load in the actions to each actuator, so during tick it can be consumed
		for (const auto& AgentActionMap : Actions) {
			const FString AgentName = FString(UTF8_TO_TCHAR(AgentActionMap.first.c_str()));

			TMap<FString, TArray<uint8>> AgentActions = UMLUtilsFunctionLibrary::ConvertAgentActions(AgentActionMap.second);
			//const std::map<std::string, std::vector<float>>& AgentActions = ;

			UMLAgentCoreComponent* AgentCoreComp = GetAgentCoreComp(AgentName);
			if (AgentCoreComp == nullptr)
			{
				// Agent no longer exists
				// TODO(p_konarik): Handle this case better
				continue;
			}
			
			AgentCoreComp->PrepStep(AgentActions);
		}

		// Perform actual world tick
		bTickWorld = true;
		bTickFinished = false;

		while (bTickFinished == false)
		{
			FPlatformProcess::Sleep(0.001f);
		}

		// Collect step outcome data (observations, rewards, dones, infos)
		std::map<std::string, FAgentStepOutcome> AgentsOutData;

		//std::map<std::string, std::map<std::string, std::vector<uint8>>> Observations;

		for (const TPair<FString, UMLAgentCoreComponent*>& AgentPair : Agents)
		{
			FAgentStepOutcome AgentStep;

			FString AgentName = AgentPair.Key;
			UMLAgentCoreComponent* AgentCore = AgentPair.Value;

			// Gather the observations
			{
				TMap<FString, TArray<uint8>> AgentObservations;
				AgentCore->CollectObservations(AgentObservations);

				// Convert the observations to std, so we can send it over the RPC
				UMLUtilsFunctionLibrary::ConvertAgentObservationsToStdMap(AgentObservations, AgentStep.Observations);
			}

			// Gather rewards
			AgentStep.Reward = AgentCore->ConsumeReward();

			// Get Agent name in std::string
			std::string AgentNameStd(TCHAR_TO_UTF8(*AgentName));
			AgentsOutData.emplace(AgentNameStd, AgentStep);
		}


		return AgentsOutData;
	});
	Librarian.AddRPCFunctionDescription(TEXT("step"), TEXT("(int TickCount, bool bWaitForWorldTick), Requests a TickCount world ticks. This has meaning only if \'enable_manual_world_tick(true)\' has been called prior to this function. If bWaitForWorldTick is true then the call will not return until the world has been ticked required number of times"));

	Server.bind("configure", [this](std::string const& JsonConfigString) {
		// Agent -> Sensor -> float array

		// called at the beginning of the environment launch
		// Start and init agents, actuators and sensors
		FString JsonConfigFString = FString(JsonConfigString.c_str());

		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonConfigFString);
		if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
		{
			// Create all agents and their components
			const TArray<TSharedPtr<FJsonValue>>* AgentsJson;
			bool bAgentsFound = JsonObject->TryGetArrayField("agent_configs", AgentsJson);
			if (bAgentsFound)
			{
				for (TSharedPtr<FJsonValue> AgentData : *AgentsJson)
				{
					// Convert the agent data to json object
					const TSharedPtr<FJsonObject>* AgentJsonObject = nullptr;
					check(AgentData->TryGetObject(AgentJsonObject));


					const TSharedPtr<FJsonObject> AgentJsonObject2 = *AgentJsonObject;
					auto AgentConfigJson = AgentJsonObject2.ToSharedRef();

					// Create agent
					CallOnGameThread<void>([this, AgentConfigJson]()
					{
						// Get Agent ID
						FString AgentId;
						check(AgentConfigJson->TryGetStringField("agent_id", AgentId));

						// Get Agent Class name
						FString AgentClassName;
						check(AgentConfigJson->TryGetStringField("agent_ue_class", AgentClassName));
						
						// Find matching class reference
						UClass* AvatarClass = UMLUtilsFunctionLibrary::TryGetUClassFromPath(*AgentClassName);
						check(AvatarClass);

						// Find the right spawn point
						FString SpawnPointName;
						check(AgentConfigJson->TryGetStringField("spawn_point_name", SpawnPointName));
						FTransform InstanceTransform = UMLUtilsFunctionLibrary::GetStartingTransform(this, SpawnPointName);

						// Spawn Agent
						AActor* NewAgent = GetWorld()->SpawnActorDeferred<AActor>(AvatarClass, InstanceTransform, nullptr, nullptr , ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
						if (NewAgent)
						{
							NewAgent->FinishSpawning(InstanceTransform);
						}
						check(NewAgent);

						// If the agent is pawn, spawn a default AI controller and possess it
						APawn* PawnAgent = Cast<APawn>(NewAgent);
						if (PawnAgent)
						{
							PawnAgent->SpawnDefaultController();
						}

						UMLAgentCoreComponent* AgentCore = NewObject<UMLAgentCoreComponent>(NewAgent, UMLAgentCoreComponent::StaticClass());
						check(AgentCore);
						AgentCore->RegisterComponent();
							
						AgentCore->Configure(AgentId, AgentConfigJson);

						// Register the agent
						RegisterAgent(NewAgent);
						
					});
				}
			}
		}
		else {
			return "Failed to deserialize config.";
		}


		// We need to control the timestep to make it run
		// at super-realtime speed with constant delta time
		CallOnGameThread<void>([]()
		{
			// We need custom timestep. If user is already using their own custom, throw error
			check(GEngine->GetCustomTimeStep() == nullptr);

			// Create custom timestep
			UGymEnvironmentTimeStep* NewCustomTimeStep = NewObject<UGymEnvironmentTimeStep>(GEngine, UGymEnvironmentTimeStep::StaticClass());

			// Get from the command line the constant tickrate
			int32 TickFPS = 24;
			FParse::Value(FCommandLine::Get(), TEXT("tickfps"), TickFPS);
			FFrameRate FrameRate = FFrameRate(TickFPS, 1);
			// Update constant tickrate 
			NewCustomTimeStep->FrameRate = FrameRate;
			if (!GEngine->SetCustomTimeStep(NewCustomTimeStep))
			{
				check(false && TEXT("Failed to set a UGymEnvironmentTimeStep as an engine timestep."));
			}
		});


		return "Configuration Done";
	});
	Librarian.AddRPCFunctionDescription(TEXT("configure"), TEXT("(int TickCount, bool bWaitForWorldTick), Requests a TickCount world ticks. This has meaning only if \'enable_manual_world_tick(true)\' has been called prior to this function. If bWaitForWorldTick is true then the call will not return until the world has been ticked required number of times"));

	
	Server.bind("reset", [this]() {

		// Load in the actions to each actuator, so during tick it can be consumed
		CallOnGameThread<void>([this]()
		{
			// Iterate over the Agents TMap
			for (const auto& AgentPair : GetAgents())
			{
				FString AgentKey = AgentPair.Key;
				UMLAgentCoreComponent* AgentValue = AgentPair.Value;

				// Perform any desired operations with AgentKey and AgentValue
				AgentValue->Reset();
			}

			// TODO: Reset any actors with ml_reset flag
			// Teleport them and reset physics




			// Call OnPostReset to allow for first observations
			for (const auto& AgentPair : GetAgents())
			{
				FString AgentKey = AgentPair.Key;
				UMLAgentCoreComponent* AgentValue = AgentPair.Value;

				AgentValue->OnPostReset();
			}
		});


		// Perform actual world tick
		if(false) {
			bTickWorld = true;
			bTickFinished = false;

			while (bTickFinished == false)
			{
				FPlatformProcess::Sleep(0.001f);
			}
		}


		// Collect step outcome data (observations, rewards, dones, infos)
		std::map<std::string, FAgentStepOutcome> AgentsOutData;

		//std::map<std::string, std::map<std::string, std::vector<uint8>>> Observations;

		for (const TPair<FString, UMLAgentCoreComponent*>& AgentPair : Agents)
		{
			FAgentStepOutcome AgentStep;

			FString AgentName = AgentPair.Key;
			UMLAgentCoreComponent* AgentCore = AgentPair.Value;

			// Gather the observations
			{
				TMap<FString, TArray<uint8>> AgentObservations;
				AgentCore->CollectObservations(AgentObservations);

				// Convert the observations to std, so we can send it over the RPC
				UMLUtilsFunctionLibrary::ConvertAgentObservationsToStdMap(AgentObservations, AgentStep.Observations);
			}

			// Gather rewards
			AgentStep.Reward = AgentCore->ConsumeReward();

			// Get Agent name in std::string
			std::string AgentNameStd(TCHAR_TO_UTF8(*AgentName));
			AgentsOutData.emplace(AgentNameStd, AgentStep);
		}


		return AgentsOutData;
	});



	

#endif // WITH_RPCLIB

	
}
*/

#pragma region Agents




bool URLManager::RegisterAgent(AActor* Agent)
{
	if (!IsValid(Agent))
	{
		return false;
	}

	UMLAgentCoreComponent* AgentCoreComponent = Agent->FindComponentByClass<UMLAgentCoreComponent>();
	if (AgentCoreComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to register agent %s. No mandatory UMLAgentCoreComponent component found."), *Agent->GetName());
		return false;
	}

	FString AgentName = AgentCoreComponent->GetAgentName();
	// Sanity check - Check if agent of the same name already exists or this agent is already registered
	UMLAgentCoreComponent* ExistingAgentComp = GetAgentCoreComp(AgentName);
	if (ExistingAgentComp)
	{
		if (ExistingAgentComp == AgentCoreComponent)
		{
			// This exact agent was already registered
			UE_LOG(LogTemp, Warning, TEXT("Agent %s was already registered"), *Agent->GetName());
			return true;
		}
		else {
			// Another agent with the same name registered. This is illegal
			UE_LOG(LogTemp, Error, TEXT("Agent %s failed to register, because another agent (%s) already uses that name."), *Agent->GetName(), *ExistingAgentComp->GetOwner()->GetName());
			return true;
		}		
	}

	// All conditions met, just register agent
	Agents.Add(AgentName, AgentCoreComponent);
	return true;
}

void URLManager::UnregisterAgent(FString AgentName)
{
	Agents.Remove(AgentName);
}

AActor* URLManager::GetAgent(FString AgentName)
{
	UMLAgentCoreComponent* CoreComp = GetAgentCoreComp(AgentName);
	if (CoreComp == nullptr)
	{
		return nullptr;
	}
	return CoreComp->GetOwner();
}

UMLAgentCoreComponent* URLManager::GetAgentCoreComp(FString AgentName)
{
	UMLAgentCoreComponent** CoreComp = Agents.Find(AgentName);
	if (CoreComp == nullptr)
	{
		return nullptr;
	}
	return *CoreComp;
}


void URLManager::HandleResetRequest(FAction_ResetRequest& ResetData)
{
	OnPreEpisodeReset.Broadcast();

	// Trigger reset
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	GameMode->ResetLevel();

	// Iterate over the Agents TMap
	for (const auto& AgentPair : GetAgents())
	{
		FString AgentKey = AgentPair.Key;
		UMLAgentCoreComponent* AgentValue = AgentPair.Value;

		// Perform any desired operations with AgentKey and AgentValue
		AgentValue->Reset();
	}

	// Reset any actors with ml_reset flag
	// Teleport them and reset physics
	ResetMLResetActorsTransforms();

	// Give anyone else change to reset
	OnPostEpisodeReset.Broadcast();

	// Call OnPostReset to allow for first observations
	for (const auto& AgentPair : GetAgents())
	{
		FString AgentKey = AgentPair.Key;
		UMLAgentCoreComponent* AgentValue = AgentPair.Value;

		AgentValue->OnPostReset();
	}
}


void URLManager::PrepHumanStep()
{
	for (const auto& AgentActionMap : Agents) {
		const FString AgentName = AgentActionMap.Key;
		UMLAgentCoreComponent* AgentCoreComp = AgentActionMap.Value;
		if (AgentCoreComp == nullptr)
		{
			// Agent no longer exists
			// TODO(p_konarik): Handle this case better
			continue;
		}

		AgentCoreComp->PrepStepHuman();
	}
}

void URLManager::HandleStepRequest(FAction_StepRequest& StepRequest)
{
	for (const auto& AgentActionMap : StepRequest.Agents) {
		const FString AgentName = AgentActionMap.Key;
		const FAgentActuators ActuatorsActions = AgentActionMap.Value;

		UMLAgentCoreComponent* AgentCoreComp = GetAgentCoreComp(AgentName);
		if (AgentCoreComp == nullptr)
		{
			// Agent no longer exists
			// TODO(p_konarik): Handle this case better
			continue;
		}

		AgentCoreComp->PrepStep(ActuatorsActions);
	}

	StartStep();
}

void URLManager::GatherObservations(TMap<FString, FAgentObservations>& Observations)
{
	for (const TPair<FString, UMLAgentCoreComponent*>& AgentPair : Agents)
	{
		FAgentObservations AgentObservations;

		FString AgentName = AgentPair.Key;
		UMLAgentCoreComponent* AgentCore = AgentPair.Value;

		// Gather the observations
		AgentCore->CollectObservations(AgentObservations);
		Observations.Add(AgentName, AgentObservations);
	}
}

void URLManager::GatherRewards(TMap<FString, FAgentRewards>& Rewards)
{
	for (const TPair<FString, UMLAgentCoreComponent*>& AgentPair : Agents)
	{
		FAgentRewards AgentRewards;

		FString AgentName = AgentPair.Key;
		UMLAgentCoreComponent* AgentCore = AgentPair.Value;

		// Consume the actual reward
		AgentCore->CollectAndConsumeReward(AgentRewards);

		Rewards.Add(AgentName, AgentRewards);
	}
}

void URLManager::GatherTerminators(TMap<FString, FAgentTerminations>& Terminators)
{
	for (const TPair<FString, UMLAgentCoreComponent*>& AgentPair : Agents)
	{
		FAgentTerminations AgentTerminations;

		FString AgentName = AgentPair.Key;
		UMLAgentCoreComponent* AgentCore = AgentPair.Value;

		AgentCore->CollectTerminations(AgentTerminations);

		Terminators.Add(AgentName, AgentTerminations);
	}
}

/*
void URLManager::GatherTerminated(TMap<FString, bool>& Terminated)
{
	bool bAllFinished = true;

	for (const TPair<FString, UMLAgentCoreComponent*>& AgentPair : Agents)
	{
		FString AgentName = AgentPair.Key;
		UMLAgentCoreComponent* AgentCore = AgentPair.Value;

		bool AgentTerminated = AgentCore->CollectTerminated();
		Terminated.Add(AgentName, AgentTerminated);
		
		bAllFinished = bAllFinished && AgentTerminated;
	}
	
	Terminated.Add("__all__", bAllFinished);
}

void URLManager::GatherTruncated(TMap<FString, bool>& Truncated)
{
	bool bAllTruncated = true;

	for (const TPair<FString, UMLAgentCoreComponent*>& AgentPair : Agents)
	{
		FString AgentName = AgentPair.Key;
		UMLAgentCoreComponent* AgentCore = AgentPair.Value;

		bool AgentTruncated = AgentCore->CollectTruncated();
		Truncated.Add(AgentName, AgentTruncated);

		bAllTruncated = bAllTruncated && AgentTruncated;
	}

	Truncated.Add("__all__", bAllTruncated);
}
*/

void URLManager::GatherInfos(TMap<FString, FAgentInfos>& Infos)
{
	for (const TPair<FString, UMLAgentCoreComponent*>& AgentPair : Agents)
	{
		FAgentInfos AgentInfos;

		FString AgentName = AgentPair.Key;
		UMLAgentCoreComponent* AgentCore = AgentPair.Value;

		// Gather the observations
		AgentCore->CollectInfos(AgentInfos);
		Infos.Add(AgentName, AgentInfos);
	}
}



void URLManager::StoreMLResetActorsTransforms()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("MLReset"), FoundActors);

	UE_LOG(LogTemp, Warning, TEXT("Storing position of %d MLReset actors"), FoundActors.Num());

	for (AActor* Actor : FoundActors)
	{
		FActorTransformData TransformData;
		TransformData.Actor = Actor;
		TransformData.OriginalTransform = Actor->GetTransform();
		StoredTransforms.Add(TransformData);
	}
}

void URLManager::ResetMLResetActorsTransforms()
{
	for (const FActorTransformData& TransformData : StoredTransforms)
	{
		AActor* Actor = TransformData.Actor;
		if (IsValid(Actor))
		{
			Actor->SetActorTransform(TransformData.OriginalTransform, false, nullptr, ETeleportType::ResetPhysics);
		}
	}
}

void URLManager::WaitForStep()
{
	UServerWebSocketSubsystem* WebsocketServer = GetWorld()->GetGameInstance()->GetSubsystem<UServerWebSocketSubsystem>();
	check(StepState != ERLStepState::PerformingTick);
	
	StepState = ERLStepState::WaitingForCommand;

	while (StepState == ERLStepState::WaitingForCommand)
	{
		FPlatformProcess::SleepNoStats(0.0001f);
		WebsocketServer->Tick();
	}
}

void URLManager::StartStep()
{
	StepState = ERLStepState::PerformingTick;
}

void URLManager::FinishStep()
{
	if (StepState == ERLStepState::PerformingTick)
	{
		StepState = ERLStepState::TickFinished;

		UServerWebSocketSubsystem* WebsocketServer = GetWorld()->GetGameInstance()->GetSubsystem<UServerWebSocketSubsystem>();
		WebsocketServer->FinishStepRequest();
	}
}


// Implementation of GetAgents() function
const TMap<FString, UMLAgentCoreComponent*>& URLManager::GetAgents() const
{
	return Agents;
}

#pragma endregion Agents
