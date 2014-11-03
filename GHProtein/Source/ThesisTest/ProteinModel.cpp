#include "ThesisTest.h"
#include "ThesisTestGameMode.h"
#include "AminoAcid.h"
#include "LinkFragment.h"
#include "ProteinUtilities.h"
#include "ProteinModel.h"
#include "ResidueContainer.h"
#include "HydrogenBond.h"
#include "SecondaryStructure.h"

namespace GHProtein
{
	ProteinModel::ProteinModel(UWorld* proteinWorld)
		: m_minBounds3D(FVector::ZeroVector)
		, m_maxBounds3D(FVector::ZeroVector)
		, m_centerOfBoundingBox(FVector::ZeroVector)
		, m_headPtr(nullptr)
		, m_headSecondaryStructure(nullptr)
		, m_tailSecondaryStructure(nullptr)
		, m_hydrogenBondLinkWidth(0.f)
		, m_linkHeight(0.f)
		, m_linkWidth(0.f)
		, m_helixLinkWidth(0.f)
		, m_betaStrandLinkWidth(0.f)
		, m_aminoAcidSize(0.f)
		, m_normalColor(FColor::White)
		, m_helixColor(FColor::White)
		, m_betaStrandColor(FColor::White)
		, m_hydrogenBondColor(FColor::White)
		, m_world(proteinWorld)
		, m_temperatureStep(1.f)
		, m_stableTemperatureCelsius(0.f)
		, m_meltingTemperatureCelsius(0.f)
		, m_irreversibleTemperatureCelsius(0.f)
		, m_temperatureCelsius(0.f)						//we start the model with a temperature equal to a human's average body temperatures
		, m_hydrogenBondClass(nullptr)
	{}

	ProteinModel::~ProteinModel()
	{
		ResidueIDMap.Empty();

		//delete all the residue containers
		for (int i = 0; i < m_residueContainers.Num(); ++i)
		{
			delete m_residueContainers[i];
		}

		//delete all the residues
		for (int i = 0; i < m_residueVector.Num(); ++i)
		{
			delete m_residueVector[i];
		}

		//ignore the arrays of m_modifiedHydrogenBonds and m_modifiedSecondaryStructures
		//they just contain references to secondaryStructures and hydrogenBonds we already have
		//stored in a separate array
		m_modifiedSecondaryStructures.Empty();
		m_modifiedHydrogenBonds.Empty();

		//ignore the beta strands array, we are going to delete it when iterating through the linked list
		//of secondary structures
		m_betaStrands.Empty();

		//ignore the map of betaSheets to secondaryStructures
		m_strandToBetaSheetMap.Empty();

		//delete all the beta sheets
		for (int i = 0; i < m_betaSheets.Num(); ++i)
		{
			delete m_betaSheets[i];
		}

		//delete all the secondary structures
		SecondaryStructure* currentSecondaryStructure = m_headSecondaryStructure;
		SecondaryStructure* nextSecondaryStructure = nullptr;

		//deleting secondary structures also deletes the aminoacids
		//NOTE: amino acids are not destroyed if the model is a customChainModel
		bool keepDeleting = currentSecondaryStructure != nullptr;
		while (keepDeleting)
		{
			nextSecondaryStructure = currentSecondaryStructure->GetNextStructurePtr();
			keepDeleting = currentSecondaryStructure != m_tailSecondaryStructure;

			delete currentSecondaryStructure;
			currentSecondaryStructure = nextSecondaryStructure;
		}

		m_headSecondaryStructure = nullptr;
		m_tailSecondaryStructure = nullptr;
	}

	bool ProteinModel::AddResidue(Residue* residueAttemptingToInsert)
	{
		//this brings an interesting problem
		//I want to create something i have fast access to, but I also want to create something I
		//could traverse really quickly in order

		//I know I am going to need a map
		ResidueContainer** residues = ResidueIDMap.Find(residueAttemptingToInsert->GetNumber());

		if (residues && *residues)
		{
			//we should probably check to see if we have not yet inserted this element
			if (!(*residues)->AddResidue(residueAttemptingToInsert))
			{
				return false;
			}
		}
		else
		{
			ResidueContainer* newContainer = new ResidueContainer();
			newContainer->AddResidue(residueAttemptingToInsert);
			m_residueContainers.Add(newContainer);
			ResidueIDMap.Add(residueAttemptingToInsert->GetNumber(),
				newContainer);
		}

		m_residueVector.Add(residueAttemptingToInsert);
		return true;
	}

