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
		ProteinModel(){};

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

		/** Template function */
		template <typename AminoAcidType>
		void FORCEINLINE SpawnAminoAcids(UWorld* world,
									UClass* blueprint,
									const FVector& locationOffset)
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

				AminoAcidType* headPtr = nullptr;
				AminoAcidType* previousAminoAcid = nullptr;
				AminoAcidType* currentAminoAcid = nullptr;
				Residue* currentResidue = nullptr;
				FVector aminoAcidLocation = FVector::ZeroVector;

				//iterate over all of the amino acids and spawn an actor for each one of them
				for (int residueIndex = 0; residueIndex < m_residueVector.Num(); ++residueIndex)
				{
					currentResidue = m_residueVector[residueIndex];
					currentResidue->GetCALocation(aminoAcidLocation);
					aminoAcidLocation += originLocation;
					aminoAcidLocation *= 200;
					currentAminoAcid = UThesisStaticLibrary::SpawnBP<AminoAcidType>(world, blueprint, aminoAcidLocation, originRotation);

					if (previousAminoAcid)
					{
						previousAminoAcid->SetNextAminoAcid(currentAminoAcid);
					}

					previousAminoAcid = currentAminoAcid;

					if (!headPtr)
					{
						headPtr = currentAminoAcid;
					}
				}

				//iterate ove the chain of amino acids and spawn the link particle effect
				currentAminoAcid = headPtr;
				while (currentAminoAcid)
				{
					currentAminoAcid->SpawnLinkParticleToNextAminoAcid();
					currentAminoAcid = currentAminoAcid->GetNextAminoAcidPtr();
				}
			}
		}

	private:
		/** Private utility methods go here */

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