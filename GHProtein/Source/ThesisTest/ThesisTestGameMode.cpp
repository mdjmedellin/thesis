// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "ThesisTestGameMode.h"
#include "ThesisStaticLibrary.h"
#include "ProteinUtilities.h"
#include "ThesisTestHUD.h"
#include "CameraPlayerController.h"
#include "ProteinBuilder.h"
#include "ProteinModel.h"
#include "AminoAcid.h"
#include "NeuralNetwork.h"
#include "ProteinModelSpawnPoint.h"

AThesisTestGameMode::AThesisTestGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
	, m_proteinModel(nullptr)
	, m_customChainModel(nullptr)
	, m_aminoAcidSize(0.f)
	, m_proteinModelCenterLocation(FVector(0.f,0.f,0.f))
	, DefaultAminoAcidClass(nullptr)
	, DefaultHydrogenBondClass(nullptr)
	, m_linkWidth(100.f)
	, m_linkHeight(100.f)
	, m_distanceScale(1.f)
	, m_helixColor(FColor::White)
	, m_betaStrandColor(FColor::White)
	, m_normalColor(FColor::White)
	, m_hydrogenBondColor(FColor::White)
	, m_helixLinkWidth(100.f)
	, m_betaStrandLinkWidth(100.f)
	, m_hydrogenBondLinkWidth(100.f)
	, m_predictionNeuralNetwork(nullptr)
{}

void AThesisTestGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	//Load the pdb file used to create the protein model and parse the data inside of it
	ProteinBuilder* localProteinBuilder = &ProteinBuilder::GetInstance();
	FString testString = "";
	bool debug_file = true;

	if (debug_file)
	{
		testString = "../../../ThesisData/Lysozyme.dssp";
	}
	else
	{
		testString = "./ThesisData/Lysozyme.dssp";
	}
	localProteinBuilder->LoadFile(testString);
	m_proteinModel = localProteinBuilder->GetCurrentProteinModel(GetWorld());

	//Create the neural network
	m_predictionNeuralNetwork = new NeuralNetwork(m_predictionWeightsFileLocation);
}

void AThesisTestGameMode::Tick(float DeltaSeconds)
{
	Interpolator::UpdateInterpolators(DeltaSeconds);
	Super::Tick(DeltaSeconds);
}

void AThesisTestGameMode::StartMatch()
{
	Super::StartMatch();

	//Get the spawn location of the protein
	AProteinModelSpawnPoint* bestModelSpawnPoint = GetBestProteinModelSpawnPoint(EProteinSpawnPointType::ESpawn_ProteinModel);

	//check if we received a valid protein model
	if (m_proteinModel 
		&& DefaultAminoAcidClass
		&& DefaultHydrogenBondClass
		&& bestModelSpawnPoint)
	{
		m_proteinModelCenterLocation = bestModelSpawnPoint->GetActorLocation();

		//set the render properties for the protein model
		m_proteinModel->UpdateRenderProperties(m_normalColor, m_helixColor, m_betaStrandColor, m_hydrogenBondColor,
			m_linkWidth, m_linkHeight, m_helixLinkWidth, m_betaStrandLinkWidth, m_hydrogenBondLinkWidth, m_aminoAcidSize,
			DefaultHydrogenBondClass);

		//se the enviromental properties of the model
		m_proteinModel->SetEnviromentalProperties(m_startingTemperatureCelsius, m_stableTemperatureCelsius,
			m_meltingTemperatureCelsius, m_irreversibleTemperatureCelsius, m_temperatureStep);

		UWorld* const World = GetWorld();
		m_proteinModel->SpawnAminoAcids(World, DefaultAminoAcidClass, m_proteinModelCenterLocation,
			m_distanceScale);
	}
}

AProteinModelSpawnPoint* AThesisTestGameMode::GetBestProteinModelSpawnPoint(EProteinSpawnPointType::Type spawnType)
{
	//at the moment we only expect to have one protein model spawn point
	if (ProteinModelSpawnPoints.Num() > 0)
	{
		for (int i = 0; i < ProteinModelSpawnPoints.Num(); ++i)
		{
			if (ProteinModelSpawnPoints[i]->m_typeOfSpawnPoint == spawnType)
			{
				return ProteinModelSpawnPoints[i];
			}
		}
		return nullptr;
	}

	return nullptr;
}

