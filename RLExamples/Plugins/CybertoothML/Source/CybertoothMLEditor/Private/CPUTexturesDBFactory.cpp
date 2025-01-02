// Fill out your copyright notice in the Description page of Project Settings.


#include "CPUTexturesDBFactory.h"
#include "CybertoothML/CPUTexturesDB.h"
#include "CPUTexturesDBActions.h"
#include <IAssetTools.h>
#include <Modules/ModuleManager.h>

UCPUTexturesDBFactory::UCPUTexturesDBFactory() 
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UCPUTexturesDB::StaticClass();
}

UObject* UCPUTexturesDBFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto MyCPUTexturesDB = NewObject<UCPUTexturesDB>(InParent, InClass, InName, Flags);
	return MyCPUTexturesDB;
}
