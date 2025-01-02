// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class SMLUtilsWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMLUtilsWindow) {}
    SLATE_END_ARGS()

        void Construct(const FArguments& InArgs);

    FReply OnFindBlueprintsButtonClicked();
};