// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Sensors/CPURendererComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "JsonObjectConverter.h"
#include "CybertoothML/Components/MLAgentCoreComponent.h"
#include "CybertoothML/Utils/MLUtilsFunctionLibrary.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "CybertoothML/CybertoothMLSettings.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "CybertoothML/Components/Sensors/MLCameraSensorComponent.h"
#include "../../../Public/CybertoothML/RLManager.h"

int32 CPURendererDebugRender = 0;
FAutoConsoleVariableRef CVarCPURendererDebugRender(
	TEXT("ML.CPURenderer.DebugRender"),
	CPURendererDebugRender, TEXT("1 enables updating the CPU render to a texture target."));


// Sets default values for this component's properties
UCPURendererComponent::UCPURendererComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;

	NoTraceColor = FColor(153, 204, 255);
}


// Called when the game starts
void UCPURendererComponent::BeginPlay()
{
	Super::BeginPlay();

	CPURender.Empty(ResolutionWidth * ResolutionHeight);
	CPURender.AddUninitialized(ResolutionWidth * ResolutionHeight);

}


void UCPURendererComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RenderTick();
}


void UCPURendererComponent::RenderTick()
{
	TArray<AActor*> IgnoreActors;
	if (!bRenderOwner)
	{
		IgnoreActors.Add(GetOwner());
	}

	FVector CameraLocation = GetComponentLocation();
	FRotator CameraRotation = GetComponentRotation();

	RenderSingleFrame(this, CameraLocation, CameraRotation, ResolutionWidth, ResolutionHeight, MaxTraceDistance, FieldOfView, false, bSRGB, NoTraceColor, IgnoreActors, CPURender);
	if (bGrayscale)
	{
		UMLUtilsFunctionLibrary::ConvertToGrayscale(CPURender, CPURender_Grayscale);
	}


	URLManager* RLManager = URLManager::Get(this);
	if (CPURendererDebugRender
		|| (RLManager && RLManager->IsHumanModeEnabled())
		|| (RLManager->GetDebuggedAgent() && RLManager->GetDebuggedAgent()->GetOwner() == GetOwner())
		)
	{
		// Update debug rendering only when owner is locally controlled pawn
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (OwnerPawn && OwnerPawn->IsLocallyControlled())
		{
			if (bGrayscale)
			{
				UpdateTextureFromGrayscale(CPURender_Grayscale, ResolutionWidth, ResolutionHeight);
			}
			else {
				UpdateTextureFromBGRA(CPURender.GetData(), ResolutionWidth, ResolutionHeight);
			}
		}
	}
}