	Residue* ProteinModel::GetResidueWithSpecifiedID(int residueNumber, Residue* partnerResidue)
	{
		ResidueContainer** residues = ResidueIDMap.Find(residueNumber);
		Residue* foundResidue = nullptr;

		if (residues && *residues)
		{
			//we have found the appropriate residue container
			//if we have a partner residue specified, then we return the residue whose partner is the specified residue
			if (partnerResidue)
			{
				foundResidue = (*residues)->FindBetaPartnerToResidue(partnerResidue);
			}
			else
			{
				foundResidue = (*residues)->GetFirstResidue();
			}
		}

		return foundResidue;
	}

	void ProteinModel::BuildProteinModel()
	{
		int numberOfResidues = m_residueVector.Num();
		Residue* currentResidue = nullptr;
		Residue* betaPartnerResidue = nullptr;
		Residue* prevResidue = nullptr;
		BridgePartner currentBridgePartner;
		char currentBridgeLabel = ' ';

		//set the beta partners along with the previous and next residues
		for (int residueIndex = 0; residueIndex < numberOfResidues; ++residueIndex)
		{
			//grab the current residue
			currentResidue = m_residueVector[residueIndex];

			//Get the beta partners
			for (int betaPartnerID = 0; betaPartnerID < 2; ++betaPartnerID)
			{
				currentBridgePartner = currentResidue->GetBetaPartner(betaPartnerID);

				if (currentBridgePartner.ladder != ' ')
				{
					//at this point we know that we have a beta partner
					//now we need to get the right reference to it
					betaPartnerResidue = GetResidueWithSpecifiedID(currentBridgePartner.number, currentResidue);
					
					//set the beta partner residue
					currentResidue->SetBetaPartnerResidue(betaPartnerID, betaPartnerResidue);
				}
			}

			//set the next and previous residue
			currentResidue->SetPrevious(prevResidue);
			if (prevResidue != nullptr)
			{
				prevResidue->SetNext(currentResidue);
			}
			prevResidue = currentResidue;
		}

		//current residue now holds the last residue
		currentResidue->SetNext(nullptr);

		return;
	}

	void ProteinModel::UpdateMinAndMaxBounds(const FVector& newPoint)
	{
		m_minBounds3D.X = GetMin(m_minBounds3D.X, newPoint.X);
		m_minBounds3D.Y = GetMin(m_minBounds3D.Y, newPoint.Y);
		m_minBounds3D.Z = GetMin(m_minBounds3D.Z, newPoint.Z);

		m_maxBounds3D.X = GetMax(m_maxBounds3D.X, newPoint.X);
		m_maxBounds3D.Y = GetMax(m_maxBounds3D.Y, newPoint.Y);
		m_maxBounds3D.Z = GetMax(m_maxBounds3D.Z, newPoint.Z);

		//get the center of the bounds
		m_centerOfBoundingBox = (m_minBounds3D * .5f) + (m_maxBounds3D * .5f);
	}

	void ProteinModel::MoveCenterOfModelToSpecifiedLocation(const FVector& proteinModelCenterLocation)
	{
		//translate all amino acids
		AAminoAcid* currentAminoAcid = m_headPtr;
		FVector newResidueLocation = FVector::ZeroVector;
		while (currentAminoAcid)
		{
			currentAminoAcid->Translate(proteinModelCenterLocation - m_centerOfBoundingBox);
			currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
		}

		//offset the bounding box
		m_minBounds3D = m_minBounds3D - m_centerOfBoundingBox + proteinModelCenterLocation;
		m_maxBounds3D = m_maxBounds3D - m_centerOfBoundingBox + proteinModelCenterLocation;
		m_centerOfBoundingBox = proteinModelCenterLocation;
	}

	void ProteinModel::UpdateRenderProperties(const FColor& normalColor, const FColor& helixColor, const FColor& betaStrandColor,
		const FColor& hydrogenBondColor, float normalLinkWidth, float normalLinkHeight, float helixLinkWidth, float betaStrandLinkWidth,
		float hydrogenBondLinkWidth, float aminoAcidSize, UClass* hydrogenBondClass)
	{
		m_normalColor = normalColor;
		m_helixColor = helixColor;
		m_betaStrandColor = betaStrandColor;
		m_hydrogenBondColor = hydrogenBondColor;

		m_linkWidth = normalLinkWidth;
		m_helixLinkWidth = helixLinkWidth;
		m_betaStrandLinkWidth = betaStrandLinkWidth;
		m_hydrogenBondLinkWidth = hydrogenBondLinkWidth;

		m_linkHeight = normalLinkHeight;

		m_aminoAcidSize = aminoAcidSize;

		m_hydrogenBondClass = hydrogenBondClass;
	}

