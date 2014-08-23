// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "ThesisTestHUD.h"

AThesisTestHUD::AThesisTestHUD(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshiarTexObj(TEXT("/Game/Textures/Crosshair"));
	CrosshairTex = CrosshiarTexObj.Object;
	DeactiveColor = FLinearColor::Red;
	ActiveColor = FLinearColor::Green;

	CrosshairColor = DeactiveColor;
}


void AThesisTestHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X - (CrosshairTex->GetSurfaceWidth() * 0.5)),
										   (Center.Y - (CrosshairTex->GetSurfaceHeight() * 0.5f)) );

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, CrosshairColor);
	TileItem.BlendMode = SE_BLEND_Opaque;
	Canvas->DrawItem( TileItem );
}

void AThesisTestHUD::ActivateCrosshair()
{
	CrosshairColor = ActiveColor;
}

void AThesisTestHUD::DeactivateCrosshair()
{
	CrosshairColor = DeactiveColor;
}

