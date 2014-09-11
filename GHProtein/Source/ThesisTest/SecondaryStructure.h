

#pragma once

class AAminoAcid;
/**
 * 
 */
class THESISTEST_API SecondaryStructure
{
public:
	SecondaryStructure();
	~SecondaryStructure();

	void SetNextStructurePtr(SecondaryStructure* nextStructure);
	void AppendAminoAcid(AAminoAcid* residue);
	void SetSelected();
	void Deselect();
	SecondaryStructure* GetNextStructurePtr();
	bool ContainsSpecifiedResidue(AAminoAcid* residue);

	static SecondaryStructure* GetSelectedStructure();

private:
	void ChangeRibbonColor(const FColor& ribbonColor);
	void ResetRibbonColor();

private:
	SecondaryStructure* m_nextSecondaryStructure;

	AAminoAcid* m_headAminoAcid;
	AAminoAcid* m_tailAminoAcid;

	static SecondaryStructure* s_selectedStructure;
public:
	
};
