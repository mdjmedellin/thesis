#pragma once

#include "ThesisStaticLibrary.h"
#include "LinkFragment.h"

class AAminoAcid;
class SecondaryStructure;

namespace GHProtein
{
	class ProteinModel;
}

class HydrogenBond
{
public:
	HydrogenBond(AAminoAcid* startResidue, AAminoAcid* endResidue, ALinkFragment* linkFragment)
	: m_linkFragment(linkFragment)
	, m_relativeRotation(FRotator::ZeroRotator)
	{
		m_bondResidues[0] = startResidue;
		m_bondResidues[1] = endResidue;

		m_linkFragment->ChangeLinkType(ELinkType::ELink_HydrogenBond);
	};

	bool ContainsSpecifiedResidue(const AAminoAcid* residue)
	{
		for (int i = 0; i < 2; ++i)
		{
			if (m_bondResidues[i] == residue)
			{
				return true;
			}
		}

		return false;
	}

	void Hide()
	{
		m_linkFragment->Hide();
	}

	void ToggleBreaking();
	void ToggleShake();
	void Translate(const FVector& displacement);
	void RotateAboutSpecifiedPoint(const FRotationMatrix& rotation, const FVector& rotationPoint);
	void ChangeLocationOfAssociatedEnd(AAminoAcid* aminoAcidEnd, const FVector& newLocation);

private:
	FRotator m_relativeRotation;
	AAminoAcid* m_bondResidues[2];
	ALinkFragment* m_linkFragment;
};

class BetaSheet
{
public:
	BetaSheet(SecondaryStructure* strand1, SecondaryStructure* strand2, GHProtein::ProteinModel* parentModel);
	void SpawnHydrogenBonds();
	void SpawnHydrogenBondsOfSpecifiedResidue(AAminoAcid* residue);

	TArray<SecondaryStructure*> m_strands;
	GHProtein::ProteinModel* m_proteinModel;
};

/**
 * 
 */
class THESISTEST_API SecondaryStructure
{
public:
	SecondaryStructure(ESecondaryStructure::Type secondaryStructureType, GHProtein::ProteinModel* parentModel);
	~SecondaryStructure();

	void SetNextStructurePtr(SecondaryStructure* nextStructure);
	void AppendAminoAcid(AAminoAcid* residue);
	/*
	void SetSelected();
	void Deselect();
	*/
	SecondaryStructure* GetNextStructurePtr();
	AAminoAcid* GetHeadResidue();
	AAminoAcid* GetEndResidue();
	bool ContainsSpecifiedResidue(AAminoAcid* residue);
	ESecondaryStructure::Type GetSecondaryStructureType() const;
	void GetBridgeLabels(TArray<uint32>& out_bridgeLabels) const;
	bool IsPartOfSpecifiedBridgeLabels(const TArray<uint32>& bridgeLabels) const;
	AAminoAcid* GetAminoAcidWithSpecifiedId(int sequenceNumber);
	void SpawnHydrogenBonds();
	void SetTemperature(float temperatureCelsius);

	static SecondaryStructure* GetSelectedStructure();

private:
	void AddBridgeLabel(uint32 bridgeLabel);
	void ExtractResidues(TArray<AAminoAcid*>& out_residuecontainer);
	void BreakStructure(const TArray<AAminoAcid*>& residues);
	void ShakeResidues(TArray<AAminoAcid*>& residues);
	void StabilizeResidues(TArray<AAminoAcid*>& residues);
	/*
	void ChangeRibbonColor(const FColor& ribbonColor);
	void ResetRibbonColor();
	*/

private:
	ESecondaryStructure::Type m_secondaryStructureType;
	SecondaryStructure* m_nextSecondaryStructure;

	AAminoAcid* m_headAminoAcid;
	AAminoAcid* m_tailAminoAcid;

	TArray<uint32> m_bridgeLabels;
	TArray<HydrogenBond*> m_hydrogenBonds;

	float m_irreversibleChangeTemperatureCelsius;
	float m_breakTemperature;
	float m_regularTemperature;
	float m_prevTemperature;

	bool m_canReverseChange;

	GHProtein::ProteinModel* m_parentModel;

	static SecondaryStructure* s_selectedStructure;
public:
	
};