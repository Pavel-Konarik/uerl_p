// Copyright Epic Games, Inc. All Rights Reserved.

#include "CybertoothMLPluginStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FCybertoothMLPluginStyle::StyleInstance = nullptr;

void FCybertoothMLPluginStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FCybertoothMLPluginStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FCybertoothMLPluginStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("CybertoothMLPluginStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FCybertoothMLPluginStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("CybertoothMLPluginStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("CybertoothML")->GetBaseDir() / TEXT("Resources"));

	Style->Set("CybertoothML.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("MLWindowButtonIcon"), Icon20x20));

	return Style;
}

void FCybertoothMLPluginStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FCybertoothMLPluginStyle::Get()
{
	return *StyleInstance;
}
