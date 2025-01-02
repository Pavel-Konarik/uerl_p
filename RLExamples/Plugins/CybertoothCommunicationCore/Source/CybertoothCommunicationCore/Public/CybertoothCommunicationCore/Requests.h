// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WebSocketsModule.h" // Module definition
#include "IWebSocket.h"
#include "Containers/Ticker.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include <JsonObjectConverter.h>

#include "Requests.generated.h"


template<typename InStructType>
static bool SendMessageWS(const InStructType& InStruct, TSharedPtr<IWebSocket> Websocket)
{
	if (Websocket && Websocket->IsConnected())
	{
		FString MessageStr = "";
		FJsonObjectConverter::UStructToJsonObjectString<InStructType>(InStruct, MessageStr);

		UE_LOG(LogTemp, Log, TEXT("Sending WS: %s"), *MessageStr);

		Websocket->Send(MessageStr);
		return true;
	}
	return false;
}

/** This method is designed to likely return false and can be used for routing message to the correct handler */
template<typename InStructType>
static bool HandleMessage(FString ActionName, const TSharedPtr<FJsonObject> JsonObject, InStructType& OutStruct)
{
	if (ActionName.Equals(InStructType::ActionName, ESearchCase::IgnoreCase)) {
		bool Success = InStructType::FromJson(JsonObject, OutStruct);
		if (Success) {
			return true;
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("[CoreComms] Failed to convert %s to UStruct."), *ActionName);
		}
	}
	return false;

}

template<typename InStructType>
static bool HandleMessage(const TSharedPtr<FJsonObject> JsonObject, InStructType& OutStruct)
{
	bool Success = InStructType::FromJson(JsonObject, OutStruct);
	if (Success) {
		return true;
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("[CoreComms] Failed to convert  to UStruct."));
	}
	return false;
}

USTRUCT(BlueprintType)
struct FWebsocketMessageBase
{
	GENERATED_BODY()
public:
	FWebsocketMessageBase()
	{
	}

	template<typename InStructType>
	static bool FromJson(const TSharedPtr<FJsonObject> JsonObject, InStructType& OutStruct)
	{
		return FJsonObjectConverter::JsonObjectToUStruct<InStructType>(JsonObject.ToSharedRef(), &OutStruct, 0, 0);
	}

	template<typename InStructType>
	static bool FromString(const FString& JsonString, InStructType& OutStruct)
	{
		return FJsonObjectConverter::JsonObjectStringToUStruct<InStructType>(JsonString, &OutStruct, 0, 0);
	}
	
};


USTRUCT(BlueprintType)
struct FWebsocketActionMessageBase : public FWebsocketMessageBase
{
	GENERATED_BODY()
public:
	static FString ActionName;

	UPROPERTY(BlueprintReadWrite, Category = "Dashboard")
	FString action;

	FWebsocketActionMessageBase()
	{
		action = "";
	}
};
