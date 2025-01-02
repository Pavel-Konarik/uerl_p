// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/Tests/RenderToFileTest.h"
/*
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Blueprint/BlueprintSupport.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "IMaterialBakingModule.h"
#include "MaterialOptions.h"
#include "MaterialBakingStructures.h"
#include "MeshMergeHelpers.h"
#include "Engine/SkeletalMesh.h"
*/

// Sets default values
ARenderToFileTest::ARenderToFileTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}
// 
// // Called when the game starts or when spawned
// void ARenderToFileTest::BeginPlay()
// {
// 	Super::BeginPlay();
// 
// 	BakeMaterialsForMesh();
// }
// 
// // Called every frame
// void ARenderToFileTest::Tick(float DeltaTime)
// {
// 	Super::Tick(DeltaTime);
// 
// }
// 
// bool ARenderToFileTest::BakeMaterialsForMesh()
// {
// 	/*
// 	if (MeshToBake == nullptr)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("MeshToBake is null!"));
// 		return false;
// 	}
// 	*/
// 
// 	IMaterialBakingModule& MaterialBakingModule = FModuleManager::Get().LoadModuleChecked<IMaterialBakingModule>("MaterialBaking");
// 	
// 	
// 	FMeshData MeshSettings;
// 	
// 	FMaterialData MaterialSettings;
// 	MaterialSettings.Material = SkeletalMesh->GetMaterials()[0].MaterialInterface;
// 	MaterialSettings.PropertySizes.Add(EMaterialProperty::MP_BaseColor, FIntPoint(1024, 1024));
// 
// 	TArray<FMeshData*> MeshSettingPtrs{ &MeshSettings };
// 	TArray<FMaterialData*> MaterialSettingPtrs{ &MaterialSettings };
// 
// 	IMaterialBakingModule& Module = FModuleManager::Get().LoadModuleChecked<IMaterialBakingModule>("MaterialBaking");
// 
// 	TArray<FBakeOutput> BakeOutputs;
// 	Module.BakeMaterials(MaterialSettingPtrs, MeshSettingPtrs, BakeOutputs);
// 
// 	// Set the image data
// 	TArray<FColor> PixelData = BakeOutputs[0].PropertyData[EMaterialProperty::MP_BaseColor];
// 	for (int32 i = 0; i < PixelData.Num(); i++)
// 	{
// 		PixelData[i].A = 255;
// 	}
// 
// 
// 
// 	UE_LOG(LogTemp, Warning, TEXT("Here %d"), BakeOutputs.Num());
// 
// 	// Create an image wrapper for the pixel data
// 	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
// 	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
// 
// 	
// 
// 
// 	ImageWrapper->SetRaw(PixelData.GetData(), PixelData.Num() * sizeof(FColor), 1024, 1024, ERGBFormat::BGRA, 8);
// 
// 	// Encode the image data to a file
// 	TArray<uint8, FDefaultAllocator64> EncodedImage = ImageWrapper->GetCompressed();
// 
// 	FString Path = "Z:/Projects/UERL/UEProject/Saved/Temp/file.png";
// 	FFileHelper::SaveArrayToFile(EncodedImage, *Path);
// 
// 	return true;
// }
