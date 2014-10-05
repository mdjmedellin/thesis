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
	HydrogenBond(AAminoAcid* residue1, AAminoAcid* residue2, ALinkFragment* linkFragment,
		float linkHeight, float linkWidth)
	: m_linkFragment(linkFragment)
	, m_relativeRotation(FRotator::ZeroRotator)
	, m_linkHeight(linkHeight)
	, m_linkWidth(linkWidth)
	{
		m_bondResidues[0] = residue1;
		m_bondResidues[1] = residue2;

		//scale the link fragment to the appropriate size
		FVector size = linkFragment->SplineMeshComponent->StaticMesh->GetBounds().GetBox().GetSize();
		FVector2D scale(1.f, 1.f);
		scale.X = m_linkWidth / size.X;
		scale.Y = m_linkHeight / size.Y;

		linkFragment->SplineMeshComponent->SetStartScale(scale);
		linkFragment->SplineMeshComponent->SetEndScale(scale);
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

	void ToggleShake();
	void Translate(const FVector& displacement);
	void RotateAboutSpecifiedPoint(const FRotationMatrix& rotation, const FVector& rotationPoint);

private:
	FRotator m_relativeRotation;
	AAminoAcid* m_bondResidues[2];
	ALinkFragment* m_linkFragment;
	float m_linkWidth;
	float m_linkHeight;
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
	void SpawnHydrogenBonds();
	void BreakStructure();

	static SecondaryStructure* GetSelectedStructure();

private:
	void AddBridgeLabel(uint32 bridgeLabel);
	void ChangeRibbonColor(const FColor& ribbonColor);
	void ResetRibbonColor();
	void TestLineFitting(TArray<AAminoAcid*>& residues);

private:
	ESecondaryStructure::Type m_secondaryStructureType;
	SecondaryStructure* m_nextSecondaryStructure;

	AAminoAcid* m_headAminoAcid;
	AAminoAcid* m_tailAminoAcid;

	TArray<uint32> m_bridgeLabels;
	TArray<HydrogenBond*> m_hydrogenBonds;

	GHProtein::ProteinModel* m_parentModel;

	static SecondaryStructure* s_selectedStructure;
public:
	
};