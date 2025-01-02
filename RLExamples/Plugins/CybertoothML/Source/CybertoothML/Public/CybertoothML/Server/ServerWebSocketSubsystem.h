// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "INetworkingWebSocket.h"
#include "IWebSocketServer.h"
#include "Containers/CircularQueue.h"
#include "CybertoothCommunicationCore/Requests.h"
#include "ServerWebSocketSubsystem.generated.h"

class AStrategyAgentEnvCharacter;

UENUM()
enum class ERLEnvCommandType : uint8
{
	NoAction = 0,
	ConfigureRequest = 1,
	ConfigureOutcome = 2,
	ResetRequest = 3,
	ResetOutcome = 4,
	StepRequest = 5,
	StepOutcome = 6,
};



/**
 * 
 */
UCLASS()
class CYBERTOOTHML_API UServerWebSocketSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:

	virtual bool ShouldCreateSubsystem(UObject * Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick();

protected:
	void OnWebSocketClientConnected(INetworkingWebSocket* ClientWebSocket); // to the server.

	virtual void ReceivedRawPacket(void* Data, int32 Count);
	virtual void OnSocketClose(INetworkingWebSocket* Socket);

	virtual void HandleConfigureRequest(TArray<uint8>& FunctionPayload);
	virtual void HandleResetRequest(TArray<uint8>& FunctionPayload);
	virtual void HandleStepRequest(TArray<uint8>& FunctionPayload);

	UPROPERTY()
	bool bUseSubsystem = true;

	UPROPERTY()
	uint32 WebSocketPortDefault = 33333;

public:
	void FinishStepRequest();

private:
	TUniquePtr<IWebSocketServer> ServerWebSocket;

	// TODO(p_konarik): Safety issue
	INetworkingWebSocket* ClientConnection = nullptr;

};

