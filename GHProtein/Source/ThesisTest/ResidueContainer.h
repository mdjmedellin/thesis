#pragma once
#ifndef __Residue_Container_h__
#define __Residue_Container_h__

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

	private:
		TArray< Residue* > m_residues;
		TMap< Residue*, int > PointerResidueIDMap;
	};
}

#endif