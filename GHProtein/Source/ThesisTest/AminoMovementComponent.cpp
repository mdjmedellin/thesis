// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "AminoMovementComponent.h"


UAminoMovementComponent::UAminoMovementComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	m_defaultGravityScale = ProjectileGravityScale;
	ProjectileGravityScale = 0.0;
}

void UAminoMovementComponent::ToggleGravity()
{
	ProjectileGravityScale = ProjectileGravityScale == 0.0 ? m_defaultGravityScale : 0.0;
}