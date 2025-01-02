// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CybertoothML/Components/MLInfoProviderInterface.h"
#include "SpectatorCameraComp.generated.h"




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERTOOTHML_API USpectatorCameraComp : public UActorComponent, public IMLInfoProviderInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpectatorCameraComp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	class UMLCameraSensorComponent* CameraSensor;

#pragma region IMLInfoProviderInterface
	FString GetMLName_Implementation() override;
	void Configure_Implementation(const FJsonObjectWrapper& JsonConfig) override;
	void GetInfo_Implementation(FSensorObservation& OutInfo);
	void Reset_Implementation() override;
	void OnPostReset_Implementation() override;
	FString GetDebugString_Implementation() override;
#pragma endregion IMLInfoProviderInterface
		
};
