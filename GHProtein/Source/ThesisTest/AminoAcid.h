// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/Actor.h"
#include "ThesisStaticLibrary.h"
#include "Residue.h"
#include "AminoAcid.generated.h"

namespace GHProtein
{
	class ProteinModel;
}

class ALinkFragment;
class HydrogenBond;

UCLASS()
class AAminoAcid : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	void ClearNextAminoAcidPtr();
	void ClearPreviousAminoAcidPtr();
	void RotateLinkFragmentAboutSpecifiedPoint(const FRotationMatrix& rotation, const FVector& rotationPoint);
	void TranslateLinkFragment(const FVector& deltaLocation);
	void UpdateLinkFragmentRenderProperties();

public:
	virtual void BeginPlay();

	bool SpawnLinkParticleToNextAminoAcid();

	void SetNextAminoAcid(AAminoAcid* nextAminoAcid);
	void SetPreviousAminoAcid(AAminoAcid* previousAminoAcid);

	void HideLinkFragment();

	int GetSequenceNumber();
	bool GetDistanceToNextAminoAcid(FVector& out_vector);
	void GetTangent(FVector& out_vector);

	AAminoAcid* GetNextAminoAcidPtr();
	AAminoAcid* GetPreviousAminoAcidPtr();

	const Residue* GetResidueInformation() const;

	void UpdateLinkToNextAminoAcid();

	void RotateAminoAcidFromSpecifiedPoint(const FRotationMatrix& rotation, const FVector& rotationPoint);
	void Translate(const FVector& deltaLocation);

	void SetParentModel(GHProtein::ProteinModel* parentModel);
	void SetResidueInformation(Residue* residueInformation);
	void SetSecondaryStructure(ESecondaryStructure::Type secondaryStructure);

	void SetRenderProperties(const FColor& normalColor, const FColor& helixColor, const FColor& betaStrandColor,
		const FColor& hydrogenBondColor, float normalWidth, float helixLinkWidth, float betaStrandLinkWidth, 
		float hydrogenBondLinkWidth, float linkHeight);
	/*
	void SetLinkFragmentColor(const FColor& fragmentColor);
	void ResetLinkFragmentColorToDefault();
	*/

	ESecondaryStructure::Type GetSecondaryStructure();

	void SetAminoAcidSize(float aminoAcidSize);

	bool BondWithResidueExists(const AAminoAcid* residue) const;
	void AddHydrogenBond(HydrogenBond* newBond);

	UClass* GetDetaultLinkFragmentClass();
	ResidueInfo GetAminoAcidInfo() const;

	UFUNCTION(BlueprintCallable, Category = Residue)
		FString GetResidueTypeString() const;

	UFUNCTION(BlueprintCallable, Category = Residue)
		void SetAminoAcidType(TEnumAsByte<EResidueType::Type> aminoAcidType);

private:
	//private data members
	AAminoAcid* m_nextAminoAcid;
	AAminoAcid* m_previousAminoAcid;

	UMaterialInstanceDynamic* m_dynamicMaterial;
	UClass* m_defaultLinkFragmentClass;
	ALinkFragment* m_linkFragment;
	Residue* m_residueInformation;
	GHProtein::ProteinModel* m_model;

	ESecondaryStructure::Type m_secondaryStructure;
	FColor m_normalColor;
	FColor m_helixColor;
	FColor m_betaStrandColor;
	FColor m_hydrogenBondColor;
	float m_normalHeight;
	float m_normalWidth;
	float m_helixWidth;
	float m_betaStrandWidth;
	float m_hydrogenBondLinkWidth;
	float m_linkFragmentScalePerUnrealUnit;

	TArray<HydrogenBond*> m_hydrogenBonds;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		float m_lengthOfLinkFragment;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		TSubobjectPtr<USphereComponent> BaseCollisionComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		TSubobjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AminoAcidInterface)
		TSubobjectPtr<UTextRenderComponent> TextRenderComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		UBlueprint* m_linkBlueprint;
};
