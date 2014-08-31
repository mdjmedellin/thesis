// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "ThesisTestGameMode.generated.h"

namespace GHProtein
{
	class ProteinModel;
}

UCLASS(minimalapi)
class AThesisTestGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage);
	
	/** Transition from WaitingToStart to InProgress. You can call this manually, will also get called if ReadyToStartMatch returns true */
	virtual void StartMatch();

public:
	GHProtein::ProteinModel* m_proteinModel;
private:
	/* Private data members */
	UClass* DefaultAminoAcidClass;
};