// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CybertoothCommunicationCore/Requests.h"
#include "MLTypes.generated.h"


USTRUCT(BlueprintType)
struct FSensorObservation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML")
	TArray<uint8> Data;

	FSensorObservation()
	{
	}
};




USTRUCT(BlueprintType)
struct FAgentObservations
{
	GENERATED_BODY()

	/** Sensor observation name -> the observation bytes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ML")
	TMap<FString, FSensorObservation> Sensors; 

	FAgentObservations()
	{
		
	}
};



USTRUCT(BlueprintType)
struct FTerminatorStepOutcome
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML")
	bool Terminated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML")
	bool Truncated;

	FTerminatorStepOutcome()
	{
		Terminated = false;
		Truncated = false;
	}

	FTerminatorStepOutcome(bool bInTerminated, bool bInTruncated)
	{
		Terminated = bInTerminated;
		Truncated = bInTruncated;
	}
};




USTRUCT(BlueprintType)
struct FAgentTerminations
{
	GENERATED_BODY()

	/** Terminator name -> termination outcome (bool Terminated and bool Truncated) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ML")
	TMap<FString, FTerminatorStepOutcome> Terminators;

	FAgentTerminations()
	{
		
	}
};







USTRUCT(BlueprintType)
struct FRewarderStepOutcome
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML")
	float Reward;

	FRewarderStepOutcome()
	{
		Reward = 0.0f;
	}

	FRewarderStepOutcome(float InReward)
	{
		Reward = InReward;
	}
};




USTRUCT(BlueprintType)
struct FAgentRewards
{
	GENERATED_BODY()

	/** Rewarder name -> reward for the given step */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ML")
	TMap<FString, FRewarderStepOutcome> Rewarders;

	FAgentRewards()
	{
		
	}
};







USTRUCT(BlueprintType)
struct FAgentInfos
{
	GENERATED_BODY()

	/** Info Provider name -> the info bytes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML")
	TMap<FString, FSensorObservation> InfoProviders;

	FAgentInfos()
	{
		
	}
};



USTRUCT()
struct FAction_ResetRequest
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 seed;

	UPROPERTY()
	TMap<FString, FString> options;

	FAction_ResetRequest()
	{
		seed = 0;
	}
};


USTRUCT()
struct FAction_ResetOutcome
{
	GENERATED_BODY()

public:
	/** Agent ID -> Sensors */
	UPROPERTY()
	TMap<FString, FAgentObservations> Obs;

	/** Agent ID -> Info Providers */
	UPROPERTY()
	TMap<FString, FAgentInfos> Infos;

	FAction_ResetOutcome()
	{
	}
};


USTRUCT(BlueprintType)
struct FActuatorData
{
	GENERATED_BODY()

public:
	/** */
	UPROPERTY()
	TArray<uint8> Data;

	FActuatorData()
	{
	}
};

USTRUCT()
struct FAgentActuators
{
	GENERATED_BODY()

public:
	/** Agent ID -> Actions ->  */
	UPROPERTY()
	TMap<FString, FActuatorData> Actuators;

	FAgentActuators()
	{
	}
};

USTRUCT()
struct FAction_StepRequest
{
	GENERATED_BODY()

public:
	/** Agent ID -> Actions ->  */
	UPROPERTY()
	TMap<FString, FAgentActuators> Agents;

	FAction_StepRequest()
	{
	}
};

USTRUCT()
struct FAction_StepOutcome
{
	GENERATED_BODY()

public:
	/** Agent ID -> Sensors */
	UPROPERTY()
	TMap<FString, FAgentObservations> Obs;

	/** Agent ID -> Rewards */
	UPROPERTY()
	TMap<FString, FAgentRewards> Rewards;

	/** Agent ID -> Terminated */
	UPROPERTY()
	TMap<FString, FAgentTerminations> Terminated;

	/** Agent ID -> Info Providers */
	UPROPERTY()
	TMap<FString, FAgentInfos> Infos;

	FAction_StepOutcome()
	{
	}
};


#pragma region Configuration

USTRUCT()
struct FAgentComponentConfig
{
    GENERATED_BODY()

    UPROPERTY()
    FString Name;

    UPROPERTY()
    FString UEClassName;

    FAgentComponentConfig()
        : UEClassName(TEXT("unset_class"))
    {
    }
};

USTRUCT()
struct FSensorConfig : public FAgentComponentConfig
{
	GENERATED_BODY()
};


USTRUCT()
struct FActuatorConfig : public FAgentComponentConfig
{
	GENERATED_BODY()
};


USTRUCT()
struct FRewarderConfig : public FAgentComponentConfig
{
	GENERATED_BODY()
};


USTRUCT()
struct FTerminatorConfig : public FAgentComponentConfig
{
	GENERATED_BODY()
};


USTRUCT()
struct FInfoProviderConfig : public FAgentComponentConfig
{
	GENERATED_BODY()
};




USTRUCT(BlueprintType)
struct FAgentConfig
{
    GENERATED_BODY()

    UPROPERTY()
    FString AgentId;

    UPROPERTY()
    FString AgentUEClass;

    UPROPERTY()
    FString SpawnPointName;

    UPROPERTY()
    TArray<FSensorConfig> SensorConfigs;

    UPROPERTY()
    TArray<FActuatorConfig> ActuatorConfigs;

    UPROPERTY()
    TArray<FRewarderConfig> RewarderConfigs;

    UPROPERTY()
    TArray<FTerminatorConfig> TerminatorConfigs;

    UPROPERTY()
    TArray<FInfoProviderConfig> InfosConfigs;
};

USTRUCT()
struct FUELaunchSettings
{
    GENERATED_BODY()

    UPROPERTY()
    FString MapName;

    UPROPERTY()
    bool bNoSound;

    UPROPERTY()
    FString RunName;

    UPROPERTY()
    int32 TickFPS;

    UPROPERTY()
    FString SpecificIP;

    UPROPERTY()
    int32 SpecificPort;

    UPROPERTY()
    int32 WindowResX;

    UPROPERTY()
    int32 WindowResY;

    UPROPERTY()
    bool bOutputUELog;
};


USTRUCT()
struct FUEEnvConfig
{
	GENERATED_BODY()

	UPROPERTY()
	FUELaunchSettings launch_settings;

	UPROPERTY()
	TArray<FAgentConfig> agent_configs;

	FUEEnvConfig()
	{

	}
};
#pragma endregion Configuration

