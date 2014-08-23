// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/StaticMeshActor.h"
#include "ThesisAminoAcidActor.generated.h"

/**
 * 
 */
UCLASS()
class AThesisAminoAcidActor : public AStaticMeshActor
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Tick(float DeltaTime);
};
