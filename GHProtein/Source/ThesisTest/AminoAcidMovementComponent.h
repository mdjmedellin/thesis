// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "AminoAcidMovementComponent.generated.h"

/**
 * 
 */
UCLASS( meta=( BlueprintSpawnableComponent ) )
class UAminoAcidMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

public:
	void TurnOffGravity();
};
