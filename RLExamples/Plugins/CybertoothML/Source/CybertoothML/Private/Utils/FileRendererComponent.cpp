// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Utils/FileRendererComponent.h"
#include "CybertoothML/Server/ServerWebSocketSubsystem.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Misc/Base64.h"
/*

void UFileRendererComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bHideOwner)
	{
		HiddenActors.Add(GetOwner());
	}
	
	// DEBUG
	ETextureRenderTargetFormat RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
	RenderTarget2D = NewObject<UTextureRenderTarget2D>(this);
	check(RenderTarget2D);
	RenderTarget2D->RenderTargetFormat = RenderTargetFormat;

	int32 Width = 256;
	FParse::Value(FCommandLine::Get(), TEXT("video_width"), Width);

	int32 Height = 256;
	FParse::Value(FCommandLine::Get(), TEXT("video_height"), Height);

	RenderTarget2D->InitAutoFormat(Width, Height);
	RenderTarget2D->UpdateResourceImmediate(true);
	TextureTarget = RenderTarget2D;

}

void UFileRendererComponent::ResetRenderTarget()
{
	const bool bOutVideo = FParse::Param(FCommandLine::Get(), TEXT("outvideo"));
	if (bOutVideo && IsActive())
	{
		if (RenderTarget2D)
		{
			UKismetRenderingLibrary::ClearRenderTarget2D(this, RenderTarget2D, RenderTarget2D->ClearColor);
		}
	}
}


void UFileRendererComponent::RenderAgentStepToFile(FString AgentName)
{
	/ *
	const bool bOutVideo = FParse::Param(FCommandLine::Get(), TEXT("outvideo"));
	if (bOutVideo && IsActive())
	{
		AMLGameMode* GameMode = Cast<AMLGameMode>(UGameplayStatics::GetGameMode(this));
		check(GameMode);

		UServerWebSocketSubsystem* WebsocketServer = GetWorld()->GetGameInstance()->GetSubsystem<UServerWebSocketSubsystem>();
		FString path = FString::Printf(TEXT("./videos/%s/%s/%s/%09d.png"), *WebsocketServer->RunName, *WebsocketServer->EpisodeStartTime.ToString(), *AgentName, GameMode->EpisodeStepCount);

		// Get the texture's size
		FIntPoint Size = FIntPoint(RenderTarget2D->SizeX, RenderTarget2D->SizeY);

		// Read the pixel data from the render target
		TArray<FColor> PixelData;
		FRenderTarget* RenderTargetResource = RenderTarget2D->GameThread_GetRenderTargetResource();
		RenderTargetResource->ReadPixels(PixelData);

		// Create an image wrapper for the pixel data
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

		// Set the image data
		ImageWrapper->SetRaw(PixelData.GetData(), PixelData.Num() * sizeof(FColor), Size.X, Size.Y, ERGBFormat::BGRA, 8);

		// Encode the image data to a file
		const TArray<uint8, FDefaultAllocator64>& EncodedImage = ImageWrapper->GetCompressed();
		FFileHelper::SaveArrayToFile(EncodedImage, *path);
	}
	* /

	UServerWebSocketSubsystem* WebsocketServer = GetWorld()->GetGameInstance()->GetSubsystem<UServerWebSocketSubsystem>();
	bool bShouldRender = WebsocketServer && WebsocketServer->ShouldRenderVideo();
	if (bShouldRender)
	{
		TArray<uint8, FDefaultAllocator64> EncodedImage = RenderAgentStep();
		AMLGameMode* GameMode = Cast<AMLGameMode>(UGameplayStatics::GetGameMode(this));
		FString path = FString::Printf(TEXT("./videos/%s/%s/%s/%09d.png"), *WebsocketServer->RunName, *WebsocketServer->EpisodeStartTime.ToString(), *AgentName, GameMode->EpisodeStepCount);
		FFileHelper::SaveArrayToFile(EncodedImage, *path);
	}

}

FString UFileRendererComponent::RenderAgentStepToBase64()
{
	TArray<uint8, FDefaultAllocator64> Frame = RenderAgentStep();
	return FBase64::Encode((uint8*)Frame.GetData(), Frame.Num());
}

TArray<uint8, FDefaultAllocator64> UFileRendererComponent::RenderAgentStep()
{
	UServerWebSocketSubsystem* WebsocketServer = GetWorld()->GetGameInstance()->GetSubsystem<UServerWebSocketSubsystem>();
	bool bShouldRender = WebsocketServer && WebsocketServer->ShouldRenderVideo();
	if (bShouldRender)
	{
		AMLGameMode* GameMode = Cast<AMLGameMode>(UGameplayStatics::GetGameMode(this));
		check(GameMode);

		// Get the texture's size
		FIntPoint Size = FIntPoint(RenderTarget2D->SizeX, RenderTarget2D->SizeY);

		// Read the pixel data from the render target
		TArray<FColor> PixelData;
		FRenderTarget* RenderTargetResource = RenderTarget2D->GameThread_GetRenderTargetResource();
		RenderTargetResource->ReadPixels(PixelData);

		// Create an image wrapper for the pixel data
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

		// Set the image data
		ImageWrapper->SetRaw(PixelData.GetData(), PixelData.Num() * sizeof(FColor), Size.X, Size.Y, ERGBFormat::BGRA, 8);

		// Encode the image data to a file
		return ImageWrapper->GetCompressed();
	}
	return TArray<uint8, FDefaultAllocator64>();

}
*/
