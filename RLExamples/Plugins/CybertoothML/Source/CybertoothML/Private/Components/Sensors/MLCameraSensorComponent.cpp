// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Sensors/MLCameraSensorComponent.h"
#include "CybertoothML/RLManager.h"

// Sets default values for this component's properties
UMLCameraSensorComponent::UMLCameraSensorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


#pragma region IMLSensorInterface

FString UMLCameraSensorComponent::GetMLName_Implementation()
{
	return TEXT("camera");
}

void UMLCameraSensorComponent::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FCameraSensorConfig CameraConfig;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &CameraConfig))
	{
		// Check if we can use GPU or CPU
		bool bUseGPU = CameraConfig.use_gpu;
		if (bUseGPU && GUsingNullRHI)
		{
			UE_LOG(LogTemp, Warning, TEXT("UMLCameraSensorComponent::Configure - Requested to use GPU camera sensor, but env is using NullRHI. Switching to CPU rendering."));
			bUseGPU = false;
		}

		// Get the correct CPU or GPU sensor class
		URLManager* RLManager = URLManager::Get(this);
		FString CameraSensorClassName = bUseGPU ? "camera_gpu" : "camera_cpu";
		UClass* CameraSensorClass = RLManager->GetSensorClass(CameraSensorClassName);
		check(CameraSensorClass);

		// Spawn the component
		CameraSensor = NewObject<UActorComponent>(GetOwner(), CameraSensorClass);
		check(CameraSensor);
		CameraSensor->RegisterComponent();

		// This component MUST implement the specified interface
		check(CameraSensor->GetClass()->ImplementsInterface(UMLSensorInterface::StaticClass()));

		// Actually configure the component
		IMLSensorInterface::Execute_Configure(CameraSensor, JsonConfig);
	}
	else {
		check(false && "Failed to get Camera config from JSON.");
	}
}

void UMLCameraSensorComponent::GetObservation_Implementation(FSensorObservation& OutData)
{
	IMLSensorInterface::Execute_GetObservation(CameraSensor, OutData);
}

void UMLCameraSensorComponent::Reset_Implementation()
{
	IMLSensorInterface::Execute_Reset(CameraSensor);
}

void UMLCameraSensorComponent::OnPostReset_Implementation()
{
	IMLSensorInterface::Execute_OnPostReset(CameraSensor);
}

FString UMLCameraSensorComponent::GetDebugString_Implementation()
{
	return IMLSensorInterface::Execute_GetDebugString(CameraSensor);
}

UTexture* UMLCameraSensorComponent::GetDebugTexture_Implementation()
{
	return IMLSensorInterface::Execute_GetDebugTexture(CameraSensor);
}

#pragma endregion IMLSensorInterface