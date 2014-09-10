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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode)
		float m_aminoAcidSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode)
		FVector m_proteinModelCenterLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameMode)
		UBlueprint* m_aminoAcidBlueprint;
private:
	/* Private data members */
	UPROPERTY(EditDefaultsOnly, Category = GameMode)
		UClass* DefaultAminoAcidClass;

	UPROPERTY(EditDefaultsOnly, Category = GameMode)
		float m_linkWidth;

	UPROPERTY(EditDefaultsOnly, Category = GameMode)
		float m_linkHeight;

	UPROPERTY(EditDefaultsOnly, Category = GameMode)
		float m_distanceScale;

	UPROPERTY(EditDefaultsOnly, Category = ProteinModel)
		FColor m_helixColor;

	UPROPERTY(EditDefaultsOnly, Category = ProteinModel)
		FColor m_betaStrandColor;

	UPROPERTY(EditDefaultsOnly, Category = ProteinModel)
		float m_betaStrandLinkWidth;

	UPROPERTY(EditDefaultsOnly, Category = ProteinModel)
		float m_helixLinkWidth;
};