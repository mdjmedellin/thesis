#pragma once

#include "ThesisStaticLibrary.h"
#include "LinkFragment.h"

class AAminoAcid;
class AHydrogenBond;
class SecondaryStructure;

namespace GHProtein
{
	class ProteinModel;
}

class BetaSheet
{
public:
	BetaSheet(SecondaryStructure* strand1, SecondaryStructure* strand2, GHProtein::ProteinModel* parentModel);
	~BetaSheet();
	void SpawnHydrogenBonds();
	void SpawnHydrogenBondsOfSpecifiedResidue(AAminoAcid* residue);

public:
	TArray<AHydrogenBond*> m_hydrogenBonds;
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

	void SetEnviromentalProperties(float currentTemperatureCelsius, float regularTemperatureCelsius,
		float breakTemperatureCelsius, float irreversibleChangeTemperatureCelsius);
	void AppendAminoAcid(AAminoAcid* residue, bool isCustomChain = false);
	SecondaryStructure* GetNextStructurePtr();
	SecondaryStructure* GetPreviousSecondaryStructure();
	AAminoAcid* GetHeadResidue();
	AAminoAcid* GetEndResidue();
	int GetAminoAcidCount();
	bool ContainsSpecifiedResidue(AAminoAcid* residue);
	ESecondaryStructure::Type GetSecondaryStructureType() const;
	void GetBridgeLabels(TArray<uint32>& out_bridgeLabels) const;
	bool IsPartOfSpecifiedBridgeLabels(const TArray<uint32>& bridgeLabels) const;
	AAminoAcid* GetAminoAcidWithSpecifiedId(int sequenceNumber);
	void SpawnHydrogenBonds();
	bool SetTemperature(float temperatureCelsius);
	void AddToListOfModifiedResidues(AAminoAcid* residueModified);
	void RemoveFromListOfModifiedResidues(AAminoAcid* residueModified);
	void RemoveReferencesToAminoAcid(AAminoAcid* referenceToRemove);
	void SetNextStructurePtr(SecondaryStructure* nextStructure);
	void SetPreviousSecondaryStructure(SecondaryStructure* previousSecondaryStructure);

	static SecondaryStructure* GetSelectedStructure();

private:
	void AddBridgeLabel(uint32 bridgeLabel);
	void ExtractResidues(TArray<AAminoAcid*>& out_residuecontainer);
	void BreakStructure(const TArray<AAminoAcid*>& residues);
	void ShakeResidues(TArray<AAminoAcid*>& residues);
	void StabilizeResidues(TArray<AAminoAcid*>& residues);
	bool UpdateStructureAccordingToSpecifiedTemperature(float temperatureCelsius);

private:
	ETemperatureState::Type m_temperatureState;
	ESecondaryStructure::Type m_secondaryStructureType;
	SecondaryStructure* m_nextSecondaryStructure;
	SecondaryStructure* m_previousSecondaryStructure;

	AAminoAcid* m_headAminoAcid;
	AAminoAcid* m_tailAminoAcid;

	TArray<uint32> m_bridgeLabels;
	TArray<AHydrogenBond*> m_hydrogenBonds;
	TArray<AAminoAcid*> m_modifiedResidues;

	float m_irreversibleChangeTemperatureCelsius;
	float m_breakTemperatureCelsius;
	float m_regularTemperatureCelsius;
	float m_prevTemperatureCelsius;
	bool m_canReverseChange;

	GHProtein::ProteinModel* m_parentModel;

public:
	
};