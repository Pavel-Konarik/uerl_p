// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/InfoProviders/SpectatorCameraComp.h"
#include "CybertoothML/Components/Sensors/MLCameraSensorComponent.h"

// Sets default values for this component's properties
USpectatorCameraComp::USpectatorCameraComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
}


// Called when the game starts
void USpectatorCameraComp::BeginPlay()
{
	Super::BeginPlay();

	
}


// Called every frame
void USpectatorCameraComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}

#pragma region IMLInfoProviderInterface

FString USpectatorCameraComp::GetMLName_Implementation()
{
	return "spectator_camera";
}


void USpectatorCameraComp::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	/*FPickupRewarderConfig RewarderConfig;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &RewarderConfig))
	{
		RewardInstanceName = RewarderConfig.name;
	}*/

	CameraSensor = NewObject<UMLCameraSensorComponent>(GetOwner(), UMLCameraSensorComponent::StaticClass());
	IMLSensorInterface::Execute_Configure(CameraSensor, JsonConfig);
}

void USpectatorCameraComp::GetInfo_Implementation(FSensorObservation& OutInfo)
{
	IMLSensorInterface::Execute_GetObservation(CameraSensor, OutInfo);
}

void USpectatorCameraComp::Reset_Implementation()
{
	IMLSensorInterface::Execute_Reset(CameraSensor);
}

void USpectatorCameraComp::OnPostReset_Implementation()
{
	IMLSensorInterface::Execute_OnPostReset(CameraSensor);
}

FString USpectatorCameraComp::GetDebugString_Implementation()
{
	return IMLSensorInterface::Execute_GetDebugString(CameraSensor);
}

#pragma endregion IMLRewarderInterface