// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "ProteinModelSpawnPoint.h"
#include "ThesisTestGameMode.generated.h"

namespace GHProtein
{
	class ProteinModel;
}

class NeuralNetwork;
class AAminoAcid;

UCLASS(minimalapi)
class AThesisTestGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage);
	virtual void Tick(float DeltaSeconds) override;

	/** Transition from WaitingToStart to InProgress. You can call this manually, will also get called if ReadyToStartMatch returns true */
	virtual void StartMatch();

	virtual void AddProteinModelSpawnPoint(AProteinModelSpawnPoint* NewProteinModelSpawnPoint);
	virtual void RemoveProteinModelSpawnPoint(AProteinModelSpawnPoint* RemovedProteinModelSpawnPoint);

	AProteinModelSpawnPoint* GetBestProteinModelSpawnPoint(EProteinSpawnPointType::Type spawnType);

	GHProtein::ProteinModel* PredictSecondaryStructure(TArray<AAminoAcid*>& residues);

	UFUNCTION(BlueprintImplementableEvent, Category = "ProteinEvents")
		void FinishedProteinAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "ProteinEvents")
		void StartedProteinAnimation();

private:
	void GetInputVals(const TArray<AAminoAcid*>& residues, int residueIndex, int slidingWindowWidth,
		TArray< TArray<double> >& out_values);

	void FilterSecondaryStructures(TArray<AAminoAcid*>& residues);
	void ApplyChouFasmanAlgorithm(TArray<AAminoAcid*>& residues);

public:
	GHProtein::ProteinModel* m_proteinModel;
	GHProtein::ProteinModel* m_customChainModel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode)
		float m_aminoAcidSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode)
		FVector m_proteinModelCenterLocation;

	UPROPERTY(EditDefaultsOnly, Category = GameMode)
		UClass* DefaultAminoAcidClass;

	UPROPERTY(EditDefaultsOnly, Category = GameMode)
		UClass* DefaultHydrogenBondClass;

private:
	/* Private data members */
	UPROPERTY(EditDefaultsOnly, Category = GameMode)
		float m_linkWidth;

	UPROPERTY(EditDefaultsOnly, Category = GameMode)
		float m_linkHeight;

	UPROPERTY(EditDefaultsOnly, Category = GameMode)
		float m_distanceScale;

	UPROPERTY(EditDefaultsOnly, Category = ProteinModel)
		FColor m_normalColor;

	UPROPERTY(EditDefaultsOnly, Category = ProteinModel)
		FColor m_helixColor;

	UPROPERTY(EditDefaultsOnly, Category = ProteinModel)
		FColor m_betaStrandColor;

	UPROPERTY(EditDefaultsOnly, Category = ProteinModel)
		FColor m_hydrogenBondColor;

	UPROPERTY(EditDefaultsOnly, Category = ProteinModel)
		float m_betaStrandLinkWidth;

	UPROPERTY(EditDefaultsOnly, Category = ProteinModel)
		float m_helixLinkWidth;

	UPROPERTY(EditDefaultsOnly, Category = ProteinModel)
		float m_hydrogenBondLinkWidth;

	UPROPERTY(EditDefaultsOnly, Category = ProteinEnviroment)
		float m_startingTemperatureCelsius;

	UPROPERTY(EditDefaultsOnly, Category = ProteinEnviroment)
		float m_stableTemperatureCelsius;

	UPROPERTY(EditDefaultsOnly, Category = ProteinEnviroment)
		float m_meltingTemperatureCelsius;

	UPROPERTY(EditDefaultsOnly, Category = ProteinEnviroment)
		float m_irreversibleTemperatureCelsius;

	UPROPERTY(EditDefaultsOnly, Category = ProteinEnviroment)
		float m_temperatureStep;

	UPROPERTY(EditDefaultsOnly, Category = SecondaryStructurePrediction)
		FString m_predictionWeightsFileLocation;

	UPROPERTY()
		TArray<class AProteinModelSpawnPoint*> ProteinModelSpawnPoints;


		NeuralNetwork* m_predictionNeuralNetwork;
};