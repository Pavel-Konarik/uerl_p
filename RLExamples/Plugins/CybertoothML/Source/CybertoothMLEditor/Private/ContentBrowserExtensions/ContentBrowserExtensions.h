// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Integrate CybertoothML actions associated with existing engine types
class FMLContentBrowserExtensions
{
public:
	static void InstallHooks();
	static void RemoveHooks();
};
