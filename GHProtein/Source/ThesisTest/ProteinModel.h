#pragma once
#ifndef __Protein_Model_h__
#define __Protein_Model_h__

#include "ThesisStaticLibrary.h"
#include "AminoMovementComponent.h"
#include "Residue.h"

/** JM: Protein Model is intended to be the actual structure of the protein */
namespace GHProtein
{
	class ResidueContainer;

	class ProteinModel
	{
	public:
		/** Constructor */
		ProteinModel();

		/** Destructor */
		~ProteinModel();

	private:

		/** No conversion allowed */
		ProteinModel(const ProteinModel& rhs) {};
		/** No assignment allowed */
		ProteinModel& operator=(const ProteinModel& rhs) { return *this; };

	public:
		/** Public utility methods go here */
		bool AddResidue(Residue* insertedResidue);
		void BuildProteinModel();
		void SpawnAminoAcids(UWorld* world, UClass* blueprint, const FVector& locationOffset);

	private:
		/** Private utility methods go here */
		FVector m_minBounds3D;
		FVector m_maxBounds3D;

	public:
		/** public data members go here */

	private:
		/** private data members go here */
		TMap< int, ResidueContainer* > ResidueIDMap;
		TArray< Residue* > m_residueVector;
		TArray< ResidueContainer* > m_residueContainers;
	};
}

#endif