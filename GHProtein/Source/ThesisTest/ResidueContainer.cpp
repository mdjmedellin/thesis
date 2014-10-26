#include "ThesisTest.h"
#include "Residue.h"
#include "ResidueContainer.h"

namespace GHProtein
{
	ResidueContainer::~ResidueContainer()
	{
		//JM
		//We do not delete the residues here because they are going to be deleted by the protein model
		//This is easier for the protein model because it has a vector it can iterate through and use
		//to quickly traverse items
		m_residues.Empty();
		PointerResidueIDMap.Empty();
	}

	bool ResidueContainer::AddResidue(Residue* residueAttemptingToInsert)
	{
		int* residueIndex = PointerResidueIDMap.Find(residueAttemptingToInsert);

		if (!residueIndex)
		{
			PointerResidueIDMap.Add(residueAttemptingToInsert, m_residues.Num());
			m_residues.Add(residueAttemptingToInsert);
			return true;
		}

		return false;
	}

	Residue* ResidueContainer::FindBetaPartnerToResidue(Residue* partnerResidue)
	{
		if (partnerResidue == nullptr)
		{
			return nullptr;
		}

		for (int index = 0; index < m_residues.Num(); ++index)
		{
			//we have to be careful here
			//what if the original residue was deleted
			//how do we take that into account?

			//the reason we do this extra check is because of how dssp assigns the number of residues
			//they cap the residue number at 9,999. Therefore there is a possibility for the residue
			//number to repeat and for us to set the wrong residue as the partner
			if (m_residues[index]->IsBetaPartnerOf(partnerResidue))
			{
				return m_residues[index];
			}
		}

		//at this point we did not find the partner to the residue
		return nullptr;
	}

	Residue* ResidueContainer::GetFirstResidue()
	{
		if (m_residues.Num() > 0)
		{
			return m_residues[0];
		}
		else
		{
			return nullptr;
		}
	}
}