	void ProteinModel::SetEnviromentalProperties(float startingTemperatureCelsius, float stableTemperatureCelsius,
		float meltingTemperatureCelsius, float irreversibleTemperatureCelsius, float temperatureStep)
	{
		m_temperatureCelsius = startingTemperatureCelsius;
		m_stableTemperatureCelsius = stableTemperatureCelsius;
		m_meltingTemperatureCelsius = meltingTemperatureCelsius;
		m_irreversibleTemperatureCelsius = irreversibleTemperatureCelsius;
		m_temperatureStep = temperatureStep;
	}

	//space out the residue
	void ProteinModel::SpaceOutResidue(AAminoAcid* previousAminoAcid, AAminoAcid* currentAminoAcid,
		SecondaryStructure* currentSecondaryStructure, float distanceScale, int index)
	{
		if (previousAminoAcid)
		{
			//get the distance between the two residues
			FVector previousResidueLocation = previousAminoAcid->GetActorLocation();
			FVector currentResidueLocation = currentAminoAcid->GetActorLocation();
			
			//we only care about the distance along the x axis when constructing a custom chain
			previousResidueLocation.Y = currentResidueLocation.Y;
			previousResidueLocation.Z = currentResidueLocation.Z;

			//get the horizontal distance between the previous and current amino acid
			FVector distanceVector = currentResidueLocation - previousResidueLocation;
			distanceVector.Normalize();

			//get the current number of residues in the secondary structure
			int numberOfResiduesInSecondaryStructure = currentSecondaryStructure->GetAminoAcidCount();
			
			//for the moment we assume that it is a residue in a random coil
			//therefore we are going to offset the location of the residue as if it was part of a random coil
			FVector offset = distanceVector * 3.8;

			//check what type of secondary structure it is
			switch (currentSecondaryStructure->GetSecondaryStructureType())
			{
			case ESecondaryStructure::ssAlphaHelix:
				if (numberOfResiduesInSecondaryStructure != 0)
				{
					//this is not the first residue in the helix structure
					//therefore we have to start rotating it as if it was a helix

					//we know that each new rediue in the helix adds 1.5A along the helix's axis
					offset = distanceVector * 1.5;

					//we also know that there are 3.6 residues per rotation in the helix
					float radians = ((2.0 * PI) / 3.6) * numberOfResiduesInSecondaryStructure;
					offset.Z += cosf(radians) * 3.8;
					offset.X += -sinf(radians) * 3.8;
				}
				break;
			case ESecondaryStructure::ssStrand:
				if (numberOfResiduesInSecondaryStructure != 0)
				{
					//we space the residue as a strand
					//the average space between strands in a residue is 3.5
					offset = distanceVector * 3.5;
				}
				break;
			default:
				break;
			}

			offset *= distanceScale;
			currentResidueLocation += offset;

			//assign the new position to the residue
			currentAminoAcid->SetActorLocation(currentResidueLocation);
		}
	}

	void ProteinModel::BuildCustomChain(float distanceScale, TArray<AAminoAcid*>& residues)
	{
		if (residues.Num() != 0)
		{
			AAminoAcid* previousAminoAcid = nullptr;
			AAminoAcid* currentAminoAcid = nullptr;
			ESecondaryStructure::Type currentSecondaryStructureType = ESecondaryStructure::ssCount;
			SecondaryStructure* currentSecondaryStructure = nullptr;
			FVector aminoAcidLocation = FVector::ZeroVector;

			//iterate over all of the residues and space them according to their secondary structure
			for (int residueIndex = 0; residueIndex < residues.Num(); ++residueIndex)
			{
				//get the type of secondary structure
				currentAminoAcid = residues[residueIndex];

				currentAminoAcid->SetAminoAcidSize(m_aminoAcidSize);
				currentAminoAcid->SetParentModel(this);

				//link the previous amino acid to this
				if (previousAminoAcid)
				{
					previousAminoAcid->SetNextAminoAcid(currentAminoAcid);
				}
				previousAminoAcid = currentAminoAcid;


				//update bounding box
				if (m_headPtr)
				{
					UpdateMinAndMaxBounds(aminoAcidLocation);
				}
				else
				{
					m_headPtr = currentAminoAcid;
					m_minBounds3D.Set(aminoAcidLocation.X, aminoAcidLocation.Y, aminoAcidLocation.Z);
					m_maxBounds3D.Set(aminoAcidLocation.X, aminoAcidLocation.Y, aminoAcidLocation.Z);
				}

				if (currentSecondaryStructureType != currentAminoAcid->GetSecondaryStructure())
				{
					if (currentSecondaryStructure)
					{
						AppendSecondaryStructure(currentSecondaryStructure, false);
					}

					currentSecondaryStructureType = currentAminoAcid->GetSecondaryStructure();
					currentSecondaryStructure = new SecondaryStructure(currentSecondaryStructureType, this);
				}
				//space out the residue
				SpaceOutResidue(previousAminoAcid, currentAminoAcid, currentSecondaryStructure, distanceScale, residueIndex);
				currentSecondaryStructure->AppendAminoAcid(currentAminoAcid, true);
			}
			AppendSecondaryStructure(currentSecondaryStructure, false);


			//we want to bring everything to the center, so subtract the middle of the bounding box from all locations
			//MoveCenterOfModelToSpecifiedLocation(proteinModelCenterLocation);

			//iterate over the chain of amino acids and spawn the link fragments
			currentAminoAcid = m_headPtr;
			while (currentAminoAcid)
			{
				currentAminoAcid->SetRenderProperties(m_normalColor, m_helixColor, m_betaStrandColor, m_hydrogenBondColor,
					m_linkWidth, m_helixLinkWidth, m_betaStrandLinkWidth, m_hydrogenBondLinkWidth, m_linkHeight);

				currentAminoAcid->SpawnLinkParticleToNextAminoAcid(true);
				currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
			}

			//spawn the hydrogen bonds in the beta sheet
			for (int betaSheetIndex = 0; betaSheetIndex < m_betaSheets.Num(); ++betaSheetIndex)
			{
				m_betaSheets[betaSheetIndex]->SpawnHydrogenBonds();
			}

			for (SecondaryStructure* currentStructure = m_headSecondaryStructure;
				currentStructure; currentStructure = currentStructure->GetNextStructurePtr())
			{
				currentStructure->SpawnHydrogenBonds();
			}
		}
	}

