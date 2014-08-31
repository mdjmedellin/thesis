#include "ThesisTest.h"
#include "ProteinModel.h"
#include "ResidueContainer.h"
#include "AminoAcid.h"

namespace GHProtein
{
	ProteinModel::ProteinModel()
		: m_minBounds3D(FVector(0.f,0.f,0.f))
		, m_maxBounds3D(FVector(0.f,0.f,0.f))
		, m_headPtr(nullptr)
	{}

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

	void ProteinModel::SpawnAminoAcids(UWorld* world, UClass* blueprint, const FVector& locationOffset)
	{
		if (!world || !blueprint)
		{
			//we need to have a valid world and blueprint
			return;
		}
		else
		{
			FVector originLocation = FVector::ZeroVector;
			FRotator originRotation = FRotator::ZeroRotator;
			AAminoAcid* previousAminoAcid = nullptr;
			AAminoAcid* currentAminoAcid = nullptr;
			Residue* currentResidue = nullptr;
			FVector aminoAcidLocation = FVector::ZeroVector;

			//iterate over all of the amino acids and spawn an actor for each one of them
			for (int residueIndex = 0; residueIndex < m_residueVector.Num(); ++residueIndex)
			{
				currentResidue = m_residueVector[residueIndex];
				currentResidue->GetCALocation(aminoAcidLocation);
				aminoAcidLocation += originLocation;
				aminoAcidLocation *= 200; // this is done in order to space out the proteins
				currentAminoAcid = UThesisStaticLibrary::SpawnBP<AAminoAcid>(world, blueprint, aminoAcidLocation, originRotation);

				if (previousAminoAcid)
				{
					previousAminoAcid->SetNextAminoAcid(currentAminoAcid);
				}
				previousAminoAcid = currentAminoAcid;

				if (m_headPtr)
				{
					m_minBounds3D.X = m_minBounds3D.X < aminoAcidLocation.X ? m_minBounds3D.X : aminoAcidLocation.X;
					m_minBounds3D.Y = m_minBounds3D.Y < aminoAcidLocation.Y ? m_minBounds3D.Y : aminoAcidLocation.Y;
					m_minBounds3D.Z = m_minBounds3D.Z < aminoAcidLocation.Z ? m_minBounds3D.Z : aminoAcidLocation.Z;

					m_maxBounds3D.X = m_maxBounds3D.X > aminoAcidLocation.X ? m_maxBounds3D.X : aminoAcidLocation.X;
					m_maxBounds3D.Y = m_maxBounds3D.Y > aminoAcidLocation.Y ? m_maxBounds3D.Y : aminoAcidLocation.Y;
					m_maxBounds3D.Z = m_maxBounds3D.Z > aminoAcidLocation.Z ? m_maxBounds3D.Z : aminoAcidLocation.Z;
				}
				else
				{
					m_headPtr = currentAminoAcid;
					m_minBounds3D.Set(aminoAcidLocation.X, aminoAcidLocation.Y, aminoAcidLocation.Z);
					m_maxBounds3D.Set(aminoAcidLocation.X, aminoAcidLocation.Y, aminoAcidLocation.Z);
				}
			}

			//get the center of the bounds
			m_centerOfBoundingBox = (m_minBounds3D * .5f) + (m_maxBounds3D * .5f);

			//we want to bring everything to the center, so subtract the middle from all locations
			currentAminoAcid = m_headPtr;
			while (currentAminoAcid)
			{
				currentAminoAcid->SetActorLocation(currentAminoAcid->GetActorLocation() - m_centerOfBoundingBox);
				currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
			}

			//offset the bounding box
			m_minBounds3D -= m_centerOfBoundingBox;
			m_maxBounds3D -= m_centerOfBoundingBox;
			m_centerOfBoundingBox -= m_centerOfBoundingBox;

			//iterate ove the chain of amino acids and spawn the link particle effect
			currentAminoAcid = m_headPtr;
			while (currentAminoAcid)
			{
				currentAminoAcid->SpawnLinkParticleToNextAminoAcid();
				currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
			}
		}
	}

	void ProteinModel::RotateModel(const FVector& anglesDegrees)
	{
		FVector distanceFromCenter = FVector::ZeroVector;

		//iterate ove the chain of amino acids and rotate them from the model's center point
		AAminoAcid* currentAminoAcid = m_headPtr;
		FRotator rotation(anglesDegrees.X, anglesDegrees.Y, anglesDegrees.Z);
		while (currentAminoAcid)
		{
			currentAminoAcid->RotateFromSpecifiedPoint(m_centerOfBoundingBox, rotation);
			currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
		}
	}
}