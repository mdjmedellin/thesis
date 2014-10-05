#include "ThesisTest.h"
#include "AminoAcid.h"
#include "LinkFragment.h"
#include "ProteinUtilities.h"
#include "ProteinModel.h"
#include "ResidueContainer.h"
#include "SecondaryStructure.h"

namespace GHProtein
{
	ProteinModel::ProteinModel(UWorld* proteinWorld)
		: m_minBounds3D(FVector(0.f, 0.f, 0.f))
		, m_maxBounds3D(FVector(0.f, 0.f, 0.f))
		, m_headPtr(nullptr)
		, m_headSecondaryStructure(nullptr)
		, m_tailSecondaryStructure(nullptr)
		, m_hydrogenBondLinkWidth(0.f)
		, m_linkHeight(0.f)
		, m_linkWidth(0.f)
		, m_world(proteinWorld)
		, m_temperatureCelsius(37.f)						//we start the model with a temperature equal to a human's average body temperatures
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
	}

	bool ProteinModel::AddResidue(Residue* residueAttemptingToInsert)
	{
		//this brings an interesting problem
		//I want to create something i have fast access to, but I also want to create something I
		//could traverse really quickly in order

		//I know I am going to need a map
		//TArray< Residue* >* residueArray = ResidueIDMap.Find(insertedResidue->GetNumber());
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
			newResidueLocation = currentAminoAcid->GetActorLocation() - m_centerOfBoundingBox;		//this brings the residue centered at 0,0,0
			newResidueLocation += proteinModelCenterLocation;										//this centers the residue about the new location

			currentAminoAcid->SetActorLocation(newResidueLocation);
			currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
		}

		//offset the bounding box
		m_minBounds3D = m_minBounds3D - m_centerOfBoundingBox + proteinModelCenterLocation;
		m_maxBounds3D = m_maxBounds3D - m_centerOfBoundingBox + proteinModelCenterLocation;
		m_centerOfBoundingBox = proteinModelCenterLocation;
	}

	void ProteinModel::SpawnAminoAcids(UWorld* world, UClass* blueprint, float aminoAcidSize, const FVector& proteinModelCenterLocation
		, float linkWidth, float linkHeight, float distanceScale, const FColor& helixColor, const FColor& betaStrandColor, float helixLinkWidth
		, float betaStrandLinkWidth, float hydrogenBondLinkWidth)
	{
		m_linkWidth = linkWidth;
		m_linkHeight = linkHeight;
		m_hydrogenBondLinkWidth = hydrogenBondLinkWidth;

		if (!world || !blueprint || aminoAcidSize <= 0.f)
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

			//iterate over all of the amino acids and spawn an actor for each one of them
			for (int residueIndex = 0; residueIndex < m_residueVector.Num(); ++residueIndex)
			{
				currentResidue = m_residueVector[residueIndex];
				currentResidue->GetCALocation(aminoAcidLocation);
				aminoAcidLocation *= distanceScale; // this is done in order to space out the proteins

				currentAminoAcid = UThesisStaticLibrary::SpawnBP<AAminoAcid>(world, blueprint, aminoAcidLocation, originRotation);
				currentAminoAcid->SetAminoAcidSize(aminoAcidSize);
				currentAminoAcid->SetResidueInformation(currentResidue);
				currentAminoAcid->SetParentModel(this);

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

				//set the residues secondary structure and updates the current secondary structure
				currentAminoAcid->SetSecondaryStructure(currentResidue->GetSecondaryStructure());

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
				currentAminoAcid->SpawnLinkParticleToNextAminoAcid(linkWidth, linkHeight);
				currentAminoAcid->SetRenderProperties(helixColor, betaStrandColor, helixLinkWidth, betaStrandLinkWidth);
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

	HydrogenBond* ProteinModel::SpawnHydrogenBond(AAminoAcid* residue1, AAminoAcid* residue2)
	{
		//Get the size of the hydrogen bond link fragment from the game mode
		FVector startLocation = residue1->GetActorLocation();
		FVector endLocation = residue2->GetActorLocation();
		UWorld* world = residue1->GetWorld();
		UClass* linkFragmentClass = residue1->GetDetaultLinkFragmentClass();

		ALinkFragment* linkChain = UThesisStaticLibrary::SpawnBP<ALinkFragment>(world, linkFragmentClass,
			FVector::ZeroVector, FRotator::ZeroRotator);

		linkChain->SplineMeshComponent->SetStartPosition(startLocation);
		linkChain->SplineMeshComponent->SetEndPosition(endLocation);

		HydrogenBond* newHydrogenBond = new HydrogenBond(residue1, residue2, linkChain, m_linkHeight, m_hydrogenBondLinkWidth);

		//should probably add the hydrogen bond into the array of current hydrogen bonds
		m_hydrogenBonds.Add(newHydrogenBond);

		return newHydrogenBond;
	};

	void ProteinModel::HighlightSecondaryStructure(AAminoAcid* residueMember)
	{
		//iterate through the secondary structures to see what to highlight
		for (SecondaryStructure* currentSecondaryStructure = m_headSecondaryStructure;
			currentSecondaryStructure != nullptr && (m_tailSecondaryStructure && currentSecondaryStructure != m_tailSecondaryStructure->GetNextStructurePtr());
			currentSecondaryStructure = currentSecondaryStructure->GetNextStructurePtr())
		{
			if (currentSecondaryStructure->ContainsSpecifiedResidue(residueMember))
			{
				currentSecondaryStructure->SetSelected();
				break;
			}
		}
	}

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

	void ProteinModel::AppendSecondaryStructure(SecondaryStructure* secondaryStructure)
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
		if (secondaryStructure->GetSecondaryStructureType() == ESecondaryStructure::ssStrand)
		{
			AddBetaStrand(secondaryStructure);
		}
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

		/*
		//update links
		currentAminoAcid = m_headPtr;
		
		while (currentAminoAcid)
		{
			//currentAminoAcid->UpdateLinkToNextAminoAcid();
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
		*/
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

		//update links
		/*
		currentAminoAcid = m_headPtr;
		FVector aminoAcidLocation;
		while (currentAminoAcid)
		{
			currentAminoAcid->UpdateLinkToNextAminoAcid();
			currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
		}
		*/

		//translate the bounding box min and max
		m_minBounds3D += displacement;
		m_maxBounds3D += displacement;
		m_centerOfBoundingBox += displacement;

		//displace the hydrogen bonds
		for (int i = 0; i < m_hydrogenBonds.Num(); ++i)
		{
			m_hydrogenBonds[i]->Translate(displacement);
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

	void ProteinModel::HideHydrogenBonds()
	{
		for (int i = 0; i < m_hydrogenBonds.Num(); ++i)
		{
			m_hydrogenBonds[i]->Hide();
		}
	}

	void ProteinModel::BreakFirstSpiral()
	{
		//look for the first spiral and start modifying it
		SecondaryStructure* test = m_headSecondaryStructure;

		while (test && test->GetSecondaryStructureType() != ESecondaryStructure::ssAlphaHelix)
		{
			test = test->GetNextStructurePtr();
		}

		//modify this alpha helix
		if (test)
		{
			test->BreakStructure();
		}
	}

	UWorld* ProteinModel::GetWorld()
	{
		return m_world;
	}
}