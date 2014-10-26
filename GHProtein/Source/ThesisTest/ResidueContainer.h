#pragma once

class Residue;

namespace GHProtein
{
	class ResidueContainer
	{
	public:
		~ResidueContainer();

	public:
		bool AddResidue(Residue* residueAttemptingToInsert);
		Residue* FindBetaPartnerToResidue(Residue* partnerResidue);
		Residue* GetFirstResidue();

	private:
		TArray< Residue* > m_residues;
		TMap< Residue*, int > PointerResidueIDMap;
	};
}