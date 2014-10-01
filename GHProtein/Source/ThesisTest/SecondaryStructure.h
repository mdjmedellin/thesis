#pragma once

#include "ThesisStaticLibrary.h"

class AAminoAcid;
class SecondaryStructure;
class ALinkFragment;

namespace GHProtein
{
	class ProteinModel;
}

class HydrogenBond
{
public:
	HydrogenBond(AAminoAcid* residue1, AAminoAcid* residue2, ALinkFragment* linkFragment)
	: m_linkFragment(linkFragment)
	{
		m_bondResidues[0] = residue1;
		m_bondResidues[1] = residue2;
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

	void Translate(const FVector& displacement);
	void RotateAboutSpecifiedPoint(const FRotator& rotation, const FVector& rotationPoint);

private:
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
	SecondaryStructure(ESecondaryStructure::Type secondaryStructureType);
	~SecondaryStructure();

	void SetNextStructurePtr(SecondaryStructure* nextStructure);
	void AppendAminoAcid(AAminoAcid* residue);
	void SetSelected();
	void Deselect();
	SecondaryStructure* GetNextStructurePtr();
	AAminoAcid* GetHeadResidue();
	AAminoAcid* GetEndResidue();
	bool ContainsSpecifiedResidue(AAminoAcid* residue);
	ESecondaryStructure::Type GetSecondaryStructureType() const;
	void GetBridgeLabels(TArray<uint32>& out_bridgeLabels) const;
	bool IsPartOfSpecifiedBridgeLabels(const TArray<uint32>& bridgeLabels) const;
	AAminoAcid* GetAminoAcidWithSpecifiedId(int sequenceNumber);

	static SecondaryStructure* GetSelectedStructure();

private:
	void AddBridgeLabel(uint32 bridgeLabel);
	void ChangeRibbonColor(const FColor& ribbonColor);
	void ResetRibbonColor();

private:
	ESecondaryStructure::Type m_secondaryStructureType;
	SecondaryStructure* m_nextSecondaryStructure;

	AAminoAcid* m_headAminoAcid;
	AAminoAcid* m_tailAminoAcid;

	TArray<uint32> m_bridgeLabels;

	static SecondaryStructure* s_selectedStructure;
public:
	
};