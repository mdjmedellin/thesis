#include "ThesisTest.h"
#include "ProteinModel.h"
#include "SecondaryStructure.h"
#include "AminoAcid.h"
#include "LinkFragment.h"

//========================HydrogenBond========================
void HydrogenBond::Translate(const FVector& displacement)
{
	m_linkFragment->SetActorLocation(m_linkFragment->GetActorLocation() + displacement);
}

void HydrogenBond::RotateAboutSpecifiedPoint(const FRotationMatrix& rotationMatrix, const FVector& rotationPoint)
{
	FVector distanceFromPivotPoint = m_linkFragment->GetActorLocation() - rotationPoint;

	//rotate the distance
	distanceFromPivotPoint = rotationMatrix.TransformVector(distanceFromPivotPoint);
	//now set the new location to the fragment
	m_linkFragment->SetActorLocation(rotationPoint + distanceFromPivotPoint);

	//rotate the object
	FRotationMatrix currentRotationMatrix(m_linkFragment->GetActorRotation());
	currentRotationMatrix *= rotationMatrix;
	m_linkFragment->SetActorRotation(currentRotationMatrix.Rotator());

	/*
	FVector distanceFromPivotPoint = m_linkFragment->GetActorLocation() - rotationPoint;

	//rotate the distance
	distanceFromPivotPoint = rotation.RotateVector(distanceFromPivotPoint);
	//now set the new location to the fragment
	m_linkFragment->SetActorLocation(rotationPoint + distanceFromPivotPoint);

	//rotate the object
	FRotationMatrix currentRotationMatrix(m_linkFragment->GetActorRotation());
	FRotationMatrix newRotation(rotation);
	currentRotationMatrix *= newRotation;
	m_linkFragment->SetActorRotation(currentRotationMatrix.Rotator());
	*/
}
//============================================================

//======================BetaSheet===========================
BetaSheet::BetaSheet(SecondaryStructure* strand1, SecondaryStructure* strand2, GHProtein::ProteinModel* parentModel)
: m_proteinModel(parentModel)
{
	m_strands.Add(strand1);
	m_strands.Add(strand2);
}

void BetaSheet::SpawnHydrogenBonds()
{
	//we are going to need to iterate through all the strands
	SecondaryStructure* outerStrand = nullptr;
	AAminoAcid* currentOuterResidue = nullptr;
	AAminoAcid* endOuterResidue = nullptr;
	for (int outerIndex = 0; outerIndex < m_strands.Num(); ++outerIndex)
	{
		//get the current strand and iterate and spawn the hydrogen bonds of all its residues
		outerStrand = m_strands[outerIndex];
		currentOuterResidue = outerStrand->GetHeadResidue();
		endOuterResidue = outerStrand->GetEndResidue();
		bool continueIteratingOverResidues = currentOuterResidue != nullptr;
		while (continueIteratingOverResidues)
		{
			//spawn the hydrogen bond of the current residue
			SpawnHydrogenBondsOfSpecifiedResidue(currentOuterResidue);

			//check end condition
			if (currentOuterResidue == endOuterResidue)
			{
				continueIteratingOverResidues = false;
			}
			else
			{
				currentOuterResidue = currentOuterResidue->GetNextAminoAcidPtr();
			}
		}
	}
}

void BetaSheet::SpawnHydrogenBondsOfSpecifiedResidue(AAminoAcid* residue)
{
	const Residue* residueInfo = residue->GetResidueInformation();

	//a residue can only have to bridge partners
	//one on each lateral side
	for (int i = 0; i < 2; ++i)
	{
		BridgePartner partner = residueInfo->GetBetaPartner(i);

		if (partner.residue)
		{
			AAminoAcid* betaPartner = m_proteinModel->GetAminoAcidWithSpecifiedId(partner.number);

			//if the residue has a valid partner and a bond between them has not been created
			//spawn a bond between them
			if (betaPartner && !residue->BondWithResidueExists(betaPartner))
			{
				//if the bond has not yet been created, then we create it
				HydrogenBond* hBond = m_proteinModel->SpawnHydrogenBond(residue, betaPartner);
				residue->AddHydrogenBond(hBond);
				betaPartner->AddHydrogenBond(hBond);

				//add bond to the array of hydrogen bonds in the beta sheet

			}
		}
	}
}
//==========================================================

SecondaryStructure* SecondaryStructure::s_selectedStructure = nullptr;

SecondaryStructure::SecondaryStructure(ESecondaryStructure::Type secondaryStructureType)
: m_secondaryStructureType(secondaryStructureType)
, m_nextSecondaryStructure(nullptr)
, m_headAminoAcid(nullptr)
, m_tailAminoAcid(nullptr)
{}

SecondaryStructure::~SecondaryStructure()
{}

void SecondaryStructure::SetSelected()
{
	if (s_selectedStructure)
	{
		s_selectedStructure->Deselect();
	}

	s_selectedStructure = this;

	//perform other selected operations here, such as changing colors of the materials
	ChangeRibbonColor(FColor::Red);
}

