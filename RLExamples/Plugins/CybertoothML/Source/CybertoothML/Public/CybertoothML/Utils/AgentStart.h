// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AgentStart.generated.h"

UCLASS()
class CYBERTOOTHML_API AAgentStart : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAgentStart();

	UPROPERTY(Category = Components, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* SceneRoot;

	UPROPERTY(Category = Components, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* CapsuleComp;



	UPROPERTY(EditAnywhere, Category = "Defaults")
	FString AgentName;

	UPROPERTY(EditAnywhere, Category = "Defaults")
	FColor AgentColor;

	UPROPERTY(EditAnywhere, Category = "Defaults") //, meta = (AllowedClasses = "AvatarInterface")
	TSubclassOf<AActor> AvatarClass;



	/** Arrow component to indicate forward direction of start */
#if WITH_EDITORONLY_DATA
private:
	UPROPERTY(Category = Components, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* ForwardArrowComp;
public:
#endif



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