void AThesisTestGameMode::AddProteinModelSpawnPoint(AProteinModelSpawnPoint* NewProteinModelSpawnPoint)
{
	ProteinModelSpawnPoints.AddUnique(NewProteinModelSpawnPoint);
}

void AThesisTestGameMode::RemoveProteinModelSpawnPoint(AProteinModelSpawnPoint* RemovedProteinModelSpawnPoint)
{
	ProteinModelSpawnPoints.Remove(RemovedProteinModelSpawnPoint);
}

void AThesisTestGameMode::GetInputVals(const TArray<AAminoAcid*>& residues, int residueIndex, int slidingWindowWidth,
	TArray< TArray<double> >& out_values)
{
	out_values.Empty();

	TArray<double> unknownResidue;
	unknownResidue.Init(0.0, 21);
	unknownResidue[EResidueType::kUnknownResidue] = 1.0;

	for (int index = residueIndex - slidingWindowWidth; index <= residueIndex + slidingWindowWidth; ++index)
	{
		if (index < 0
			|| index >= residues.Num())
		{
			out_values.Add(unknownResidue);
		}
		else
		{
			out_values.Add(residues[index]->GetVectorRepresentationOfResidueType());
		}
	}
}

void AThesisTestGameMode::FilterSecondaryStructures(TArray<AAminoAcid*>& residues)
{
	if (residues.Num() > 0)
	{
		ESecondaryStructure::Type currentSecondaryStructure = residues[0]->GetSecondaryStructure();
		int continuousCount = 1;
		int resetIndex = 0;
		AAminoAcid* currentResidue = nullptr;

		for (int i = 1; i < residues.Num(); ++i)
		{
			currentResidue = residues[i];
			if (currentSecondaryStructure == currentResidue->GetSecondaryStructure())
			{
				++continuousCount;
			}
			else
			{
				switch (currentSecondaryStructure)
				{
				case ESecondaryStructure::ssAlphaHelix:
					if (continuousCount < 4)
					{
						resetIndex = i - continuousCount;
					}
					else
					{
						resetIndex = i;
					}
					break;

				case ESecondaryStructure::ssStrand:
					if (continuousCount < 2)
					{
						resetIndex = i - continuousCount;
					}
					else
					{
						resetIndex = i;
					}
					break;

				default:
					resetIndex = i;
					break;
				}

				//reset the residues
				for (; resetIndex < i; ++resetIndex)
				{
					residues[resetIndex]->ChangeSecondaryStructureType(ESecondaryStructure::ssLoop);
				}

				//change the current secondary type
				currentSecondaryStructure = residues[i]->GetSecondaryStructure();
				continuousCount = 1;
			}
		}
	}
}

