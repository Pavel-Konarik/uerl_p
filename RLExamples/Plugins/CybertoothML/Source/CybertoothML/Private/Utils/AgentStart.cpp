// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Utils/AgentStart.h"
#include "CybertoothML/Utils/MLUtilsFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"

// Sets default values
AAgentStart::AAgentStart()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->SetupAttachment(GetRootComponent());
	CapsuleComp->SetCapsuleHalfHeight(92.0f);
	CapsuleComp->SetCapsuleRadius(42.0f);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);


#if WITH_EDITORONLY_DATA
	ForwardArrowComp = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("ForwardArrowComp"));

	if (ForwardArrowComp)
	{
		ForwardArrowComp->ArrowColor = FColor(150, 200, 255);

		ForwardArrowComp->ArrowSize = 1.0f;
		ForwardArrowComp->bTreatAsASprite = true;
		ForwardArrowComp->SetupAttachment(GetRootComponent());
		ForwardArrowComp->bIsScreenSizeScaled = true;
	}

#endif // WITH_EDITORONLY_DATA	
}

// Called when the game starts or when spawned
void AAgentStart::BeginPlay()
{
	Super::BeginPlay();


}

// Called every frame
void AAgentStart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

