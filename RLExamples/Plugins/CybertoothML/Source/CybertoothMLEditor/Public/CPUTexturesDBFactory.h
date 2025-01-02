// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "CPUTexturesDBFactory.generated.h"

/**
 * 
 */
UCLASS()
class CYBERTOOTHMLEDITOR_API UCPUTexturesDBFactory : public UFactory
{
	GENERATED_BODY()
		
public:
	UCPUTexturesDBFactory();

	// Initial texture to create the tile set from (Can be nullptr)
	class UTexture2D* InitialTexture;

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)override;
	

};
