#include "ProteinModel.hpp"
#include "ResidueContainer.hpp"

namespace GHProtein
{
	ProteinModel::ProteinModel()
		: m_headPtr(nullptr)
		, m_headSecondaryStructure(nullptr)
		, m_tailSecondaryStructure(nullptr)
	{}

	ProteinModel::~ProteinModel()
	{
		ResidueIDMap.clear();

		//delete all the residue containers
		for (int i = 0; i < m_residueContainers.size(); ++i)
		{
			delete m_residueContainers[i];
		}

		//delete all the residues
		for (int i = 0; i < m_residueVector.size(); ++i)
		{
			delete m_residueVector[i];
		}
	}

	bool ProteinModel::AddResidue(Residue* residueAttemptingToInsert)
	{
		m_residueVector.push_back(residueAttemptingToInsert);
		return true;
	}

	void ProteinModel::BuildProteinModel()
	{
		int numberOfResidues = m_residueVector.size();
		Residue* currentResidue = nullptr;
		Residue* prevResidue = nullptr;

		//set the previous and next residues
		for (int residueIndex = 0; residueIndex < numberOfResidues; ++residueIndex)
		{
			//grab the current residue
			currentResidue = m_residueVector[residueIndex];

			//set the next and previous residue
			currentResidue->SetPrevious(prevResidue);
			if (prevResidue != nullptr)
			{
				prevResidue->SetNext(currentResidue);
			}
			prevResidue = currentResidue;
		}

		//current residue now holds the last residue
		currentResidue->SetNext(nullptr);

		return;
	}

	int ProteinModel::GetLengthOfChain() const
	{
		return m_residueVector.size();
	}

	void ProteinModel::GetInputValues(int startingResidue, std::vector< std::vector<double> >& out_inputs,
		int numberOfInputsToExtract) const
	{
		out_inputs.clear();

		std::vector< double > residueVectorRepresentation(21, 0.0);

		//figure the width of the window, the starting residue is always the middle
		int windowWidth = (numberOfInputsToExtract - 1) * .5f;

		for (int residueIndex = startingResidue - windowWidth; residueIndex <= startingResidue + windowWidth; ++residueIndex)
		{
			//check if we are to create a null residue input
			if (residueIndex < 0
				|| residueIndex > m_residueVector.size() - 1)
			{
				Residue::GetVectorRepresentationOfResidue(residueVectorRepresentation);
			}
			else
			{
				Residue::GetVectorRepresentationOfResidue(residueVectorRepresentation, m_residueVector[residueIndex]->GetType());
			}

			out_inputs.push_back(residueVectorRepresentation);
		}
	}

	void ProteinModel::GetOutputValues(int residueIndex, std::vector< double >& out_outputs) const
	{
		out_outputs.resize(3);
		std::fill(out_outputs.begin(), out_outputs.end(), 0.0);

		if (residueIndex >= 0
			&& residueIndex > out_outputs.size())
		{
			Residue::GetVectorRepresentationOfSecondaryStructure(out_outputs, m_residueVector[residueIndex]->GetSecondaryStructure());
		}
	}
}