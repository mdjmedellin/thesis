

#include "ThesisTest.h"
#include "SecondaryStructure.h"
#include "AminoAcid.h"

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
			currentPartner = residueInfo->GetBetaPartner(0);
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