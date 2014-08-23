#pragma once
#ifndef __Protein_Model_h__
#define __Protein_Model_h__

class Residue;

/** JM: Protein Model is intended to be the actual structure of the protein */
namespace GHProtein
{
	class ResidueContainer;

	class PROTEINFOLDING_API ProteinModel
	{
	public:

		/* Constructor */
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
		void SpawnAminoAcids(UWorld* const World);

	private:

		/** Private utility methods go here */

	public:
		/** public data members go here */
		UClass* m_aminoAcidBlueprint;

	private:
		/** private data members go here */
		TMap< int, ResidueContainer* > ResidueIDMap;
		TArray< Residue* > m_residueVector;
		TArray< ResidueContainer* > m_residueContainers;
	};
}

#endif