// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Server/ServerWebSocketSubsystem.h"

#include "INetworkingWebSocket.h"
#include "IWebSocketNetworkingModule.h"
#include "WebSocketNetworkingDelegates.h"
#include <Containers/UnrealString.h>
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "CybertoothCommunicationCore/Requests.h"
#include "JsonUtilities.h"
#include "CborWriter.h"
#include "CborReader.h"
#include "Backends/CborStructSerializerBackend.h"
#include "IStructSerializerBackend.h"
#include "StructSerializer.h"
#include "Backends/CborStructDeserializerBackend.h"
#include "StructDeserializer.h"
#include "../../Public/CybertoothML/MLTypes.h"
#include "../../Public/CybertoothML/RLManager.h"




int32 PredictMode = 0;
FAutoConsoleVariableRef CVarPredictModeMode(
	TEXT("ML.PredictMode"),
	PredictMode, TEXT("1 enables to play the game."));


int32 RenderVideo = 0;
FAutoConsoleVariableRef CVarRenderVideo(TEXT("ML.RenderVideo"), RenderVideo, TEXT("1 enables GPU renderer."));






bool UServerWebSocketSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return bUseSubsystem;
}

void UServerWebSocketSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	
	ServerWebSocket = FModuleManager::Get().LoadModuleChecked<IWebSocketNetworkingModule>(TEXT("WebSocketNetworking")).CreateServer();

	FWebSocketClientConnectedCallBack CallBack;
	CallBack.BindUObject(this, &UServerWebSocketSubsystem::OnWebSocketClientConnected);

	// Get port 
	int32 WebSocketPort = WebSocketPortDefault;
	FParse::Value(FCommandLine::Get(), TEXT("ws_port"), WebSocketPort);

	UE_LOG(LogTemp, Warning, TEXT("Websocket server port %d"), WebSocketPort);

	if (!ServerWebSocket->Init(WebSocketPort, CallBack))
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerWebSocket Init FAIL"));
		ServerWebSocket.Reset();
		CallBack.Unbind();

		// Kill the whole simulation
		FGenericPlatformMisc::RequestExit(true);

		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("**** Starting run %s"), *RunName);

	/*
	FTSTicker::FDelegateHandle TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([=](float Time)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Tick"));
		if (ServerWebSocket.IsValid())
		{
			ServerWebSocket->Tick();
			return true;
		}
		else
		{
			return false;
		}
	}));
	*/
}


void UServerWebSocketSubsystem::Deinitialize()
{
	ServerWebSocket = nullptr;


	/*
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
	*/
}

void UServerWebSocketSubsystem::Tick()
{
	if (ServerWebSocket)
	{
		ServerWebSocket->Tick();
	}
}



void UServerWebSocketSubsystem::OnWebSocketClientConnected(INetworkingWebSocket* ClientWebSocket)
{
	FWebSocketPacketReceivedCallBack CallBack;
	CallBack.BindUObject(this, &UServerWebSocketSubsystem::ReceivedRawPacket);
	ClientWebSocket->SetReceiveCallBack(CallBack);

	FWebSocketInfoCallBack CloseCallback;
	CloseCallback.BindUObject(this, &UServerWebSocketSubsystem::OnSocketClose, ClientWebSocket);
	ClientWebSocket->SetSocketClosedCallBack(CloseCallback);


	// Do not allow multiple connections to this server
	if (ClientConnection != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("New connection was connected, but we had already client connection."));
	}

	// Store the connection so we can send data to it
	ClientConnection = ClientWebSocket;
}



void UServerWebSocketSubsystem::ReceivedRawPacket(void* Data, int32 Count)
{
	if (Count == 0)   // nothing to process
	{
		return;
	}

	// Cast the data pointer to uint8* and create a TArray from it
	uint8* DataPtr = static_cast<uint8*>(Data);
	TArray<uint8> FunctionPayload(DataPtr, Count);
	
	// The first byte is the identifier of the action (step/reset/configure)
	uint8 FunctionIdentifier = FunctionPayload[0];
	FunctionPayload.RemoveAt(0);

	ERLEnvCommandType CommandType = ERLEnvCommandType(FunctionIdentifier);

	switch (CommandType)
	{
	case ERLEnvCommandType::ConfigureRequest:
		// Handle ConfigureRequest case
		HandleConfigureRequest(FunctionPayload);
		break;
	case ERLEnvCommandType::ResetRequest:
		// Handle ResetRequest case
		HandleResetRequest(FunctionPayload);
		break;
	case ERLEnvCommandType::StepRequest:
		// Handle StepRequest case
		HandleStepRequest(FunctionPayload);
		break;
	case ERLEnvCommandType::NoAction:
	case ERLEnvCommandType::ConfigureOutcome:
	case ERLEnvCommandType::ResetOutcome:
	case ERLEnvCommandType::StepOutcome:
	default:
		// Handle unknown or invalid CommandType
		break;
	}


	/*
	FStepOutcomeData2 SomeStruct;
	FMemoryReader Reader(FunctionPayload);
	FCborStructDeserializerBackend DeserializerBackend(Reader);

	bool bSuccess = FStructDeserializer::Deserialize(SomeStruct, DeserializerBackend);
	check(bSuccess);

	int32 DesNum = SomeStruct.AgentsData.FindRef("Agent1").Sensors.FindRef("sensor1").Observations.Num();
	UE_LOG(LogTemp, Warning, TEXT("Here %d"), DesNum);
	*/

	/*
	const FUTF8ToTCHAR TCHARConverter(reinterpret_cast<const ANSICHAR*>(Data), Count);
	const FString Message(TCHARConverter.Length(), TCHARConverter.Get());
	
	Messages.Enqueue(Message);
	*/
}

