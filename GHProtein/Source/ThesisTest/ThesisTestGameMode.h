// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "ThesisTestGameMode.generated.h"

UCLASS(minimalapi)
class AThesisTestGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage);
};



