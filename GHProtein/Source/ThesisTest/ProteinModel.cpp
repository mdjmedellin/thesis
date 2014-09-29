#include "ThesisTest.h"
#include "ProteinUtilities.h"
#include "ProteinModel.h"
#include "ResidueContainer.h"
#include "AminoAcid.h"
#include "SecondaryStructure.h"

namespace GHProtein
{
	ProteinModel::ProteinModel()
		: m_minBounds3D(FVector(0.f, 0.f, 0.f))
		, m_maxBounds3D(FVector(0.f, 0.f, 0.f))
		, m_headPtr(nullptr)
		, m_headSecondaryStructure(nullptr)
		, m_tailSecondaryStructure(nullptr)
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

	Residue* ProteinModel::GetResidueWithSpecifiedID(int residueNumber, Residue* partnerResidue)
	{
		ResidueContainer** residues = ResidueIDMap.Find(residueNumber);
		Residue* foundResidue = nullptr;

		if (residues && *residues)
		{
			//we have found the appropriate residue container
			//if we have a partner residue specified, then we return the residue whose partner is the specified residue
			if (partnerResidue)
			{
				foundResidue = (*residues)->FindBetaPartnerToResidue(partnerResidue);
			}
			else
			{
				foundResidue = (*residues)->GetFirstResidue();
			}
		}

		return foundResidue;
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
					betaPartnerResidue = GetResidueWithSpecifiedID(currentBridgePartner.number, currentResidue);
					
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

	void ProteinModel::UpdateMinAndMaxBounds(const FVector& newPoint)
	{
		m_minBounds3D.X = GetMin(m_minBounds3D.X, newPoint.X);
		m_minBounds3D.Y = GetMin(m_minBounds3D.Y, newPoint.Y);
		m_minBounds3D.Z = GetMin(m_minBounds3D.Z, newPoint.Z);

		m_maxBounds3D.X = GetMax(m_maxBounds3D.X, newPoint.X);
		m_maxBounds3D.Y = GetMax(m_maxBounds3D.Y, newPoint.Y);
		m_maxBounds3D.Z = GetMax(m_maxBounds3D.Z, newPoint.Z);

		//get the center of the bounds
		m_centerOfBoundingBox = (m_minBounds3D * .5f) + (m_maxBounds3D * .5f);
	}

	void ProteinModel::MoveCenterOfModelToSpecifiedLocation(const FVector& proteinModelCenterLocation)
	{
		//translate all amino acids
		AAminoAcid* currentAminoAcid = m_headPtr;
		FVector newResidueLocation = FVector::ZeroVector;
		while (currentAminoAcid)
		{
			newResidueLocation = currentAminoAcid->GetActorLocation() - m_centerOfBoundingBox;		//this brings the residue centered at 0,0,0
			newResidueLocation += proteinModelCenterLocation;										//this centers the residue about the new location

			currentAminoAcid->SetActorLocation(newResidueLocation);
			currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
		}

		//offset the bounding box
		m_minBounds3D = m_minBounds3D - m_centerOfBoundingBox + proteinModelCenterLocation;
		m_maxBounds3D = m_maxBounds3D - m_centerOfBoundingBox + proteinModelCenterLocation;
		m_centerOfBoundingBox = proteinModelCenterLocation;
	}

	void ProteinModel::SpawnAminoAcids(UWorld* world, UClass* blueprint, float aminoAcidSize, const FVector& proteinModelCenterLocation
		, float linkWidth, float linkHeight, float distanceScale, const FColor& helixColor, const FColor& betaStrandColor, float helixLinkWidth
		, float betaStrandLinkWidth)
	{
		if (!world || !blueprint || aminoAcidSize <= 0.f)
		{
			//we need to have a valid world and blueprint
			//we can only spawn amino acids with a positive non-zero size
			return;
		}
		else
		{
			FRotator originRotation = FRotator::ZeroRotator;
			AAminoAcid* previousAminoAcid = nullptr;
			AAminoAcid* currentAminoAcid = nullptr;
			Residue* currentResidue = nullptr;
			FVector aminoAcidLocation = FVector::ZeroVector;
			ESecondaryStructure::Type currentSecondaryStructureType = ESecondaryStructure::ssCount;
			SecondaryStructure* currentSecondaryStructure = nullptr;

			//iterate over all of the amino acids and spawn an actor for each one of them
			for (int residueIndex = 0; residueIndex < m_residueVector.Num(); ++residueIndex)
			{
				currentResidue = m_residueVector[residueIndex];
				currentResidue->GetCALocation(aminoAcidLocation);
				aminoAcidLocation *= distanceScale; // this is done in order to space out the proteins

				currentAminoAcid = UThesisStaticLibrary::SpawnBP<AAminoAcid>(world, blueprint, aminoAcidLocation, originRotation);
				currentAminoAcid->SetAminoAcidSize(aminoAcidSize);
				currentAminoAcid->SetResidueInformation(currentResidue);
				currentAminoAcid->SetParentModel(this);

				if (previousAminoAcid)
				{
					previousAminoAcid->SetNextAminoAcid(currentAminoAcid);
				}
				previousAminoAcid = currentAminoAcid;

				//update bounding box
				if (m_headPtr)
				{
					UpdateMinAndMaxBounds(aminoAcidLocation);
				}
				else
				{
					m_headPtr = currentAminoAcid;
					m_minBounds3D.Set(aminoAcidLocation.X, aminoAcidLocation.Y, aminoAcidLocation.Z);
					m_maxBounds3D.Set(aminoAcidLocation.X, aminoAcidLocation.Y, aminoAcidLocation.Z);
				}

				//set the residues secondary structure and updates the current secondary structure
				currentAminoAcid->SetSecondaryStructure(currentResidue->GetSecondaryStructure());

				if (currentSecondaryStructureType != currentAminoAcid->GetSecondaryStructure())
				{
					currentSecondaryStructureType = currentAminoAcid->GetSecondaryStructure();
					currentSecondaryStructure = new SecondaryStructure(currentSecondaryStructureType);

					if (currentSecondaryStructure)
					{
						AppendSecondaryStructure(currentSecondaryStructure);
					}
				}

				currentSecondaryStructure->AppendAminoAcid(currentAminoAcid);
			}

			AppendSecondaryStructure(currentSecondaryStructure);
			//CreateBetaSheets();

			//we want to bring everything to the center, so subtract the middle of the bounding box from all locations
			MoveCenterOfModelToSpecifiedLocation(proteinModelCenterLocation);

			//iterate ove the chain of amino acids and spawn the link particle effect
			currentAminoAcid = m_headPtr;
			while (currentAminoAcid)
			{
				currentAminoAcid->SpawnLinkParticleToNextAminoAcid(linkWidth, linkHeight);
				currentAminoAcid->SetRenderProperties(helixColor, betaStrandColor, helixLinkWidth, betaStrandLinkWidth);
				currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
			}
		}
	}

	void ProteinModel::CreateBetaSheets()
	{
	}

	void ProteinModel::HighlightSecondaryStructure(AAminoAcid* residueMember)
	{
		//iterate through the secondary structures to see what to highlight
		for (SecondaryStructure* currentSecondaryStructure = m_headSecondaryStructure;
			currentSecondaryStructure != nullptr && (m_tailSecondaryStructure && currentSecondaryStructure != m_tailSecondaryStructure->GetNextStructurePtr());
			currentSecondaryStructure = currentSecondaryStructure->GetNextStructurePtr())
		{
			if (currentSecondaryStructure->ContainsSpecifiedResidue(residueMember))
			{
				currentSecondaryStructure->SetSelected();
				break;
			}
		}
	}

	AAminoAcid* ProteinModel::GetAminoAcidWithSpecifiedId(int sequenceNumber)
	{
		//search through all of the secondary structures and grab the residue with the specified sequence number
		SecondaryStructure* currentSecondaryStructure = m_headSecondaryStructure;
		AAminoAcid* foundResidue = nullptr;

		while (currentSecondaryStructure)
		{
			foundResidue = currentSecondaryStructure->GetAminoAcidWithSpecifiedId(sequenceNumber);

			if (foundResidue)
			{
				break;
			}
			else
			{
				currentSecondaryStructure = currentSecondaryStructure->GetNextStructurePtr();
			}
		}

		return foundResidue;
	}

	void ProteinModel::AppendSecondaryStructure(SecondaryStructure* secondaryStructure)
	{
		if (!secondaryStructure)
		{
			return;
		}

		if (m_tailSecondaryStructure)
		{
			m_tailSecondaryStructure->SetNextStructurePtr(secondaryStructure);
			m_tailSecondaryStructure = secondaryStructure;
		}
		else
		{
			m_headSecondaryStructure = secondaryStructure;
			m_tailSecondaryStructure = m_headSecondaryStructure;
		}
	}

	void ProteinModel::RotateModel(const FVector& anglesDegrees)
	{
		FVector distanceFromCenter = FVector::ZeroVector;

		//iterate ove the chain of amino acids and rotate them from the model's center point
		AAminoAcid* currentAminoAcid = m_headPtr;
		FRotator rotation(anglesDegrees.X, anglesDegrees.Y, anglesDegrees.Z);
		
		//update position of the amino acids
		while (currentAminoAcid)
		{
			currentAminoAcid->RotateAminoAcidFromSpecifiedPoint(m_centerOfBoundingBox, rotation);
			currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
		}

		//update links
		currentAminoAcid = m_headPtr;
		FVector aminoAcidLocation;
		while (currentAminoAcid)
		{
			currentAminoAcid->UpdateLinkToNextAminoAcid();
			aminoAcidLocation = currentAminoAcid->GetActorLocation();

			if (currentAminoAcid != m_headPtr)
			{
				UpdateMinAndMaxBounds(aminoAcidLocation);
			}
			else
			{
				m_minBounds3D.Set(aminoAcidLocation.X, aminoAcidLocation.Y, aminoAcidLocation.Z);
				m_maxBounds3D.Set(aminoAcidLocation.X, aminoAcidLocation.Y, aminoAcidLocation.Z);
			}

			currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
		}
	}

	FVector ProteinModel::GetDirectionFromCenter(const FVector& currentLocation)
	{
		FVector returnVector = currentLocation;
		returnVector -= m_centerOfBoundingBox;
		return returnVector;
	}

	void ProteinModel::TranslateModel(const FVector& displacement)
	{
		FVector distanceFromCenter = FVector::ZeroVector;

		//iterate ove the chain of amino acids and rotate them from the model's center point
		AAminoAcid* currentAminoAcid = m_headPtr;

		//update position of the amino acids
		while (currentAminoAcid)
		{
			currentAminoAcid->Translate(displacement);
			currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
		}

		//update links
		currentAminoAcid = m_headPtr;
		FVector aminoAcidLocation;
		while (currentAminoAcid)
		{
			currentAminoAcid->UpdateLinkToNextAminoAcid();
			currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
		}

		//translate the bounding box min and max
		m_minBounds3D += displacement;
		m_maxBounds3D += displacement;
		m_centerOfBoundingBox += displacement;
	}

	FVector ProteinModel::GetBoundingBoxDimensions() const
	{
		return m_maxBounds3D - m_minBounds3D;
	}

	FVector ProteinModel::GetCenterLocation() const
	{
		return m_centerOfBoundingBox;
	}
}