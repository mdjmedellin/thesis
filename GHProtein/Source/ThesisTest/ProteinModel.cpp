#include "ThesisTest.h"
#include "ProteinModel.h"
#include "ResidueContainer.h"

namespace GHProtein
{
	ProteinModel::~ProteinModel()
	{
		ResidueIDMap.Empty();

		//delete all the residue containers
		for (int i = 0; i < m_residueContainers.Num(); ++i)
		{
			delete m_residueContainers[i];
		}

		//delete all the residues
		for (int i = 0; i < m_residueVector.Num(); ++i)
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
		//TArray< Residue* >* residueArray = ResidueIDMap.Find(insertedResidue->GetNumber());
		ResidueContainer** residues = ResidueIDMap.Find(residueAttemptingToInsert->GetNumber());

		if (residues && *residues)
		{
			//we should probably check to see if we have not yet inserted this element
			if (!(*residues)->AddResidue(residueAttemptingToInsert))
			{
				return false;
			}
		}
		else
		{
			ResidueContainer* newContainer = new ResidueContainer();
			newContainer->AddResidue(residueAttemptingToInsert);
			m_residueContainers.Add(newContainer);
			ResidueIDMap.Add(residueAttemptingToInsert->GetNumber(),
				newContainer);
		}

		m_residueVector.Add(residueAttemptingToInsert);
		return true;
	}

	void ProteinModel::BuildProteinModel()
	{
		int numberOfResidues = m_residueVector.Num();
		Residue* currentResidue = nullptr;
		Residue* betaPartnerResidue = nullptr;
		Residue* prevResidue = nullptr;
		BridgePartner currentBridgePartner;
		char currentBridgeLabel = ' ';

		//set the beta partners along with the previous and next residues
		for (int residueIndex = 0; residueIndex < numberOfResidues; ++residueIndex)
		{
			//grab the current residue
			currentResidue = m_residueVector[residueIndex];

			//Get the beta partners
			for (int betaPartnerID = 0; betaPartnerID < 2; ++betaPartnerID)
			{
				currentBridgePartner = currentResidue->GetBetaPartner(betaPartnerID);

				if (currentBridgePartner.ladder != ' ')
				{
					//at this point we know that we have a beta partner
					//now we need to get the right reference to it

					ResidueContainer** residues = ResidueIDMap.Find(currentBridgePartner.number);

					if (residues && *residues)
					{
						//we found the residue container that contains the bridge partner
						betaPartnerResidue = (*residues)->FindBetaPartnerToResidue(currentResidue);
					}
					else
					{
						betaPartnerResidue = nullptr;
					}

					//set the beta partner residue
					currentResidue->SetBetaPartnerResidue(betaPartnerID, betaPartnerResidue);
				}
			}

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