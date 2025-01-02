// Copyright Epic Games, Inc. All Rights Reserved.

#include "CybertoothMLCommands.h"

#define LOCTEXT_NAMESPACE "FMyWindowPluginModule"

void FCybertoothMLCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "CybertoothML", "Bring up MyWindowPlugin window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
