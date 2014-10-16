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
		, m_prevTemperature(23.5)
		, m_irreversibleChangeTemperatureCelsius(50.f)
		, m_breakTemperature(41.f)
		, m_regularTemperature(23.5f)
		, m_canReverseChange(true)
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
	void SetTemperature(float temperatureCelsius);

private:
	FRotator m_relativeRotation;
	AAminoAcid* m_bondResidues[2];
	ALinkFragment* m_linkFragment;
	float m_prevTemperature;
	float m_irreversibleChangeTemperatureCelsius;
	float m_breakTemperature;
	float m_regularTemperature;
	bool m_canReverseChange;
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
	void AddToListOfModifiedResidues(AAminoAcid* residueModified);
	void RemoveFromListOfModifiedResidues(AAminoAcid* residueModified);

	static SecondaryStructure* GetSelectedStructure();

private:
	void AddBridgeLabel(uint32 bridgeLabel);
	void ExtractResidues(TArray<AAminoAcid*>& out_residuecontainer);
	void BreakStructure(const TArray<AAminoAcid*>& residues);
	void ShakeResidues(TArray<AAminoAcid*>& residues);
	void StabilizeResidues(TArray<AAminoAcid*>& residues);

private:
	ESecondaryStructure::Type m_secondaryStructureType;
	SecondaryStructure* m_nextSecondaryStructure;

	AAminoAcid* m_headAminoAcid;
	AAminoAcid* m_tailAminoAcid;

	TArray<uint32> m_bridgeLabels;
	TArray<HydrogenBond*> m_hydrogenBonds;
	TArray<AAminoAcid*> m_modifiedResidues;

	float m_irreversibleChangeTemperatureCelsius;
	float m_breakTemperature;
	float m_regularTemperature;
	float m_prevTemperature;

	bool m_canReverseChange;

	GHProtein::ProteinModel* m_parentModel;

	static SecondaryStructure* s_selectedStructure;
public:
	
};