	void ProteinModel::SpawnAminoAcids(UWorld* world, UClass* blueprint, const FVector& proteinModelCenterLocation,
		float distanceScale)
	{
		if (!world || !blueprint || m_aminoAcidSize <= 0.f)
		{
			//we need to have a valid world and blueprint
			//we can only spawn amino acids with a positive non-zero size
			return;
		}
		else
		{
			FRotator originRotation = FRotator::ZeroRotator;
			AAminoAcid* previousAminoAcid = nullptr;
			AAminoAcid* currentAminoAcid = nullptr;
			Residue* currentResidue = nullptr;
			FVector aminoAcidLocation = FVector::ZeroVector;
			ESecondaryStructure::Type currentSecondaryStructureType = ESecondaryStructure::ssCount;
			SecondaryStructure* currentSecondaryStructure = nullptr;

			//iterate over all of the residues and spawn an amino acid actor for each one of them
			//also creates the secondary structures
			for (int residueIndex = 0; residueIndex < m_residueVector.Num(); ++residueIndex)
			{
				currentResidue = m_residueVector[residueIndex];
				currentResidue->GetCALocation(aminoAcidLocation);
				aminoAcidLocation *= distanceScale; // this is done in order to space out the proteins

				currentAminoAcid = UThesisStaticLibrary::SpawnBP<AAminoAcid>(world, blueprint, aminoAcidLocation, originRotation);
				currentAminoAcid->SetAminoAcidSize(m_aminoAcidSize);
				currentAminoAcid->SetResidueInformation(currentResidue);
				currentAminoAcid->SetParentModel(this);

				//link the previous amino acid to this
				if (previousAminoAcid)
				{
					previousAminoAcid->SetNextAminoAcid(currentAminoAcid);
				}
				previousAminoAcid = currentAminoAcid;


				//update bounding box
				if (m_headPtr)
				{
					UpdateMinAndMaxBounds(aminoAcidLocation);
				}
				else
				{
					m_headPtr = currentAminoAcid;
					m_minBounds3D.Set(aminoAcidLocation.X, aminoAcidLocation.Y, aminoAcidLocation.Z);
					m_maxBounds3D.Set(aminoAcidLocation.X, aminoAcidLocation.Y, aminoAcidLocation.Z);
				}

				//set the residue's secondary structure and updates the current secondary structure
				currentAminoAcid->ChangeSecondaryStructureType(currentResidue->GetSecondaryStructure());

				if (currentSecondaryStructureType != currentAminoAcid->GetSecondaryStructure())
				{
					if (currentSecondaryStructure)
					{
						AppendSecondaryStructure(currentSecondaryStructure);
					}

					currentSecondaryStructureType = currentAminoAcid->GetSecondaryStructure();
					currentSecondaryStructure = new SecondaryStructure(currentSecondaryStructureType, this);
				}
				currentSecondaryStructure->AppendAminoAcid(currentAminoAcid);

			}
			AppendSecondaryStructure(currentSecondaryStructure);


			//we want to bring everything to the center, so subtract the middle of the bounding box from all locations
			MoveCenterOfModelToSpecifiedLocation(proteinModelCenterLocation);

			//iterate over the chain of amino acids and spawn the link fragments
			currentAminoAcid = m_headPtr;
			while (currentAminoAcid)
			{
				currentAminoAcid->SetRenderProperties(m_normalColor, m_helixColor, m_betaStrandColor, m_hydrogenBondColor,
					m_linkWidth, m_helixLinkWidth, m_betaStrandLinkWidth, m_hydrogenBondLinkWidth, m_linkHeight);

				currentAminoAcid->SpawnLinkParticleToNextAminoAcid();
				currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
			}

			//spawn the hydrogen bonds in the beta sheet
			for (int betaSheetIndex = 0; betaSheetIndex < m_betaSheets.Num(); ++betaSheetIndex)
			{
				m_betaSheets[betaSheetIndex]->SpawnHydrogenBonds();
			}

			for (SecondaryStructure* currentStructure = m_headSecondaryStructure;
				currentStructure; currentStructure = currentStructure->GetNextStructurePtr())
			{
				currentStructure->SpawnHydrogenBonds();
			}
		}
	}

