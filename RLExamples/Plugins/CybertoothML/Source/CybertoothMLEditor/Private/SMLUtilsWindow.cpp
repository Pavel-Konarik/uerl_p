
#include "SMLUtilsWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/BlueprintSupport.h"
#include "SMLUtilsWindow.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Blueprint/BlueprintSupport.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "ImageWriteBlueprintLibrary.h"



#define LOCTEXT_NAMESPACE "FindBlueprintsButton"

void SMLUtilsWindow::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		// Put your tab content here!
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("FindBlueprintsButton", "Find Blueprints"))
			.OnClicked(this, &SMLUtilsWindow::OnFindBlueprintsButtonClicked)
		]
	];
}

FReply SMLUtilsWindow::OnFindBlueprintsButtonClicked()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> OutAssetData;
	AssetRegistryModule.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), OutAssetData, true);

	UWorld* World = GEditor->GetEditorWorldContext().World();



	ETextureRenderTargetFormat RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	check(RenderTarget);
	RenderTarget->RenderTargetFormat = RenderTargetFormat;

	int32 Width = 256;
	int32 Height = 256;

	RenderTarget->InitAutoFormat(Width, Height);
	RenderTarget->UpdateResourceImmediate(true);

	/*
	// Create a render target that will be used to render out materials to
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	//RenderTarget->InitCustomFormat(256, 256, EPixelFormat::PF_FloatRGBA, true);
	RenderTarget->InitAutoFormat(256, 256);
	RenderTarget->UpdateResourceImmediate(true);
	*/

	for (const FAssetData& AssetData : OutAssetData) {
		UE_LOG(LogTemp, Warning, TEXT("Blueprint Name: %s"), *AssetData.AssetName.ToString());

		// Load the Blueprint asset
		UBlueprint* BlueprintAsset = Cast<UBlueprint>(AssetData.GetAsset());

		// Check if the Blueprint asset is valid
		if (BlueprintAsset)
		{
			// Get the Blueprint's Class Default Object (CDO)
			AActor* CDO = Cast<AActor>(BlueprintAsset->GeneratedClass->GetDefaultObject());
			if (CDO)
			{
				// Loop over all components of the CDO
				for (UActorComponent* Component : CDO->GetComponents())
				{
					// Check if the component is a UStaticMeshComponent
					UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component);
					if (MeshComponent)
					{
						// Loop through all the materials in the static mesh component
						for (int32 i = 0; i < MeshComponent->GetNumMaterials(); i++)
						{
							// Get the material
							UMaterialInterface* Material = MeshComponent->GetMaterial(i);
							// Check if the material is valid
							if (Material)
							{
								UE_LOG(LogTemp, Warning, TEXT("		- Material found: %s"), *Material->GetName());

								// Create a dynamic material instance from the given material
								UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, World);

								// Draw the material to the render target
								UKismetRenderingLibrary::DrawMaterialToRenderTarget(World, RenderTarget, DynamicMaterial);


								// Get the texture's size
								FIntPoint Size = FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY);

								// Read the pixel data from the render target
								TArray<FColor> PixelData;
								FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
								RenderTargetResource->ReadPixels(PixelData);

								// Create an image wrapper for the pixel data
								IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
								TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

								// Set the image data
								ImageWrapper->SetRaw(PixelData.GetData(), PixelData.Num() * sizeof(FColor), Size.X, Size.Y, ERGBFormat::BGRA, 8);

								// Encode the image data to a file
								TArray<uint8, FDefaultAllocator64> CompressedImg = ImageWrapper->GetCompressed();

								FString Path = "Z:/Projects/UERL/UEProject/Saved/Temp/file.png";

								FFileHelper::SaveArrayToFile(CompressedImg, *Path);
							}
						}
					}
				}
			}
		}
	}

	return FReply::Handled();
}



#undef LOCTEXT_NAMESPACE
