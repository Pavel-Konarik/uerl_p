// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "CybertoothML/CPUTexturesDB.h"




class FCPUTexturesDBActions : public FAssetTypeActions_Base
{
public:
	FCPUTexturesDBActions(EAssetTypeCategories::Type InAssetCategory);

	virtual FColor GetTypeColor() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

	virtual void GetActions(const TArray<UObject*>& InObjects, struct FToolMenuSection& Section) override;

	void ExecuteCreateCPUData(TArray<TWeakObjectPtr<UTexture>> Objects);

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return FText::FromName(TEXT("CPUTexturesDB")); }
	virtual UClass* GetSupportedClass() const override { return UCPUTexturesDB::StaticClass(); }
	virtual uint32 GetCategories() override { return MyAssetCategory; }

private:
	EAssetTypeCategories::Type MyAssetCategory;
};
