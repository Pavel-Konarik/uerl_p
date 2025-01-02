// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Utils/MLUtilsFunctionLibrary.h"
#include "CybertoothML/Utils/AgentStart.h"
#include "GameFramework/PlayerState.h"
#include "UnrealClient.h"
#include "Kismet/GameplayStatics.h"
#include "Dom/JsonObject.h"
#include "EngineUtils.h"
#include "../../Public/CybertoothML/CybertoothMLSettings.h"

void UMLUtilsFunctionLibrary::SetPlayerScore(APlayerState* PlayerState, float Score)
{
	if (PlayerState)
	{
		PlayerState->SetScore(Score);
	}
}


FString UMLUtilsFunctionLibrary::GenerateRandomAgentName()
{
	const FString Prefix = "Agent_";
	const int32 SuffixLength = 5;
	const FString Characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	FString Suffix;

	for (int32 i = 0; i < SuffixLength; ++i)
	{
		int32 Index = FMath::RandRange(0, Characters.Len() - 1);
		Suffix.AppendChar(Characters[Index]);
	}

	return Prefix + Suffix;
}

TSubclassOf<AActor> UMLUtilsFunctionLibrary::GetActorFromString(const FString InName)
{
	const FString aEnumTypeString(InName);
	TSubclassOf<AActor> AvatarClass = UClass::TryFindTypeSlow<UClass>(*aEnumTypeString);

	return AvatarClass;
}


bool UMLUtilsFunctionLibrary::ConvertFStringToTraceType(const FString& TraceTypeName, TEnumAsByte<ETraceTypeQuery>& OutType)
{
	const UCybertoothMLSettings* CoreSettings = GetDefault<UCybertoothMLSettings>();
	if (CoreSettings)
	{
		const TEnumAsByte<ETraceTypeQuery>* TraceType = CoreSettings->TraceChannelsMap.Find(TraceTypeName);
		if(TraceType)
		{
			OutType = *TraceType;
			return true;
		}
	}

	return false;

	/*
	static UEnum* ChannelEnum = StaticEnum<ETraceTypeQuery>();
	
	for (int32 i = 0; i < ChannelEnum->NumEnums(); i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("Here %s"), *ChannelEnum->GetDisplayNameTextByIndex(i).ToString());
		if (ChannelEnum->GetDisplayNameTextByIndex(i).ToString().Equals(TraceTypeName, ESearchCase::IgnoreCase))
		{
			OutType = static_cast<ETraceTypeQuery>(ChannelEnum->GetValueByIndex(i));
			return true;
		}
	}

	return false;
	*/
}

bool UMLUtilsFunctionLibrary::ExtractFColor(const TSharedPtr<FJsonObject> JsonObject, FColor& OutColor)
{
	OutColor = FColor::Black;

	uint8 R;
	if (JsonObject->TryGetNumberField(TEXT("r"), R) == false)
	{
		return false;
	}
	uint8 G;
	if (JsonObject->TryGetNumberField(TEXT("g"), G) == false)
	{
		return false;
	}

	uint8 B;
	if (JsonObject->TryGetNumberField(TEXT("b"), B) == false)
	{
		return false;
	}

	uint8 A = 255;
	JsonObject->TryGetNumberField(TEXT("a"), A);

	OutColor = FColor(R, G, B, A);

	return true;
}

USceneComponent* UMLUtilsFunctionLibrary::FindComponentByName(AActor* Actor, const FString& ComponentName)
{
	if (!Actor)
	{
		return nullptr;
	}

	TArray<USceneComponent*> Components;
	Actor->GetComponents<USceneComponent>(Components);

	for (USceneComponent* Component : Components)
	{
		if (Component->GetName() == ComponentName)
		{
			return Component;
		}
	}

	return nullptr;
}

AActor* UMLUtilsFunctionLibrary::GetActorByName(UObject* WorldContextObject, const FString& Name)
{
	// Ensure WorldContextObject is valid and obtain the World from it
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Iterate over all actors in the world
	for (TActorIterator<AActor> It(World, AActor::StaticClass()); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor)
		{
			// Log the name of the actor being checked
			UE_LOG(LogTemp, Log, TEXT("Checking actor: %s"), *Actor->GetActorNameOrLabel());

			// Convert FString to FName before comparison and check if names match
			if (Actor->GetActorNameOrLabel() == Name)
			{
				UE_LOG(LogTemp, Log, TEXT("Found actor with matching name: %s"), *Name);
				return Actor;
			}
		}
	}

	return nullptr;
}

UClass* UMLUtilsFunctionLibrary::TryGetUClassFromPath(FString InPath)
{
	// Create an FSoftObjectPath from the FString
	FSoftObjectPath ClassObjectPath(InPath);

	// Initialize a TSoftClassPtr from the FSoftObjectPath
	TSoftClassPtr<AActor> SoftClassPtr(ClassObjectPath);

	// Optionally, you can resolve the TSoftClassPtr to UClass* (if the asset is loaded)
	UClass* ClassPtr = SoftClassPtr.LoadSynchronous();

	return ClassPtr;
}


FTransform UMLUtilsFunctionLibrary::GetStartingTransform(UObject* WorldContextObject, FString AgentName)
{
	TArray<AAgentStart*> ValidStarts;
	for (TActorIterator<AAgentStart> It(WorldContextObject->GetWorld()); It; ++It)
	{
		AAgentStart* AgentStart = *It;
		if (IsValid(AgentStart))
		{
			if (AgentStart->AgentName.Equals(AgentName, ESearchCase::IgnoreCase))
			{
				ValidStarts.Add(AgentStart);
			}
		}
	}

	if (ValidStarts.Num() == 0)
	{
		check(false && "Each agent needs to have unique AAgentStart!");
		return FTransform();
	}
	
	// Get a random index within the bounds of the array
	const int32 RandomIndex = FMath::RandRange(0, ValidStarts.Num() - 1);

	FTransform InstanceTransform = ValidStarts[RandomIndex]->GetTransform();
	InstanceTransform.SetScale3D(FVector::OneVector);

	return InstanceTransform;
}


void UMLUtilsFunctionLibrary::ConvertToGrayscale(const TArray<FColor>& Colors, TArray<uint8>& OutGrayscale)
{
	OutGrayscale.Empty();
	OutGrayscale.Reserve(Colors.Num());

	for (const FColor& Color : Colors)
	{
		/*
		FLinearColor LinearColor = Color.ReinterpretAsLinear();

		float Red = LinearColor.R * 0.2989f;
		float Green = LinearColor.G * 0.5870f;
		float Blue = LinearColor.B * 0.1140f;

		uint8 GrayValue = static_cast<uint8>(Red + Green + Blue);
		OutGrayscale.Add(GrayValue);
		*/
		// WORKING kinda
		// Convert RGB to grayscale using the weighted average method
		uint8 GrayValue = static_cast<uint8>(0.299 * Color.R + 0.587 * Color.G + 0.114 * Color.B);
		OutGrayscale.Add(GrayValue);
	}
}

float UMLUtilsFunctionLibrary::CalculateDistanceAlongAxis(const FVector& PositionA, const FVector& PositionB, const FVector& Axis)
{
	// Normalize the axis to ensure it has a length of 1.
	FVector NormalizedAxis = Axis.GetSafeNormal();

	// Calculate the vector between PositionA and PositionB.
	FVector DeltaPosition = PositionB - PositionA;

	// Project DeltaPosition onto the normalized axis.
	float DistanceAlongAxis = FVector::DotProduct(DeltaPosition, NormalizedAxis);

	return DistanceAlongAxis;
}