	AHydrogenBond* ProteinModel::SpawnHydrogenBond(AAminoAcid* startResidue, AAminoAcid* endResidue)
	{
		//Get the size of the hydrogen bond link fragment from the game mode
		FVector startLocation = startResidue->GetActorLocation();
		FVector endLocation = endResidue->GetActorLocation();
		UWorld* world = startResidue->GetWorld();

		AHydrogenBond* hydrogenBond = UThesisStaticLibrary::SpawnBP<AHydrogenBond>(world, m_hydrogenBondClass,
			FVector::ZeroVector, FRotator::ZeroRotator);

		if (hydrogenBond)
		{
			hydrogenBond->SplineMeshComponent->SetStartPosition(startLocation);
			hydrogenBond->SplineMeshComponent->SetEndPosition(endLocation);

			//set render properties for the bond
			hydrogenBond->UpdateRenderProperties(m_hydrogenBondColor, m_hydrogenBondLinkWidth, m_linkHeight);
			//set the parent model
			hydrogenBond->SetParentModelAndResiduesInformation(this, startResidue, endResidue);
			//set the enviromental properties
			hydrogenBond->SetEnviromentalProperties(m_temperatureCelsius, m_stableTemperatureCelsius, m_meltingTemperatureCelsius,
				m_irreversibleTemperatureCelsius);

			//should probably add the hydrogen bond into the array of current hydrogen bonds
			m_hydrogenBonds.Add(hydrogenBond);
		}
		return hydrogenBond;
	};

	AAminoAcid* ProteinModel::GetAminoAcidWithSpecifiedId(int sequenceNumber)
	{
		//search through all of the secondary structures and grab the residue with the specified sequence number
		SecondaryStructure* currentSecondaryStructure = m_headSecondaryStructure;
		AAminoAcid* foundResidue = nullptr;

		while (currentSecondaryStructure)
		{
			foundResidue = currentSecondaryStructure->GetAminoAcidWithSpecifiedId(sequenceNumber);

			if (foundResidue)
			{
				break;
			}
			else
			{
				currentSecondaryStructure = currentSecondaryStructure->GetNextStructurePtr();
			}
		}

		return foundResidue;
	}

	void ProteinModel::AppendSecondaryStructure(SecondaryStructure* secondaryStructure, bool buildBetaSheet)
	{
		if (!secondaryStructure)
		{
			return;
		}

		if (m_tailSecondaryStructure)
		{
			m_tailSecondaryStructure->SetNextStructurePtr(secondaryStructure);
			m_tailSecondaryStructure = secondaryStructure;
		}
		else
		{
			m_headSecondaryStructure = secondaryStructure;
			m_tailSecondaryStructure = m_headSecondaryStructure;
		}

		//check if it is a beta strand
		if (secondaryStructure->GetSecondaryStructureType() == ESecondaryStructure::ssStrand && buildBetaSheet)
		{
			AddBetaStrand(secondaryStructure);
		}

		secondaryStructure->SetEnviromentalProperties(m_temperatureCelsius, m_stableTemperatureCelsius,
			m_meltingTemperatureCelsius, m_irreversibleTemperatureCelsius);
	}

	void ProteinModel::AddBetaStrand(SecondaryStructure* newStrand)
	{
		TArray<uint32> bridgeLabels;
		newStrand->GetBridgeLabels(bridgeLabels);

		BetaSheet* currentBetaSheet = nullptr;
		//look for other beta strands that share the same beta bridge label
		for (int betaStrandIndex = 0; betaStrandIndex < m_betaStrands.Num(); ++betaStrandIndex)
		{
			//check if the strands are connected
			if (m_betaStrands[betaStrandIndex]->IsPartOfSpecifiedBridgeLabels(bridgeLabels))
			{
				if (currentBetaSheet)
				{
					currentBetaSheet = MergeStrandIntoBetaSheet(m_betaStrands[betaStrandIndex], currentBetaSheet);
				}
				else
				{
					//if it is part of the same bridge label, then add it to the current beta strand collection for the beta sheet
					currentBetaSheet = MergeStrands(newStrand, m_betaStrands[betaStrandIndex]);
				}
			}
		}

		if (m_betaStrands.Find(newStrand) == INDEX_NONE)
		{
			m_betaStrands.Add(newStrand);
		}
	}

