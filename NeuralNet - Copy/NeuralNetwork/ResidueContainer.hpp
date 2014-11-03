#pragma once
#ifndef RESIDUE_CONTAINER_H
#define RESIDUE_CONTAINER_H

#include <vector>
#include <map>

namespace GHProtein
{
	class Residue;

	typedef std::map< Residue*, int >::iterator PointerResidueIDMapIterator;
	class ResidueContainer
	{
	public:
		~ResidueContainer();

	public:
		bool AddResidue(Residue* residueAttemptingToInsert);

	private:
		std::vector< Residue* > m_residues;
		std::map< Residue*, int > PointerResidueIDMap;
	};
}

#endif