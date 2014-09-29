

#pragma once

#include "ThesisStaticLibrary.h"
#include "Residue.h"
class AAminoAcid;
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

	AAminoAcid* GetAminoAcidWithSpecifiedId(int sequenceNumber);

	static SecondaryStructure* GetSelectedStructure();

private:
	void ChangeRibbonColor(const FColor& ribbonColor);
	void ResetRibbonColor();

private:
	ESecondaryStructure::Type m_secondaryStructureType;
	SecondaryStructure* m_nextSecondaryStructure;

	AAminoAcid* m_headAminoAcid;
	AAminoAcid* m_tailAminoAcid;

	static SecondaryStructure* s_selectedStructure;
public:
	
};