	BetaSheet* ProteinModel::MergeStrandIntoBetaSheet(SecondaryStructure* newStrand, BetaSheet* betaSheet)
	{
		//check if the strand is part of a beta sheet already
		BetaSheet* out_betaSheet = nullptr;
		BetaSheet** foundBetaSheet = m_strandToBetaSheetMap.Find(newStrand);
		if (foundBetaSheet)
		{
			if (*foundBetaSheet == betaSheet)
			{
				//the strand already belongs to the specified sheet
				out_betaSheet = betaSheet;
			}
			else
			{
				//the residue is already part of a beta sheet
				//we need to merge the sheets into one
				out_betaSheet = MergeBetaSheets(*foundBetaSheet, betaSheet);
			}
		}
		else
		{
			out_betaSheet = betaSheet;
			betaSheet->m_strands.Add(newStrand);
			m_strandToBetaSheetMap.Add(newStrand, betaSheet);
		}

		return out_betaSheet;
	}

	BetaSheet* ProteinModel::MergeStrands(SecondaryStructure* strand1, SecondaryStructure* strand2)
	{
		//check if any of the two strands are part of a beta sheet already
		BetaSheet** foundBetaSheet1 = m_strandToBetaSheetMap.Find(strand1);
		BetaSheet** foundBetaSheet2 = m_strandToBetaSheetMap.Find(strand2);
		BetaSheet* out_betaSheet = nullptr;

		if (foundBetaSheet1 && foundBetaSheet2)
		{
			out_betaSheet = MergeBetaSheets(*foundBetaSheet1, *foundBetaSheet2);
		}
		else if (foundBetaSheet1)
		{
			out_betaSheet = *foundBetaSheet1;
			(*foundBetaSheet1)->m_strands.Add(strand2);
			m_strandToBetaSheetMap.Add(strand2, *foundBetaSheet1);
		}
		else if (foundBetaSheet2)
		{
			out_betaSheet = *foundBetaSheet2;
			(*foundBetaSheet2)->m_strands.Add(strand1);
			m_strandToBetaSheetMap.Add(strand1, *foundBetaSheet2);
		}
		else
		{
			//make a new beta sheet
			BetaSheet* newBetaSheet = new BetaSheet(strand1, strand2, this);
			m_strandToBetaSheetMap.Add(strand1, newBetaSheet);
			m_strandToBetaSheetMap.Add(strand2, newBetaSheet);

			m_betaSheets.Add(newBetaSheet);

			out_betaSheet = newBetaSheet;
		}

		return out_betaSheet;
	}

	BetaSheet* ProteinModel::MergeBetaSheets(BetaSheet* sheet1, BetaSheet* sheet2)
	{
		//save the strands that were originally part of sheet1 so that their pointer is set the same
		//as the other strands in the sheet
		TArray<SecondaryStructure*> sheet1Strands = sheet1->m_strands;
		BetaSheet* mergingSheet = sheet1;
		BetaSheet* out_betaSheet = nullptr;

		//iterate over the strands of sheet2 and modify the mapping of all the strands that compose it
		for (int i = 0; i < sheet2->m_strands.Num(); ++i)
		{
			//we only add to the strands if it is not already part of the sheet1
			if (mergingSheet->m_strands.Find(sheet2->m_strands[i]) == INDEX_NONE)
			{
				mergingSheet->m_strands.Add(sheet2->m_strands[i]);
			}

			//replace the mapping of the strand to the new sheet
			m_strandToBetaSheetMap[sheet2->m_strands[i]] = mergingSheet;
		}

		//change the mapping of sheet1 original strands to the mergingSheet
		for (int i = 0; i < sheet1Strands.Num(); ++i)
		{
			m_strandToBetaSheetMap[sheet1Strands[i]] = mergingSheet;
		}

		//remove sheet2 from the array of sheets
		m_betaSheets.Remove(sheet2);

		//delete sheet2
		delete sheet2;
		sheet2 = mergingSheet;

		//return the new sheet
		out_betaSheet = mergingSheet;
		return out_betaSheet;
	}