void UCPURendererComponent::RenderSingleFrame(const UObject* WorldContextObject, FVector CameraLocation, FRotator CameraRotation, int32 Width, int32 Height, float MaxTraceDistance, float FieldOfView, bool bUseBestLOD, bool bSRGB, FColor NoTraceColor, TArray<AActor*> IgnoreActors, TArray<FColor>& OutColors)
{
	SCOPED_NAMED_EVENT(UCPURendererComponent_RenderSingleFrame, FColor::Green);

	// Check that the size hasn't changed
	if (OutColors.Num() != Width * Height)
	{
		OutColors.Empty(Width * Height);
		OutColors.AddUninitialized(Width * Height);
	}

	const UCybertoothMLSettings* CoreSettings = GetDefault<UCybertoothMLSettings>();
	UCPUTexturesDB* CPUTextureDB = CoreSettings->CPUTexturesDB.LoadSynchronous();

	
	static const FName LineTraceSingleName(TEXT("CPURendererTrace"));

	// Set up aspect ratio and clip planes
	float AspectRatio = (float)Width / (float)Height;
	float FarClipPlane = MaxTraceDistance;

	FMinimalViewInfo MinimalViewInfo;
	MinimalViewInfo.Location = CameraLocation;
	MinimalViewInfo.Rotation = CameraRotation;
	MinimalViewInfo.FOV = FieldOfView;
	MinimalViewInfo.DesiredFOV = 90.0f;
	MinimalViewInfo.AspectRatio = AspectRatio;
	MinimalViewInfo.bConstrainAspectRatio = true;

	FMatrix ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;
	UGameplayStatics::GetViewProjectionMatrix(MinimalViewInfo, ViewMatrix, ProjectionMatrix, ViewProjectionMatrix);

	FMatrix InvProjectionMatrix = ProjectionMatrix.Inverse();
	FMatrix InvViewMatrix = ViewMatrix.Inverse();

	float StepDeltaWidth = FieldOfView / Width;
	float StepDeltaHeight = FieldOfView / Height;

	// Calculate sRGB color of NoTraceColor
	FColor CurrentNoTraceColor = NoTraceColor;
	if (bSRGB)
	{
		CurrentNoTraceColor = FLinearColor(NoTraceColor).ToFColor(false);
	}


	for (int32 i = 0; i < Width; i++)
	{
		for (int32 j = 0; j < Height; j++)
		{
			int32 OutputIdx = j * Width + i;

			float ScreenX = ((float)i / (float)Width) * 2.0f - 1.0f;
			float ScreenY = 1.0f - ((float)j / (float)Height) * 2.0f; // Flip the Y-axis here

			FVector4 ScreenPosition = FVector4(ScreenX, ScreenY, 1.0f, 1.0f);

			// Convert screen coordinates to world coordinates
			FVector4 ClipSpacePosition = InvProjectionMatrix.TransformPosition(ScreenPosition);
			ClipSpacePosition /= ClipSpacePosition.W;

			FVector WorldDirection = FVector(ClipSpacePosition.X, ClipSpacePosition.Y, ClipSpacePosition.Z);
			WorldDirection = InvViewMatrix.TransformVector(WorldDirection);
			WorldDirection.Normalize();

			FVector TraceEnd = CameraLocation + WorldDirection * MaxTraceDistance;

			bool ComplexTrace = true;
			FHitResult HitResult;
						
			FCollisionQueryParams CollisionParams(LineTraceSingleName, SCENE_QUERY_STAT_ONLY(CPURenderer), ComplexTrace);
			CollisionParams.bReturnPhysicalMaterial = true;
			CollisionParams.AddIgnoredActors(IgnoreActors);
			CollisionParams.bReturnFaceIndex = true;

			bool bHit = false;
			{
				SCOPED_NAMED_EVENT(UCPURendererComponent_TraceLine, FColor::Red);

				bHit = WorldContextObject->GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParams);
			}

			//bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), CameraLocation, TraceEnd, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), true, IgnoreActors, EDrawDebugTrace::None, HitResult, true, FLinearColor::Red);

			if(bHit)
			{
				SCOPED_NAMED_EVENT(UCPURendererComponent_InHit, FColor::Red);

				// log HitResult.FaceIndex
				// UE_LOG(LogTemp, Warning, TEXT("HitResult.FaceIndex: %d"), HitResult.FaceIndex);

				
				// Get hit material
				// This was a perfect solution but no longer works in 5.5, we fallback to using single material
				
				int32 MaterialSectionIndex;
				UMaterialInterface* HitMaterial = HitResult.GetComponent()->GetMaterialFromCollisionFaceIndex(HitResult.FaceIndex, MaterialSectionIndex);

				// print FPlatformProperties::RequiresCookedData()
				
				// UMaterialInterface* HitMaterial = nullptr;
				// UStaticMeshComponent* HitMeshComponent = Cast<UStaticMeshComponent>(HitResult.GetComponent());
				// if (HitMeshComponent)
				// {
				// 	HitMaterial = HitMeshComponent->GetMaterial(0);
				// }
				

				// Get hit UV point on the material
				FVector2D UV;
				UGameplayStatics::FindCollisionUV(HitResult, 0, UV);

				OutColors[OutputIdx] = CPUTextureDB->GetPixelFromUV(HitMaterial, HitResult.Distance, UV.X, UV.Y, bUseBestLOD);
				OutColors[OutputIdx].A = 255;

				

				if (bSRGB)
				{
					FLinearColor LinearColor = OutColors[OutputIdx].ReinterpretAsLinear();

					// Exponent to raise each color component
					float Exponent = 1.0f / 0.4545f;

					// Raise each component of the FLinearColor to the exponent
					LinearColor.R = FMath::Pow(LinearColor.R, Exponent);
					LinearColor.G = FMath::Pow(LinearColor.G, Exponent);
					LinearColor.B = FMath::Pow(LinearColor.B, Exponent);

					// Convert the FLinearColor back to FColor
					FColor ResultColor = LinearColor.ToFColor(false);
					OutColors[OutputIdx] = ResultColor;
				}
			}
			else {
				OutColors[OutputIdx] = CurrentNoTraceColor;
			}
		}
	}

	
}