/*
void AThesisTestGameMode::CheckForHelicalStructureSection(TArray<AAminoAcid*>& residues,
	int& startHelixIndex, int& endHelixIndex)
{
	int lastIndex = startHelixIndex + 6;
	int nonHelixResidueCountAtEnd = 0;

	//scan to see if this is a helix
	int validHelixTypeCount = 0;
	int currentResidueIndex = startHelixIndex;
	for (; currentResidueIndex < residues.Num() && currentResidueIndex < lastIndex; ++currentResidueIndex)
	{
		if (residues[currentResidueIndex]->GetAlphaHelixPropensity() > 100)
		{
			++validHelixTypeCount;
			nonHelixResidueCountAtEnd = 0;
		}
		else if (residues[currentResidueIndex]->IsHelixBreaker())
		{
			break;
		}
		else
		{
			++nonHelixResidueCountAtEnd;
		}
	}

	//check how many valid helix residues we have on the residue window
	if (validHelixTypeCount >= 4)
	{
		//expand until end conditions are met
		for (; currentResidueIndex < residues.Num() && nonHelixResidueCountAtEnd < 4;
			++currentResidueIndex)
		{
			if (residues[currentResidueIndex]->GetAlphaHelixPropensity() > 100)
			{
				nonHelixResidueCountAtEnd = 0;
			}
			else if (residues[currentResidueIndex]->IsHelixBreaker())
			{
				break;
			}
			else
			{
				++nonHelixResidueCountAtEnd;
			}
		}

		//when we break we know that we have reached a point where the last 4 contigious residues are
		//non-helix conformant or the end of the protein
		if (currentResidueIndex < residues.Num()
			&& !residues[currentResidueIndex]->IsHelixbreaker())
		{
			currentResidueIndex -= 3;
		}

		//check that the average is higher for alpha helix
		int totalAlphaHelixPropensity = 0;
		int totalBetaStrandPropensity = 0;
		for (int index = startHelixIndex; index < currentResidueIndex; ++index)
		{
			totalAlphaHelixPropensity += residues[index]->GetAlphaHelixPropensity();
			totalBetaStrandPropensity += residues[index]->GetBetaStrandPropensity();
		}

		check which one has the higher value
	}
	else
	{
		endHelixIndex = startHelixIndex;
	}
}

void AThesisTestGameMode::CheckForBetaStrandStructureSection(int& startBetaStrandIndex, int& endBetaStrandIndex)
{

}

void AThesisTestGameMode::ApplyChouFasmanAlgorithm(TArray<AAminoAcid*>& residues)
{
	for (int residueIndex = 0; residueIndex < residues.Num(); ++residueIndex)
	{
		if (residueIndex == startHelixIndex)
		{
			//check if there is a helical section that starts from this point
			CheckForHelicalStructureSection(startHelixIndex, endHelixIndex);
			startBetaStrandIndex = endHelixIndex + 1;
		}
		else if (residueIndex == startBetaStrandIndex)
		{
			//check if there is a beta sheet section that starts from this point
			CeckForBetaStrandStructureSection(startBetaStrandIndex, endBetaStrandIndex);
			startHelixIndex = endBetaStrandIndex + 1;
		}
	}
}
*/

GHProtein::ProteinModel* AThesisTestGameMode::PredictSecondaryStructure(TArray<AAminoAcid*>& residues)
{
	GHProtein::ProteinModel* customChainProteinModel = nullptr;
	if (m_predictionNeuralNetwork != nullptr)
	{
		TArray< TArray<double> > inputValsArray;
		TArray< TArray< double > > resultsHolder;
		TArray< double > currentResults;
		ESecondaryStructure::Type secondaryStructureType;

		//the reason we subtract 1 from the width is that the current residue is always in the middle
		int numberOfInputs = m_predictionNeuralNetwork->GetNumberOfRequiredInputs() - 1;
		int slidingWindowWidth = numberOfInputs * 0.5f;

		for (int residueIndex = 0; residueIndex < residues.Num(); ++residueIndex)
		{
			GetInputVals(residues, residueIndex, slidingWindowWidth, inputValsArray);
			m_predictionNeuralNetwork->FeedForward(inputValsArray);
			m_predictionNeuralNetwork->GetResults(currentResults);
			resultsHolder.Add(currentResults);

			secondaryStructureType = UThesisStaticLibrary::GetSecondaryStructureTypeFromVector(currentResults);
			residues[residueIndex]->ChangeSecondaryStructureType(secondaryStructureType);
		}

		FilterSecondaryStructures(residues);
		//ApplyChouFasmanAlgorithm(residues);

		customChainProteinModel = ProteinBuilder::GetInstance().CreateCustomChain(residues, m_aminoAcidSize, m_distanceScale,
			m_linkWidth, m_linkHeight, m_betaStrandLinkWidth, m_helixLinkWidth, m_hydrogenBondLinkWidth,
			m_normalColor, m_helixColor, m_betaStrandColor, m_hydrogenBondColor, DefaultHydrogenBondClass,
			GetWorld());
	}

	return customChainProteinModel;
}