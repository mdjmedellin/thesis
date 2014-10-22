#pragma once
#ifndef PROTEIN_MODEL_H
#define PROTEIN_MODEL_H

#include "Residue.hpp"
#include <map>
#include <vector>

namespace GHProtein
{
	class AAminoAcid;
	class SecondaryStructure;
	class ResidueContainer;

	typedef std::map< int, ResidueContainer* >::iterator ResidueMapIterator;

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
		void AppendSecondaryStructure(SecondaryStructure* secondaryStructure);
		void BuildProteinModel();
		int GetLengthOfChain() const;
		void GetInputValues(int startingResidue, std::vector< std::vector<double> >& out_inputs, int numberOfInputsToExtract) const;
		void GetOutputValues(int startingResidue, std::vector< double >& out_outputs) const;

		AAminoAcid* GetAminoAcidWithSpecifiedId(int sequenceNumber);

	private:
		/** Private utility methods go here */
		AAminoAcid* m_headPtr;

	public:
		/** public data members go here */

	private:
		/** private data members go here */
		std::map< int, ResidueContainer* > ResidueIDMap;
		std::vector< Residue* > m_residueVector;
		std::vector< ResidueContainer* > m_residueContainers;

		SecondaryStructure* m_headSecondaryStructure;
		SecondaryStructure* m_tailSecondaryStructure;
	};
}

#endif