#pragma region IMLSensorInterface

FString UCPURendererComponent::GetMLName_Implementation()
{
	return TEXT("camera_cpu");
}

void UCPURendererComponent::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	FCameraSensorConfig CameraConfig;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &CameraConfig))
	{
		bUseGPU = CameraConfig.use_gpu;
		// Never use GPU with nullrhi
		check(!bUseGPU || (bUseGPU && !GUsingNullRHI));

		bRenderOwner = CameraConfig.render_owner;
		FieldOfView = CameraConfig.fov;

		ResolutionWidth = CameraConfig.width;
		ResolutionHeight = CameraConfig.height;
		MaxTraceDistance = CameraConfig.max_distance;
		bSRGB = CameraConfig.srgb;
		bGrayscale = CameraConfig.grayscale;

		// Resize the CPU render buffer
		CPURender.Empty(ResolutionWidth * ResolutionHeight);
		CPURender.AddUninitialized(ResolutionWidth * ResolutionHeight);

		// We need to attach it to either component a different actor
		check(CameraConfig.attach_to_comp_name.IsEmpty() == false || CameraConfig.attach_to_actor_name.IsEmpty() == false);

		// Attaching to correct component
		if (CameraConfig.attach_to_comp_name.IsEmpty() == false)
		{
			// We specified a component to attach to
			USceneComponent* TargetComp = UMLUtilsFunctionLibrary::FindComponentByName(GetOwner(), CameraConfig.attach_to_comp_name);
			check(TargetComp);
			AttachToComponent(TargetComp, FAttachmentTransformRules::KeepRelativeTransform);
		}
		else {
			check(CameraConfig.attach_to_actor_name.IsEmpty() == false);
			AActor* ParentActor = UMLUtilsFunctionLibrary::GetActorByName(this, CameraConfig.attach_to_actor_name);
			check(ParentActor);
			AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
	else {
		check(false && "Failed to get Camera config from JSON.")
	}
}

void UCPURendererComponent::GetObservation_Implementation(FSensorObservation& OutData)
{
	if (bGrayscale)
	{
		// Get the number of colors in CPURender
		int32 NumColors = CPURender_Grayscale.Num();
		// Resize OutData to fit the number of bytes needed (RGBA = 4 bytes per color)
		OutData.Data.SetNumUninitialized(NumColors * sizeof(uint8));
		// Copy color data from CPURender to OutData using memcpy
		FMemory::Memcpy(OutData.Data.GetData(), CPURender_Grayscale.GetData(), NumColors * sizeof(uint8));
	}
	else {
		// Get the number of colors in CPURender
		int32 NumColors = CPURender.Num();
		// Resize OutData to fit the number of bytes needed (RGBA = 4 bytes per color)
		OutData.Data.SetNumUninitialized(NumColors * sizeof(FColor));
		// Copy color data from CPURender to OutData using memcpy
		FMemory::Memcpy(OutData.Data.GetData(), CPURender.GetData(), NumColors * sizeof(FColor));
	}
	
}

void UCPURendererComponent::Reset_Implementation()
{
	CPURender.Empty(ResolutionWidth * ResolutionHeight);
	CPURender.AddUninitialized(ResolutionWidth * ResolutionHeight);
}


void UCPURendererComponent::OnPostReset_Implementation()
{
	RenderTick();
}

FString UCPURendererComponent::GetDebugString_Implementation()
{
	return "";
}

UTexture* UCPURendererComponent::GetDebugTexture_Implementation()
{
	return CpuRenderTexture;
}

#pragma endregion IMLSensorInterface


void UCPURendererComponent::UpdateTextureFromBGRA(FColor* data, int width, int height)
{
	if (CpuRenderTexture == nullptr) {
		CpuRenderTexture = UTexture2D::CreateTransient(width, height, PF_B8G8R8A8);
		if (!CpuRenderTexture)
		{
			return;
		}

#if WITH_EDITORONLY_DATA
		CpuRenderTexture->MipGenSettings = TMGS_NoMipmaps;
#endif
		CpuRenderTexture->NeverStream = true;

		CpuRenderTexture->SRGB = 1;
	}

	FTexture2DMipMap& Mip = CpuRenderTexture->GetPlatformData()->Mips[0];
	void* OutData = Mip.BulkData.Lock(LOCK_READ_WRITE);

	FMemory::Memcpy(OutData, data, width * height * 4);
	Mip.BulkData.Unlock();
	CpuRenderTexture->UpdateResource();
}



void UCPURendererComponent::UpdateTextureFromGrayscale(TArray<uint8>& InBytesData, int width, int height)
{
	
	if (CpuRenderTexture == nullptr) {
		CpuRenderTexture = UTexture2D::CreateTransient(width, height, PF_B8G8R8A8);
		if (!CpuRenderTexture)
		{
			return;
		}

#if WITH_EDITORONLY_DATA
		CpuRenderTexture->MipGenSettings = TMGS_NoMipmaps;
#endif
		CpuRenderTexture->NeverStream = true;

		CpuRenderTexture->SRGB = 1;
	}
		
	TArray<FColor> GrayscaleColors;
	GrayscaleColors.AddUninitialized(InBytesData.Num());
	for (int32 i = 0; i < InBytesData.Num(); i++)
	{
		uint8 GrayValue = InBytesData[i];
		GrayscaleColors[i] = FColor(GrayValue, GrayValue, GrayValue, 255);
	}
		

	FTexture2DMipMap& Mip = CpuRenderTexture->GetPlatformData()->Mips[0];
	void* OutData = Mip.BulkData.Lock(LOCK_READ_WRITE); 

	FMemory::Memcpy(OutData, GrayscaleColors.GetData(), width * height * 4);
	Mip.BulkData.Unlock();
	CpuRenderTexture->UpdateResource();
}

FString UCPURendererComponent::GetCPURenderBase64()
{
	return FBase64::Encode((uint8*)CPURender.GetData(), ResolutionWidth * ResolutionHeight * 4);
}



void UCPURendererComponent::RenderFrameToFile(const UObject* WorldContextObject, FVector CameraLocation, FRotator CameraRotation, int32 Width, int32 Height, float FOV, bool bUseBestLOD, const FString& Filename)
{
	TArray<FColor> PixelData;
	TArray<AActor*> IgnoreActors;
	bool sRGB = true;

	FColor NoTraceColor = UCPURendererComponent::StaticClass()->GetDefaultObject<UCPURendererComponent>()->NoTraceColor;

	RenderSingleFrame(WorldContextObject, CameraLocation, CameraRotation, Width, Height, 50'000, FOV, bUseBestLOD, sRGB, NoTraceColor, IgnoreActors, PixelData);


	// Create an image wrapper for the pixel data
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	// Set the image data
	ImageWrapper->SetRaw(PixelData.GetData(), PixelData.Num() * sizeof(FColor), Width, Height, ERGBFormat::BGRA, 8);

	// Encode the image data to a file
	TArray<uint8, FDefaultAllocator64> EncodedImage = ImageWrapper->GetCompressed();


	FString SavedFolderPath = FPaths::ProjectSavedDir();
	FString FilePath = FPaths::Combine(SavedFolderPath, Filename);


	FFileHelper::SaveArrayToFile(EncodedImage, *FilePath);
}
