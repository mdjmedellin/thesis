// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once 

#include "ThesisTestHUD.generated.h"

UCLASS()
class AThesisTestHUD : public AHUD
{
	GENERATED_UCLASS_BODY()

public:

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	void ActivateCrosshair();
	void DeactivateCrosshair();

private:
	/** Crosshair asset pointer */
	UTexture2D* CrosshairTex;
	
	/** Crosshair color helps us identify if we are in range of picking up items*/
	FLinearColor CrosshairColor;
	FLinearColor DeactiveColor;
	FLinearColor ActiveColor;
};

