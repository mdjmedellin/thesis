// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/ProjectileMovementComponent.h"
#include "AminoMovementComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class UAminoMovementComponent : public UProjectileMovementComponent
{
	GENERATED_UCLASS_BODY()

public:
	void ToggleGravity();

private:
	float m_defaultGravityScale;
};