	void ProteinModel::RotateModel(const FVector& anglesDegrees)
	{
		FVector distanceFromCenter = FVector::ZeroVector;

		//iterate ove the chain of amino acids and rotate them from the model's center point
		AAminoAcid* currentAminoAcid = m_headPtr;
		FRotator rotation(anglesDegrees.X, anglesDegrees.Y, anglesDegrees.Z);
		FRotationMatrix rotationMatrix(rotation);

		FVector aminoAcidLocation = FVector::ZeroVector;
		FVector rotationPoint = m_centerOfBoundingBox;

		//update position of the amino acids
		while (currentAminoAcid)
		{
			currentAminoAcid->RotateAminoAcidFromSpecifiedPoint(rotationMatrix, rotationPoint);
			aminoAcidLocation = currentAminoAcid->GetActorLocation();

			if (currentAminoAcid != m_headPtr)
			{
				UpdateMinAndMaxBounds(aminoAcidLocation);
			}
			else
			{
				m_minBounds3D.Set(aminoAcidLocation.X, aminoAcidLocation.Y, aminoAcidLocation.Z);
				m_maxBounds3D.Set(aminoAcidLocation.X, aminoAcidLocation.Y, aminoAcidLocation.Z);
			}

			currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
		}

		//update hydrogen bonds
		for (int i = 0; i < m_hydrogenBonds.Num(); ++i)
		{
			m_hydrogenBonds[i]->RotateAboutSpecifiedPoint(rotationMatrix, rotationPoint);
		}
	}

	FVector ProteinModel::GetDirectionFromCenter(const FVector& currentLocation)
	{
		FVector returnVector = currentLocation;
		returnVector -= m_centerOfBoundingBox;
		return returnVector;
	}

	void ProteinModel::TranslateModel(const FVector& displacement)
	{
		FVector distanceFromCenter = FVector::ZeroVector;

		//iterate ove the chain of amino acids and rotate them from the model's center point
		AAminoAcid* currentAminoAcid = m_headPtr;

		//update position of the amino acids
		while (currentAminoAcid)
		{
			currentAminoAcid->Translate(displacement);
			currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
		}

		//translate the bounding box min and max
		m_minBounds3D += displacement;
		m_maxBounds3D += displacement;
		m_centerOfBoundingBox += displacement;

		//displace the hydrogen bonds
		for (int i = 0; i < m_hydrogenBonds.Num(); ++i)
		{
			//m_hydrogenBonds[i]->Translate(displacement);
		}
	}

	FVector ProteinModel::GetBoundingBoxDimensions() const
	{
		return m_maxBounds3D - m_minBounds3D;
	}

	FVector ProteinModel::GetCenterLocation() const
	{
		return m_centerOfBoundingBox;
	}

	void ProteinModel::ToggleShake()
	{
		for (int i = 0; i < m_hydrogenBonds.Num(); ++i)
		{
			m_hydrogenBonds[i]->ToggleShake();
		}
	}

	void ProteinModel::ToggleBreaking()
	{
		for (int i = 0; i < m_hydrogenBonds.Num(); ++i)
		{
			m_hydrogenBonds[i]->Break();
		}
	}

	void ProteinModel::HideHydrogenBonds()
	{
		for (int i = 0; i < m_hydrogenBonds.Num(); ++i)
		{
			m_hydrogenBonds[i]->Hide();
		}
	}

	void ProteinModel::ModifyTemperature(float temperatureModifierScale)
	{
		m_temperatureCelsius += (m_temperatureStep * temperatureModifierScale);

		UpdateModelTemperature();
	}

	void ProteinModel::SetTemperature(float temperatureCelsius)
	{
		m_temperatureCelsius = temperatureCelsius;

		UpdateModelTemperature();
	}

	void ProteinModel::UpdateModelTemperature()
	{
		SecondaryStructure* currentStructure = m_headSecondaryStructure;

		bool changeInTemperatureTriggeredAnimation = false;

		while (currentStructure != nullptr)
		{
			changeInTemperatureTriggeredAnimation = currentStructure->SetTemperature(m_temperatureCelsius);
			currentStructure = currentStructure->GetNextStructurePtr();
		}

		//set the temperature on all of the hydrogen bonds
		for (int i = 0; i < m_hydrogenBonds.Num(); ++i)
		{
			changeInTemperatureTriggeredAnimation = m_hydrogenBonds[i]->SetTemperature(m_temperatureCelsius);
		}

		if (changeInTemperatureTriggeredAnimation)
		{
			//trigger the event that indicates the model is being animated
			AThesisTestGameMode* gameMode = (AThesisTestGameMode*)(m_world->GetAuthGameMode());
			if (gameMode)
			{
				gameMode->StartedProteinAnimation();
			}
		}
	}

	float ProteinModel::GetCurrentTemperature()
	{
		return m_temperatureCelsius;
	}

	UWorld* ProteinModel::GetWorld()
	{
		return m_world;
	}

