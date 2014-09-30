#pragma once

#include "ThesisStaticLibrary.h"

class AAminoAcid;
class SecondaryStructure;

struct BetaSheet
{
	BetaSheet(SecondaryStructure* strand1, SecondaryStructure* strand2)
	{
		m_strands.Add(strand1);
		m_strands.Add(strand2);
	}

	TArray<SecondaryStructure*> m_strands;
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