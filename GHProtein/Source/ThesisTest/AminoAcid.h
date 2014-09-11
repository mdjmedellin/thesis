// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "AminoAcid.generated.h"

/**
 * 
 */
UENUM()
namespace EAminoAcidType
{
	enum Type
	{
		Alanine
		, Cysteine
		, AsparticAcid
		, GlutamicAcid
		, Phenylalanine
		, Glycine
		, Histidine
		, Isoleucine
		, Lysine
		, Leucine
		, Methionine
		, Asparagine
		, Proline
		, Glutamine
		, Arginine
		, Serine
		, Threonine
		, Valine
		, Tryptophan
		, Tyrosine
		, Count						UMETA(Hidden)
	};
}

namespace ESecondaryStructure
{
	enum Type;
}

class ALinkFragment;
class Residue;

namespace GHProtein
{
	class ProteinModel;
}

UCLASS()
class AAminoAcid : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	void ClearNextAminoAcidPtr();
	void ClearPreviousAminoAcidPtr();

public:

	virtual void BeginPlay();

	bool SpawnLinkParticleToNextAminoAcid(float width, float height);

	void SetNextAminoAcid(AAminoAcid* nextAminoAcid);
	void SetPreviousAminoAcid(AAminoAcid* previousAminoAcid);

	int GetSequenceNumber();
	bool GetDistanceToNextAminoAcid(FVector& out_vector);
	void GetTangent(FVector& out_vector);

	AAminoAcid* GetNextAminoAcidPtr();

	void UpdateLinkToNextAminoAcid();
	void UpdateHydrogenBonds(bool recurse = false);

	void RotateAminoAcidFromSpecifiedPoint(const FVector& rotationPoint, const FRotator& rotation);
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

	//static functions
	static void SetTangentTension(float newTension);

private:
	//private data members
	AAminoAcid* m_nextAminoAcid;
	AAminoAcid* m_previousAminoAcid;

	UMaterialInstanceDynamic* m_dynamicMaterial;

	float m_linkFragmentScalePerUnrealUnit;

	UClass* DefaultLinkFragmentClass;

	ALinkFragment* m_linkFragment;
	ALinkFragment* m_betaPartner1;
	ALinkFragment* m_betaPartner2;

	AAminoAcid* m_betaPartnerResidue1;
	AAminoAcid* m_betaPartnerResidue2;

	ESecondaryStructure::Type m_secondaryStructure;
	FColor m_helixColor;
	FColor m_betaStrandColor;

	Residue* m_residueInformation;

	GHProtein::ProteinModel* m_model;

	//static data members
	static float s_tangentTension;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AminoAcidInterface)
		TEnumAsByte<EAminoAcidType::Type> m_typeOfAminoAcid;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		float m_lengthOfLinkFragment;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		TSubobjectPtr<USphereComponent> BaseCollisionComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		TSubobjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		UBlueprint* m_linkBlueprint;
};
