// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "CybertoothMLPluginStyle.h"

class FCybertoothMLCommands : public TCommands<FCybertoothMLCommands>
{
public:

	FCybertoothMLCommands()
		: TCommands<FCybertoothMLCommands>(TEXT("CybertoothML"), NSLOCTEXT("Contexts", "CybertoothML", "CybertoothML Plugin"), NAME_None, FCybertoothMLPluginStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};