	void ProteinModel::AddToListOfModifiedSecondaryStructures(SecondaryStructure* secondaryStructureBeingModified)
	{
		m_modifiedSecondaryStructures.AddUnique(secondaryStructureBeingModified);
	}

	void ProteinModel::RemoveFromListOfModifiedSecondaryStructures(SecondaryStructure* secondaryStructureToRemove)
	{
		m_modifiedSecondaryStructures.Remove(secondaryStructureToRemove);

		//once removed, lets check if there are any remaining structures being modified
		CheckIfEndModificationEventShouldTrigger();
	}

	void ProteinModel::AddToListOfModifiedHydrogenBonds(AHydrogenBond* hydrogenBondBeingModified)
	{
		m_modifiedHydrogenBonds.AddUnique(hydrogenBondBeingModified);
	}

	void ProteinModel::RemoveFromListOfModifiedHydrogenBonds(AHydrogenBond* hydrogenBondToRemove)
	{
		m_modifiedHydrogenBonds.Remove(hydrogenBondToRemove);
		CheckIfEndModificationEventShouldTrigger();
	}

	void ProteinModel::DestroySecondaryStructure(SecondaryStructure* secondaryStructureToDestroy)
	{
		//first we remove it from the modified secondary structures array
		m_modifiedSecondaryStructures.Remove(secondaryStructureToDestroy);

		//now we check if this is the head or tail of the secondary structures linked list
		if (m_headSecondaryStructure == m_tailSecondaryStructure && m_headSecondaryStructure == secondaryStructureToDestroy)
		{
			//this is the last secondary structure
			
		}
		else if (m_headSecondaryStructure == secondaryStructureToDestroy)
		{
			m_headSecondaryStructure = secondaryStructureToDestroy->GetNextStructurePtr();
		}
		else if (m_tailSecondaryStructure == secondaryStructureToDestroy)
		{
			m_tailSecondaryStructure = secondaryStructureToDestroy->Previous
		}
	}

	void ProteinModel::DestroyHydrogenBond(AHydrogenBond* hydrogenBondToDestroy)
	{
		//remove all references to the hydrogen bond
		m_modifiedHydrogenBonds.Remove(hydrogenBondToDestroy);
		m_hydrogenBonds.Remove(hydrogenBondToDestroy);

		hydrogenBondToDestroy->Destroy();
	}

	void ProteinModel::DestroyAminoAcids(AAminoAcid* startAminoAcid, AAminoAcid* endAminoAcid, 
		bool automaticallyDestroySecondaryStructures)
	{
		if (m_isCustomChainModel)
		{
			//we do not delete residues of custom chain models

			//we do however, destroy the hydrogen bonds and link fragments that are in it
			AAminoAcid* nextAminoAcid = startAminoAcid;
			AAminoAcid* aminoAcidToDestroy = nullptr;
			bool keepDestroying = nextAminoAcid != nullptr;
			while (keepDestroying)
			{
				keepDestroying = nextAminoAcid != endAminoAcid;
				aminoAcidToDestroy = nextAminoAcid;
				nextAminoAcid = nextAminoAcid->GetNextAminoAcidPtr();
				aminoAcidToDestroy->DestroyLinkFragmentAndHydrogenBonds();
			}
			return;
		}

		//check if the start amino acid is the head ptr
		if (m_headPtr && m_headPtr == startAminoAcid)
		{
			if (endAminoAcid)
			{
				m_headPtr = endAminoAcid->GetNextAminoAcidPtr();
			}
			else
			{
				m_headPtr = nullptr;
			}
		}

		//iterate from start amino acid to end amino acid and delete the actor
		AAminoAcid* nextAminoAcid = startAminoAcid;
		AAminoAcid* aminoAcidToDestroy = nullptr;
		bool keepDestroying = nextAminoAcid != nullptr;
		while (keepDestroying)
		{
			keepDestroying = nextAminoAcid != endAminoAcid;
			aminoAcidToDestroy = nextAminoAcid;
			nextAminoAcid = nextAminoAcid->GetNextAminoAcidPtr();

			if (!automaticallyDestroySecondaryStructures)
			{
				aminoAcidToDestroy->SetSecondaryStructure(nullptr);
			}
			aminoAcidToDestroy->Destroy();
		}
	}

	void ProteinModel::CheckIfEndModificationEventShouldTrigger()
	{
		if (m_modifiedSecondaryStructures.Num() == 0
			&& m_modifiedHydrogenBonds.Num() == 0)
		{
			//triggerEvent
			AThesisTestGameMode* gameMode = (AThesisTestGameMode*)(m_world->GetAuthGameMode());
			if (gameMode)
			{
				gameMode->FinishedProteinAnimation();
			}
		}
	}
}