AAminoAcid* SecondaryStructure::GetAminoAcidWithSpecifiedId(int sequenceNumber)
{
	AAminoAcid* foundResidue = nullptr;
	AAminoAcid* currentResidue = m_headAminoAcid;

	while (currentResidue && currentResidue != m_tailAminoAcid->GetNextAminoAcidPtr())
	{
		if (currentResidue->GetSequenceNumber() == sequenceNumber)
		{
			foundResidue = currentResidue;
			break;
		}
		else
		{
			currentResidue = currentResidue->GetNextAminoAcidPtr();
		}
	}

	return foundResidue;
}

void SecondaryStructure::Deselect()
{
	if (this == s_selectedStructure)
	{
		ResetRibbonColor();
	}
}

void SecondaryStructure::ResetRibbonColor()
{
	//iterate over all of the amino acids on this structure and set the color to the ribbon color
	for (AAminoAcid* currentResidue = m_headAminoAcid;
		currentResidue != nullptr && (m_tailAminoAcid && currentResidue != m_tailAminoAcid->GetNextAminoAcidPtr());
		currentResidue = currentResidue->GetNextAminoAcidPtr())
	{
		currentResidue->ResetLinkFragmentColorToDefault();
	}
}

void SecondaryStructure::ChangeRibbonColor(const FColor& ribbonColor)
{
	//iterate over all of the amino acids on this structure and set the color to the ribbon color
	for (AAminoAcid* currentResidue = m_headAminoAcid; 
		currentResidue != nullptr && (m_tailAminoAcid && currentResidue != m_tailAminoAcid->GetNextAminoAcidPtr());
		currentResidue = currentResidue->GetNextAminoAcidPtr())
	{
		currentResidue->SetLinkFragmentColor(ribbonColor);
	}
}

void SecondaryStructure::SetNextStructurePtr(SecondaryStructure* nextStructure)
{
	//check if we alreay have a vaird ptr to the next secondary structure
	if (m_nextSecondaryStructure)
	{
		SecondaryStructure* tmp = m_nextSecondaryStructure;
		nextStructure->SetNextStructurePtr(tmp);
	}
	
	m_nextSecondaryStructure = nextStructure;
}

void SecondaryStructure::AppendAminoAcid(AAminoAcid* residue)
{
	//Get the residue information
	const Residue* residueInfo = residue->GetResidueInformation();

	//if we do not have an amino acid, then we take the first amino acid as the head amino acid
	if (m_headAminoAcid == nullptr)
	{
		m_headAminoAcid = residue;
		//set the structure type from the head amino acid
		m_secondaryStructureType = residueInfo->GetSecondaryStructure();
	}
	else
	{
		//if we already have a head amino acid, then we take the latest appended amino acid as the tail
		m_tailAminoAcid = residue;
	}

	//if this is a beta strand, make sure to save the partners
	if (m_secondaryStructureType == ESecondaryStructure::ssStrand)
	{
		//extract the character that indicates the partner strand
		BridgePartner currentPartner;
		for (int betaPartnerIndex = 0; betaPartnerIndex < 2; ++betaPartnerIndex)
		{
			currentPartner = residueInfo->GetBetaPartner(betaPartnerIndex);
			if (currentPartner.ladder != ' ')
			{
				AddBridgeLabel(currentPartner.ladder);
			}
		}
	}
}

void SecondaryStructure::AddBridgeLabel(uint32 bridgeLabel)
{
	//if we do not have the label associated with this strand, then we add it to the list of associated labels
	if (m_bridgeLabels.Find(bridgeLabel) == INDEX_NONE)
	{
		m_bridgeLabels.Add(bridgeLabel);
	}
}

bool SecondaryStructure::IsPartOfSpecifiedBridgeLabels(const TArray<uint32>& bridgeLabels) const
{
	//check if this secondary structure shares at least one of the bridge labels passed in
	for (int j = 0; j < bridgeLabels.Num(); ++j)
	{
		if (bridgeLabels[j] == ' ')
		{
			continue;
		}
		else
		{
			for (int i = 0; i < m_bridgeLabels.Num(); ++i)
			{
				if (bridgeLabels[j] == m_bridgeLabels[i])
				{
					return true;
				}
			}
		}
	}

	return false;
}

void SecondaryStructure::GetBridgeLabels(TArray<uint32>& out_bridgeLabels) const
{
	out_bridgeLabels = m_bridgeLabels;
}

ESecondaryStructure::Type SecondaryStructure::GetSecondaryStructureType() const
{
	return m_secondaryStructureType;
}

SecondaryStructure* SecondaryStructure::GetNextStructurePtr()
{
	return m_nextSecondaryStructure;
}

AAminoAcid* SecondaryStructure::GetHeadResidue()
{
	return m_headAminoAcid;
}

AAminoAcid* SecondaryStructure::GetEndResidue()
{
	return m_tailAminoAcid;
}

bool SecondaryStructure::ContainsSpecifiedResidue(AAminoAcid* residue)
{
	//iterate through the list of amino acids and see if the specified residue is there
	for (AAminoAcid* currentResidue = m_headAminoAcid;
		currentResidue != nullptr && (m_tailAminoAcid && currentResidue != m_tailAminoAcid->GetNextAminoAcidPtr());
		currentResidue = currentResidue->GetNextAminoAcidPtr())
	{
		//if both pointers are pointing to the same address then we are pointing to the same residue
		if (currentResidue == residue)
		{
			return true;
		}
	}

	return false;
}