#pragma once

#include "ThesisStaticLibrary.h"

class AAminoAcid;
class ALinkFragment;
class SecondaryStructure;
class Residue;

class BetaSheet;
class HydrogenBond;

/** JM: Protein Model is intended to be the actual structure of the protein */
namespace GHProtein
{
	class ResidueContainer;

	class ProteinModel
	{
	public:
		/** Constructor */
		ProteinModel(UWorld* proteinWorld);

		/** Destructor */
		~ProteinModel();

	private:

		/** No conversion allowed */
		ProteinModel(const ProteinModel& rhs) {};
		/** No assignment allowed */
		ProteinModel& operator=(const ProteinModel& rhs) { return *this; };

		void AppendSecondaryStructure(SecondaryStructure* secondaryStructure);
		void MoveCenterOfModelToSpecifiedLocation(const FVector& proteinModelCenterLocation);
		void UpdateMinAndMaxBounds(const FVector& newPoint);
		
		void AddBetaStrand(SecondaryStructure* newStrand);
		BetaSheet* MergeStrands(SecondaryStructure* newStrand, SecondaryStructure* strandToMergeWith);
		BetaSheet* MergeStrandIntoBetaSheet(SecondaryStructure* newStrand, BetaSheet* betaSheet);
		BetaSheet* MergeBetaSheets(BetaSheet* sheet1, BetaSheet* sheet2);

	public:
		/** Public utility methods go here */
		bool AddResidue(Residue* insertedResidue);
		void BuildProteinModel();
		
		void UpdateRenderProperties(const FColor& normalColor, const FColor& helixColor, const FColor& betaStrandColor,
			const FColor& hydrogenColor, float normalLinkWidth, float normalLinkHeight, float helixLinkWidth, float betaStrandLinkWidth,
			float hydrogenBondLinkWidth, float aminoAcidSize);
		
		void SpawnAminoAcids(UWorld* world, UClass* blueprint, const FVector& aminoAcidCenterLocation, float distanceScale);

		void RotateModel(const FVector& angles);		//x = yaw, y = pitch, z = roll
		void HighlightSecondaryStructure(AAminoAcid* residueMember);
		Residue* GetResidueWithSpecifiedID(int residueNumber, Residue* partnerResidue = nullptr);
		FVector GetDirectionFromCenter(const FVector& currentLocation);
		void TranslateModel(const FVector& displacement);
		FVector GetBoundingBoxDimensions() const;
		FVector GetCenterLocation() const;
		HydrogenBond* SpawnHydrogenBond(AAminoAcid* residue1, AAminoAcid* residue2);
		void ToggleShake();
		void ToggleBreaking();
		void HideHydrogenBonds();
		void BreakFirstSpiral();
		UWorld* GetWorld();

		AAminoAcid* GetAminoAcidWithSpecifiedId(int sequenceNumber);

	public:
		/** public data members go here */

	private:
		/** private data members go here */
		FVector m_minBounds3D;
		FVector m_maxBounds3D;
		FVector m_centerOfBoundingBox;

		AAminoAcid* m_headPtr;
		SecondaryStructure* m_headSecondaryStructure;
		SecondaryStructure* m_tailSecondaryStructure;

		float m_hydrogenBondLinkWidth;
		float m_linkWidth;
		float m_linkHeight;
		float m_helixLinkWidth;
		float m_betaStrandLinkWidth;

		FColor m_normalColor;
		FColor m_helixColor;
		FColor m_betaStrandColor;
		FColor m_hydrogenBondColor;

		float m_temperatureCelsius;

		float m_aminoAcidSize;

		UWorld* m_world;

		TMap<int, ResidueContainer*> ResidueIDMap;
		TArray<Residue*> m_residueVector;
		TArray<ResidueContainer*> m_residueContainers;

		TArray<SecondaryStructure*> m_betaStrands;
		TMap<SecondaryStructure*, BetaSheet*> m_strandToBetaSheetMap;

		TArray<BetaSheet*> m_betaSheets;
		TArray<HydrogenBond*> m_hydrogenBonds;
	};
}