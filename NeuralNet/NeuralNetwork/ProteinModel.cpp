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
		//this brings an interesting problem
		//I want to create something i have fast access to, but I also want to create something I
		//could traverse really quickly in order

		//I know I am going to need a map
		ResidueMapIterator residueLocation = ResidueIDMap.find(residueAttemptingToInsert->GetNumber());

		if (residueLocation != ResidueIDMap.end())
		{
			if (!residueLocation->second)
			{
				residueLocation->second = new ResidueContainer();
				m_residueContainers.push_back(residueLocation->second);
			}

			if (!residueLocation->second->AddResidue(residueAttemptingToInsert))
			{
				return false;
			}
		}
		else
		{
			ResidueContainer* newContainer = new ResidueContainer();
			newContainer->AddResidue(residueAttemptingToInsert);
			m_residueContainers.push_back(newContainer);
			ResidueIDMap.insert(std::pair<int, ResidueContainer*>(residueAttemptingToInsert->GetNumber(),newContainer));
		}

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
}