#include "ResidueContainer.hpp"
#include "Residue.hpp"

namespace GHProtein
{
	ResidueContainer::~ResidueContainer()
	{
		//JM
		//We do not delete the residues here because they are going to be deleted by the protein model
		//This is easier for the protein model because it has a vector it can iterate through and use
		//to quickly traverse items
		m_residues.clear();
		PointerResidueIDMap.clear();
	}

	bool ResidueContainer::AddResidue(Residue* residueAttemptingToInsert)
	{
		PointerResidueIDMapIterator residueIndex = PointerResidueIDMap.find(residueAttemptingToInsert);

		if (residueIndex == PointerResidueIDMap.end())
		{
			PointerResidueIDMap.insert(std::pair< Residue*, int >(residueAttemptingToInsert, m_residues.size()));
			m_residues.push_back(residueAttemptingToInsert);
			return true;
		}

		return false;
	}
}