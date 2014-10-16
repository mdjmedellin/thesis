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

		void UpdateModelTemperature();

		void AppendSecondaryStructure(SecondaryStructure* secondaryStructure);
		void MoveCenterOfModelToSpecifiedLocation(const FVector& proteinModelCenterLocation);
		void UpdateMinAndMaxBounds(const FVector& newPoint);
		
		void AddBetaStrand(SecondaryStructure* newStrand);
		BetaSheet* MergeStrands(SecondaryStructure* newStrand, SecondaryStructure* strandToMergeWith);
		BetaSheet* MergeStrandIntoBetaSheet(SecondaryStructure* newStrand, BetaSheet* betaSheet);
		BetaSheet* MergeBetaSheets(BetaSheet* sheet1, BetaSheet* sheet2);

		void CheckIfEndModificationEventShouldTrigger();

	public:
		/** Public utility methods go here */
		bool AddResidue(Residue* insertedResidue);
		void BuildProteinModel();
		
		void UpdateRenderProperties(const FColor& normalColor, const FColor& helixColor, const FColor& betaStrandColor,
			const FColor& hydrogenColor, float normalLinkWidth, float normalLinkHeight, float helixLinkWidth, float betaStrandLinkWidth,
			float hydrogenBondLinkWidth, float aminoAcidSize);

		void SetEnviromentalProperties(float startingTemperatureCelsius, float stableTemperatureCelsius, float meltingTemperatureCelsius,
			float irreversibleTemperatureCelsius, float temperatureStep);
		
		void SpawnAminoAcids(UWorld* world, UClass* blueprint, const FVector& aminoAcidCenterLocation, float distanceScale);

		void RotateModel(const FVector& angles);		//x = yaw, y = pitch, z = roll
		void TranslateModel(const FVector& displacement);

		Residue* GetResidueWithSpecifiedID(int residueNumber, Residue* partnerResidue = nullptr);
		FVector GetDirectionFromCenter(const FVector& currentLocation);
		FVector GetBoundingBoxDimensions() const;
		FVector GetCenterLocation() const;
		HydrogenBond* SpawnHydrogenBond(AAminoAcid* residue1, AAminoAcid* residue2);
		void ToggleShake();
		void ToggleBreaking();
		void HideHydrogenBonds();
		void SetTemperature(float temperatureCelsius);
		void ModifyTemperature(float temperatureModifierScale);
		float GetCurrentTemperature();
		UWorld* GetWorld();

		void AddToListOfModifiedSecondaryStructures(SecondaryStructure* secondaryStructureBeingModified);
		void RemoveFromListOfModifiedSecondaryStructures(SecondaryStructure* secondaryStructureToRemove);

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
		float m_temperatureStep;
		float m_meltingTemperatureCelsius;
		float m_stableTemperatureCelsius;
		float m_irreversibleTemperatureCelsius;

		float m_aminoAcidSize;

		UWorld* m_world;

		TMap<int, ResidueContainer*> ResidueIDMap;
		TArray<Residue*> m_residueVector;
		TArray<ResidueContainer*> m_residueContainers;

		TArray<SecondaryStructure*> m_modifiedSecondaryStructures;
		TArray<HydrogenBond*> m_modifiedHydrogenBonds;

		TArray<SecondaryStructure*> m_betaStrands;
		TMap<SecondaryStructure*, BetaSheet*> m_strandToBetaSheetMap;

		TArray<BetaSheet*> m_betaSheets;
		TArray<HydrogenBond*> m_hydrogenBonds;
	};
}