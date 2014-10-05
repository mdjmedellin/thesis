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

public:
	virtual void BeginPlay();

	bool SpawnLinkParticleToNextAminoAcid(float width, float height);

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
	void UpdateHydrogenBonds(bool recurse = false);

	void RotateAminoAcidFromSpecifiedPoint(const FRotationMatrix& rotation, const FVector& rotationPoint);
	void Translate(const FVector& deltaLocation);

	void SetParentModel(GHProtein::ProteinModel* parentModel);
	void SetResidueInformation(Residue* residueInformation);
	void SetSecondaryStructure(ESecondaryStructure::Type secondaryStructure);
	void SetRenderProperties(const FColor& helixColor, const FColor& betaStrandColor, float helixLinkWidth
		, float betaStrandLinkWidth);
	void SetLinkFragmentColor(const FColor& fragmentColor);
	void ResetLinkFragmentColorToDefault();

	ESecondaryStructure::Type GetSecondaryStructure();

	void UpdateLinkFragmentRenderProperties(float helixLinkWidth, float betaStrandLinkWidth);

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

	float m_linkFragmentScalePerUnrealUnit;

	UClass* DefaultLinkFragmentClass;

	ALinkFragment* m_linkFragment;

	ESecondaryStructure::Type m_secondaryStructure;
	FColor m_helixColor;
	FColor m_betaStrandColor;

	Residue* m_residueInformation;

	TArray<HydrogenBond*> m_hydrogenBonds;

	GHProtein::ProteinModel* m_model;

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
