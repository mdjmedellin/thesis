#pragma once
#ifndef __Protein_Model_h__
#define __Protein_Model_h__

#include "ThesisStaticLibrary.h"
#include "AminoMovementComponent.h"
#include "Residue.h"

class AAminoAcid;

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
		void SpawnAminoAcids(UWorld* world, UClass* blueprint, float aminoAcidSize, const FVector& aminoAcidCenterLocation
			, float linkWidth, float linkHeight, float distanceScale);
		void RotateModel(const FVector& angles);		//x = yaw, y = pitch, z = roll

	private:
		/** Private utility methods go here */
		FVector m_minBounds3D;
		FVector m_maxBounds3D;
		FVector m_centerOfBoundingBox;
		AAminoAcid* m_headPtr;

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