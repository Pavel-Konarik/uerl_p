// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/CPUTexturesDB.h"
#include "CybertoothML/CybertoothMLSettings.h"
#include "Materials/MaterialInterface.h"

const FCPUTextureData* UCPUTexturesDB::GetCPUTexture(UMaterialInterface* Material)
{
	SCOPED_NAMED_EVENT(UCPUTexturesDB_GetCPUTexture, FColor::Purple);

	// Try finding the texture directly
	const FCPUTextureData* CPUTextureData = Data.Find(Material);
	if (CPUTextureData) {
		return CPUTextureData;
	}
	
	// Get the texture from texture cache
	const UMaterialInterface** KnownParentMat = MaterialCache.Find(Material);
	if (KnownParentMat)
	{
		// We have encountered this material before
		if (*KnownParentMat == nullptr)
		{
			// We found that this material has no valid Texture
			return nullptr;
		}
		
		// We have hit this material before, just look up the parent mat
		const FCPUTextureData* CPUTextureOfParent = Data.Find(*KnownParentMat);
		if (CPUTextureOfParent) {
			return CPUTextureOfParent;
		}
	}
	

	// We tried our best, but this is the first time we hit this material
	FMaterialInheritanceChain Chain;
	Material->GetMaterialInheritanceChain(Chain);

	for (const UMaterialInstance* CurrentMat : Chain.MaterialInstances)
	{
		CPUTextureData = Data.Find(CurrentMat);
		if (CPUTextureData) {

			// Cache this outcome
			MaterialCache.Add(Material, CurrentMat);
			UE_LOG(LogTemp, Warning, TEXT("CASHING MATERIAL"));

			return CPUTextureData;
		}
	}

	// We do not have this texture anywhere in the db, just remember it that it maps to nothing
	MaterialCache.Add(Material, nullptr);
	UE_LOG(LogTemp, Warning, TEXT("NO MATERIAL FOUND"));

	return nullptr;
}

FColor UCPUTexturesDB::GetPixelFromUV(UMaterialInterface* Material, float Distance, float U, float V, bool bUseBestLOD)
{
	SCOPED_NAMED_EVENT(UCPUTexturesDB_GetPixelFromUV, FColor::Blue);

	if (Material == nullptr)
	{
		return FColor::Black;
	}

	{
		SCOPED_NAMED_EVENT(UCPUTexturesDB_FirstHalf, FColor::Red);

	
		const FCPUTextureData* CPUTextureData = GetCPUTexture(Material);
		if (CPUTextureData == nullptr)
		{
			return FColor::Green;
		}

		{
			SCOPED_NAMED_EVENT(UCPUTexturesDB_SecondHalf, FColor::Orange);

			U = FMath::Frac(U);
			V = FMath::Frac(V);

			// Get correct LOD
			int32 LODIndex = bUseBestLOD ? 0 : GetLODLevelFromDistance(Distance);
			LODIndex = FMath::Clamp(LODIndex, 0, CPUTextureData->LODs.Num() - 1);

			int32 LODWidth = CPUTextureData->LODs[LODIndex].Width;
			int32 LODHeight = CPUTextureData->LODs[LODIndex].Height;


			int32 U_int = U * LODWidth;
			int32 V_int = V * LODHeight;

			int32 TargetIdx = V_int * LODWidth + U_int;
			if (CPUTextureData->LODs[LODIndex].Colors.IsValidIndex(TargetIdx))
			{
				return CPUTextureData->LODs[LODIndex].Colors[TargetIdx];
			}
		}
	}

	return FColor::Red;
}

int32 UCPUTexturesDB::GetLODLevelFromDistance(float Distance) const
{
	const UCybertoothMLSettings* CoreSettings = GetDefault<UCybertoothMLSettings>();
	
	for (int32 i = 0; i < CoreSettings->LODsDistances.Num(); i++)
	{
		if (Distance < CoreSettings->LODsDistances[i])
		{
			return i;
		}
	}

	return 4;
}

void UCPUTexturesDB::PrintDebug()
{
	for (const auto& Entry : Data)
	{
		const TSoftObjectPtr<UMaterialInterface>& Material = Entry.Key;
		const FCPUTextureData& CPUTextureData = Entry.Value;

		// Print material name
		FString MaterialName = Material.GetAssetName();
		UE_LOG(LogTemp, Warning, TEXT("Material: %s"), *MaterialName);

		// Print LODs information
		int32 LODIndex = 0;
		for (const FTextureLOD& LOD : CPUTextureData.LODs)
		{
			UE_LOG(LogTemp, Warning, TEXT("   LOD %d:  Width: %d, Height: %d, Colors: %d"), LODIndex, LOD.Width, LOD.Height, LOD.Colors.Num());
			LODIndex++;
		}
	}
}

void UCPUTexturesDB::ClearDatabase()
{
	Data.Empty();

	UPackage* Package = GetOutermost();
	if (Package)
	{
		Package->MarkPackageDirty();
	}
}
