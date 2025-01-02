// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Robot/RobotAgent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ARobotAgent::ARobotAgent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(ACharacter::MeshComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// set our turn rates for input
	BaseTurnRate = 75.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	
	RobotMesh = CreateDefaultSubobject<UStaticMeshComponent>("RobotMesh");
	if (RobotMesh)
	{
		RobotMesh->SetupAttachment(GetRootComponent());
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	if (SpringArm)
	{
		SpringArm->TargetArmLength = 870.0f;
		SpringArm->TargetOffset = FVector(0.0f, 0.0f, 508.210135f);
		SpringArm->SocketOffset = FVector(0.0f, 0.0f, 0.0f);
		SpringArm->bUsePawnControlRotation = true;

		SpringArm->SetupAttachment(GetRootComponent());
	}

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	if (CameraComp)
	{
		CameraComp->SetRelativeRotation(FRotator(-16.0f, 0.0f, 0.0f));
		CameraComp->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
		CameraComp->bUsePawnControlRotation = false;
	}
	
}

// Called when the game starts or when spawned
void ARobotAgent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARobotAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


#pragma region Input

// Called to bind functionality to input
void ARobotAgent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARobotAgent::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARobotAgent::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ARobotAgent::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ARobotAgent::LookUpAtRate);

}

void ARobotAgent::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARobotAgent::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


void ARobotAgent::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		//const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		const FVector Direction = GetActorForwardVector();

		AddMovementInput(Direction, Value);
	}
}

void ARobotAgent::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		//const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		const FVector Direction = GetActorRightVector();

		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

#pragma endregion Input