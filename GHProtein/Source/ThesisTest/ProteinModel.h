#pragma once
#ifndef __Protein_Model_h__
#define __Protein_Model_h__

#include "ThesisStaticLibrary.h"
#include "AminoMovementComponent.h"

class Residue;

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
				originLocation.Z = 200;

				FRotator originRotation = FRotator::ZeroRotator;

				AminoAcidType* currentAminoAcid = nullptr;
				UAminoMovementComponent* movementComponent = nullptr;
				//iterate over all of the amino acids and spawn an actor for each one of them
				for (int residueIndex = 0; residueIndex < m_residueVector.Num(); ++residueIndex)
				{
					originLocation.Z += 20;
					currentAminoAcid = UThesisStaticLibrary::SpawnBP<AminoAcidType>(world, blueprint, originLocation, originRotation);

					//find the movement component and turn on gravity for the meantime
					movementComponent = Cast<UAminoMovementComponent>(currentAminoAcid->FindComponentByClass<UAminoMovementComponent>());

					if (movementComponent != nullptr)
					{
						movementComponent->ToggleGravity();
					}
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