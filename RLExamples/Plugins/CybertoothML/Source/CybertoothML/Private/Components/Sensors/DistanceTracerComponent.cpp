// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Sensors/DistanceTracerComponent.h"
#include "CybertoothML/Utils/MLUtilsFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"
#include "../../../Public/CybertoothML/RLManager.h"
#include "CybertoothML/Components/MLAgentCoreComponent.h"

int32 TracerDebugRender = 0;
FAutoConsoleVariableRef CVarDistanceTracerDebugRender(
	TEXT("ML.DistanceTracer.DebugRender"),
	TracerDebugRender, TEXT("1 enables updating the Distance Tracer a texture target."));

// Sets default values for this component's properties
UDistanceTracerComponent::UDistanceTracerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;

	bInvertValues = true;
}


// Called when the game starts
void UDistanceTracerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	TracesBuffer.Empty(NumTraces);
	TracesBuffer.AddUninitialized(NumTraces);
}


void UDistanceTracerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TraceLines();
}

#pragma region IMLSensorInterface

FString UDistanceTracerComponent::GetMLName_Implementation()
{
	return "distance_tracer";
}

void UDistanceTracerComponent::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FDistanceTracerConfig Config;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &Config))
	{
		NumTraces = Config.num_traces;
		HalfAngle = Config.half_angle;
		TraceLength = Config.trace_length;
		bShouldNormalise = Config.should_normalise;
		InitRotation = Config.rotation;
		InitLocation = Config.location;
		bInvertValues = Config.invert_values;

		TEnumAsByte<ETraceTypeQuery> QueryType;

		bool bTraceFound = UMLUtilsFunctionLibrary::ConvertFStringToTraceType(Config.trace_channel, QueryType);
		check(bTraceFound);

		TraceType = QueryType;

		TracesBuffer.Empty(NumTraces);
		TracesBuffer.AddUninitialized(NumTraces);

		AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		SetRelativeRotation(InitRotation);
		SetRelativeLocation(InitLocation);
	}
	else {
		check(false && "Failed to get Tracer config from JSON.")
	}
}

void UDistanceTracerComponent::GetObservation_Implementation(FSensorObservation& OutData)
{
	check(TracesBuffer.Num() == NumTraces);

	OutData.Data.SetNumUninitialized(NumTraces * sizeof(float));
	FMemory::Memcpy(OutData.Data.GetData(), TracesBuffer.GetData(), NumTraces * sizeof(float));
}

void UDistanceTracerComponent::OnPostReset_Implementation()
{
	TraceLines();
}

class UTexture2D* UDistanceTracerComponent::GetDebugTexture_Implementation()
{
	return DebugRenderTexture;
}

#pragma endregion IMLSensorInterface

void UDistanceTracerComponent::TraceLines()
{
	if (TracesBuffer.Num() != NumTraces)
	{
		TracesBuffer.Empty(NumTraces);
		TracesBuffer.AddUninitialized(NumTraces);
	}


	// Define variables
	const float DegreesBetweenTraces = (HalfAngle * 2.0f) / NumTraces; // Degrees between each trace
	const FVector StartLocation = GetComponentLocation(); // Starting location for traces
	const FVector ForwardVector = GetForwardVector(); // Forward vector for traces

	// Calculate the angle between each trace in radians
	const float RadiansBetweenTraces = FMath::DegreesToRadians(DegreesBetweenTraces);

	// Loop through each trace
	for (int32 i = 0; i < NumTraces; i++)
	{
		// Calculate the angle for this trace
		const float Angle = RadiansBetweenTraces * (i - NumTraces / 2);

		// Calculate the direction for this trace
		const FVector Direction = ForwardVector.RotateAngleAxis(FMath::RadiansToDegrees(Angle), FVector::UpVector);

		// Calculate the end location for this trace
		const FVector EndLocation = StartLocation + Direction * TraceLength;

		// Perform the trace
		FHitResult HitResult;
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(GetOwner());


		//GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel2);
		bool bTraceSuccessful = UKismetSystemLibrary::LineTraceSingle(
			GetWorld(),
			StartLocation,
			EndLocation,
			TraceType, // 
			true, // bTraceComplex
			ActorsToIgnore,
			TracerDebugRender ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None, // Show trace for one frame
			HitResult,
			true // bIgnoreSelf
		);

		// If we didn't hit, treat it as max distance
		TracesBuffer[i] = 1.0f;
		if (!bShouldNormalise)
		{
			TracesBuffer[i] = TraceLength;
		}

		if (HitResult.bBlockingHit)
		{	
			TracesBuffer[i] = FVector::Dist(HitResult.Location, StartLocation);
			if (bShouldNormalise)
			{
				TracesBuffer[i] = TracesBuffer[i] / TraceLength;
			}
		}

		if (bInvertValues)
		{	
			if (bShouldNormalise)
			{
				TracesBuffer[i] = 1.0f - TracesBuffer[i];
			}
			else {
				TracesBuffer[i] = TraceLength - TracesBuffer[i];
			}
		}

		check(TracesBuffer[i] >= 0.0f);
		check(TracesBuffer[i] <= TraceLength);
	}


	URLManager* RLManager = URLManager::Get(this);
	if ( TracerDebugRender 
		|| (RLManager && RLManager->IsHumanModeEnabled())
		|| (RLManager->GetDebuggedAgent() && RLManager->GetDebuggedAgent()->GetOwner() == GetOwner())
	)
	{
		//// Update debug rendering only when owner is locally controlled pawn
		//APawn* OwnerPawn = Cast<APawn>(GetOwner());
		//if (OwnerPawn && OwnerPawn->IsLocallyControlled())
		//{
		//	UpdateTextureFromTraces(TracesBuffer.Num(), 1);
		//}
		UpdateTextureFromTraces(TracesBuffer.Num(), 1);
	}
}





void UDistanceTracerComponent::UpdateTextureFromTraces(int width, int height)
{
	TArray<FColor> ColorRepresentations;
	ColorRepresentations.Empty(TracesBuffer.Num());
	for (float TraceDist : TracesBuffer)
	{
		uint8 TraceInt = static_cast<uint8>(FMath::Clamp(TraceDist * 255.0f, 0.0f, 255.0f));
		FColor TraceColor = FColor(TraceInt, TraceInt, TraceInt);
		ColorRepresentations.Add(TraceColor);
	}

	check(ColorRepresentations.Num() == TracesBuffer.Num());

	if (DebugRenderTexture == nullptr) {
		DebugRenderTexture = UTexture2D::CreateTransient(width, height, PF_B8G8R8A8);
		if (!DebugRenderTexture)
		{
			return;
		}

#if WITH_EDITORONLY_DATA
		DebugRenderTexture->MipGenSettings = TMGS_NoMipmaps;
#endif
		DebugRenderTexture->NeverStream = true;

		DebugRenderTexture->SRGB = 0;
	}

	FTexture2DMipMap& Mip = DebugRenderTexture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);

	FMemory::Memcpy(Data, ColorRepresentations.GetData(), width * height * 4);
	Mip.BulkData.Unlock();
	DebugRenderTexture->UpdateResource();
}


