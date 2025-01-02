// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Components/Sensors/MLCameraGPUComp.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "CybertoothML/CybertoothMLSettings.h"
#include "Kismet/GameplayStatics.h"
#include "CybertoothML/Components/Sensors/MLCameraSensorComponent.h"
#include "CybertoothML/Utils/MLUtilsFunctionLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"

UMLCameraGPUComp::UMLCameraGPUComp(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
	
	bCaptureEveryFrame = !bPreciseMode;
	bCaptureOnMovement = !bPreciseMode;

	NoTraceColor = FColor(153, 204, 255);

	CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	
	ShowFlags.SetAtmosphere(false);
	ShowFlags.SetDecals(false);

	ShowFlags.SetAntiAliasing(false);
	ShowFlags.SetBSP(false);
	ShowFlags.SetCloud(false);
	ShowFlags.SetFog(false);
	ShowFlags.SetLandscape(false);
	ShowFlags.SetParticles(false);
	
	ShowFlags.SetDeferredLighting(false);
	ShowFlags.SetInstancedFoliage(false);
	ShowFlags.SetInstancedGrass(false);

	ShowFlags.SetBloom(false);
	ShowFlags.SetEyeAdaptation(false);

	ShowFlags.SetVolumetricFog(false);
	ShowFlags.SetVolumetricLightmap(false);
	ShowFlags.SetLighting(false);
}

void UMLCameraGPUComp::BeginPlay()
{
	Super::BeginPlay();

	// Create post-process material instance and apply it
	{
		const UCybertoothMLSettings* MLSettings = GetDefault<UCybertoothMLSettings>();

		// Get the material from UCybertoothMLSettings
		UMaterialInterface* PostProcessMatBase = MLSettings->AgentGPUCameraPostProcessMat.LoadSynchronous();

		// Create a dynamic material instance
		PostProcessMat = UMaterialInstanceDynamic::Create(PostProcessMatBase, this);

		PostProcessMat->SetScalarParameterValue("Grayscale", bGrayscale ? 1.0f : 0.0f);
		PostProcessMat->SetScalarParameterValue("MaxDistance", MaxTraceDistance);

		// Add the material instance to a camera component
		AddOrUpdateBlendable(PostProcessMat, 1.0f);
	}

	EnsureTextureExists();

	// Ignore all actors with ignore render tag
	const UCybertoothMLSettings* CoreSettings = GetDefault<UCybertoothMLSettings>();
	const FName IgnoreRenderTag = CoreSettings->ActorIgnoreRenderTag;

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), IgnoreRenderTag, AllActors);
	for (AActor* Actor : AllActors)
	{
		HiddenActors.Add(Actor);
	}

	// Ignore all actors with certain class
	for (TSoftClassPtr<AActor> IgnoreClassPtr : CoreSettings->IgnoreRenderClasses)
	{
		UClass* IgnoreClass = IgnoreClassPtr.LoadSynchronous();
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), IgnoreClass, AllActors);
		for (AActor* Actor : AllActors)
		{
			HiddenActors.Add(Actor);
		}
	}


}

void UMLCameraGPUComp::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bPreciseMode)
	{
		CaptureScene();
	}
}

#pragma region IMLSensorInterface

FString UMLCameraGPUComp::GetMLName_Implementation()
{
	return "camera_gpu";
}

void UMLCameraGPUComp::Configure_Implementation(const FJsonObjectWrapper& JsonConfig)
{
	EnsureTextureExists();

	FCameraSensorConfig CameraConfig;
	if (FJsonObjectConverter::JsonObjectToUStruct(JsonConfig.JsonObject.ToSharedRef(), &CameraConfig))
	{
		bUseGPU = CameraConfig.use_gpu;
		check(bUseGPU);
		check(!GUsingNullRHI);

		bRenderOwner = CameraConfig.render_owner;
		if (bRenderOwner == false && GetOwner())
		{
			HiddenActors.Add(GetOwner());
		}

		FOVAngle = CameraConfig.fov;

		ResolutionWidth = CameraConfig.width;
		ResolutionHeight = CameraConfig.height;

		TextureTarget->ResizeTarget(ResolutionWidth, ResolutionHeight);
		TextureTarget->UpdateResourceImmediate(true);

		MaxTraceDistance = CameraConfig.max_distance;
		PostProcessMat->SetScalarParameterValue("MaxDistance", MaxTraceDistance);

		// Not yet implemented
		bSRGB = CameraConfig.srgb;

		// Precise mode
		bCaptureEveryFrame = !bPreciseMode;
		bCaptureOnMovement = !bPreciseMode;

		// Render in grayscale
		bGrayscale = CameraConfig.grayscale;
		PostProcessMat->SetScalarParameterValue("Grayscale", bGrayscale ? 1.0f : 0.0f);
		
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

void UMLCameraGPUComp::EnsureTextureExists()
{
	if (TextureTarget == nullptr)
	{
		UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
		RenderTarget->InitCustomFormat(ResolutionWidth, ResolutionHeight, PF_B8G8R8A8, false);
		//RenderTarget->InitAutoFormat(128, 128);
		RenderTarget->ClearColor = FColor(153, 204, 255);
		RenderTarget->TargetGamma = 1.0f;
		RenderTarget->SRGB = false;
		RenderTarget->bAutoGenerateMips = false;
		RenderTarget->UpdateResourceImmediate(true);


		if (!RenderTarget->GetResource())
		{
			check(false);
			return;
		}

		TextureTarget = RenderTarget;
	}
}

void UMLCameraGPUComp::GetObservation_Implementation(FSensorObservation& OutData)
{
	SCOPED_NAMED_EVENT(UMLCameraGPUComp_GetObservation, FColor::Red);


	TArray<FColor> Pixels;
	{
		SCOPED_NAMED_EVENT(UMLCameraGPUComp_ReadRenderTarget, FColor::Red);
		UKismetRenderingLibrary::ReadRenderTarget(this, TextureTarget, Pixels, false);
	}
	if (bGrayscale)
	{
		// Copy only the Red channel to the output
		int32 PixelCount = Pixels.Num();
		OutData.Data.SetNumUninitialized(PixelCount);

		for (int32 i = 0; i < PixelCount; ++i)
		{
			OutData.Data[i] = Pixels[i].R;
		}
	}
	else {
		SCOPED_NAMED_EVENT(UMLCameraGPUComp_CopyData, FColor::Red);

		// Copy all pixels to output
		int32 ByteCount = Pixels.Num() * sizeof(FColor);
		OutData.Data.SetNumUninitialized(ByteCount);
		FMemory::Memcpy(OutData.Data.GetData(), Pixels.GetData(), ByteCount);
	}
	
}

void UMLCameraGPUComp::Reset_Implementation()
{
	UKismetRenderingLibrary::ClearRenderTarget2D(this, TextureTarget, TextureTarget->ClearColor);
}

void UMLCameraGPUComp::OnPostReset_Implementation()
{
	// TODO: Implement
	CaptureScene();
}

UTexture* UMLCameraGPUComp::GetDebugTexture_Implementation()
{
	return TextureTarget;
}
#pragma endregion IMLSensorInterface