void UServerWebSocketSubsystem::OnSocketClose(INetworkingWebSocket* Socket)
{
	UE_LOG(LogTemp, Warning, TEXT("Websocket connection closed"));
	if (ClientConnection == Socket)
	{
		ClientConnection = nullptr;
	}

	URLManager* RLManager = URLManager::Get(this);
	if (RLManager)
	{
		RLManager->StopExperiment();
	}

	// Check if we are in a PIE session and end it gracefully
	FGenericPlatformMisc::RequestExit(false);
}

void UServerWebSocketSubsystem::HandleConfigureRequest(TArray<uint8>& FunctionPayload)
{

	// Convert the byte array to a string
	const std::string cstr(reinterpret_cast<const char*>(FunctionPayload.GetData()), FunctionPayload.Num());
	FString JsonString = cstr.c_str();


	// Deserialize the JSON object from the string
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to deserialize FunctionPayload: %s"), *JsonString);
		return;
	}

	URLManager* RLManager = URLManager::Get(GetGameInstance());
	RLManager->Configure(JsonObject);

	// It seems all config went fine, send "ok" back to 
	const FString StringMessage = TEXT("ok");

	// Convert FString into uint8 array.
	FTCHARToUTF8 UTF8String(*StringMessage);
	int32 MessageSize = UTF8String.Length();
	TArray<uint8> MessageArr;
	MessageArr.SetNum(MessageSize);
	FMemory::Memcpy(MessageArr.GetData(), UTF8String.Get(), MessageSize);
	
	ClientConnection->Send(MessageArr.GetData(), MessageSize, false);
}

void UServerWebSocketSubsystem::HandleResetRequest(TArray<uint8>& FunctionPayload)
{
	// Convert the byte array to a string
	const std::string cstr(reinterpret_cast<const char*>(FunctionPayload.GetData()), FunctionPayload.Num());
	FString JsonString = cstr.c_str();
	
	
	FAction_ResetRequest ResetData;
	bool bSuccess = FJsonObjectConverter::UStructToJsonObjectString<FAction_ResetRequest>(ResetData, JsonString);
	check(bSuccess);

	URLManager* RLManager = URLManager::Get(GetGameInstance());
	RLManager->HandleResetRequest(ResetData);

	FAction_ResetOutcome ResetOutcome;
	// Collect observations
	RLManager->GatherObservations(ResetOutcome.Obs);
	// Collect infoers
	RLManager->GatherInfos(ResetOutcome.Infos);



	// Writing from a struct to bytes array
	TArray<uint8> Buffer;
	FMemoryWriter Writer(Buffer);
	FCborStructSerializerBackend SerializerBackend(Writer, EStructSerializerBackendFlags::LegacyUE4 | EStructSerializerBackendFlags::WriteCborStandardEndianness);
	FStructSerializer::Serialize(ResetOutcome, SerializerBackend);

	ClientConnection->Send(Buffer.GetData(), Buffer.Num(), false);
}


void UServerWebSocketSubsystem::HandleStepRequest(TArray<uint8>& FunctionPayload)
{

	FAction_StepRequest StepRequest;
	FMemoryReader Reader(FunctionPayload);
	FCborStructDeserializerBackend DeserializerBackend(Reader, ECborEndianness::BigEndian);

	bool bSuccess = FStructDeserializer::Deserialize(StepRequest, DeserializerBackend);
	check(bSuccess);
	

	URLManager* RLManager = URLManager::Get(GetGameInstance());
	// This will also trigger world tick
	RLManager->HandleStepRequest(StepRequest);
	// FinishStepRequest will be called after the world tick
	// from UGymEnvironmentTimeStep
}

void UServerWebSocketSubsystem::FinishStepRequest()
{
	URLManager* RLManager = URLManager::Get(GetGameInstance());

	FAction_StepOutcome StepOutcome;
	// Collect observations
	RLManager->GatherObservations(StepOutcome.Obs);
	// Collect Rewards
	RLManager->GatherRewards(StepOutcome.Rewards);
	// Collect Terminated
	RLManager->GatherTerminators(StepOutcome.Terminated);
	// Collect infoers
	RLManager->GatherInfos(StepOutcome.Infos);

	// Writing from a struct to bytes array
	TArray<uint8> Buffer;
	FMemoryWriter Writer(Buffer);
	FCborStructSerializerBackend SerializerBackend(Writer, EStructSerializerBackendFlags::LegacyUE4 | EStructSerializerBackendFlags::WriteCborStandardEndianness);
	FStructSerializer::Serialize(StepOutcome, SerializerBackend);

	check(ClientConnection != nullptr);

	ClientConnection->Send(Buffer.GetData(), Buffer.Num(), false);
}