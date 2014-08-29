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

class ALinkFragment;

UCLASS()
class AAminoAcid : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	void ClearNextAminoAcidPtr();
	void ClearPreviousAminoAcidPtr();
	//testing the clickable interface
	virtual void ReceiveActorOnClicked();

public:

	virtual void BeginPlay();

	bool SpawnLinkParticleToNextAminoAcid();

	void SetNextAminoAcid(AAminoAcid* nextAminoAcid);
	void SetPreviousAminoAcid(AAminoAcid* previousAminoAcid);

	bool GetDistanceToNextAminoAcid(FVector& out_vector);
	void GetTangent(FVector& out_vector);

	AAminoAcid* GetNextAminoAcidPtr();

	void UpdateLinkToNextAminoAcid();

	//static functions
	static void SetTangentTension(float newTension);

private:
	//private data members

	UParticleSystemComponent* m_linkParticleToNextAminoAcid;

	AAminoAcid* m_nextAminoAcid;
	AAminoAcid* m_previousAminoAcid;

	//static data members
	static float s_tangentTension;

	float m_linkFragmentScalePerUnrealUnit;

	UClass* DefaultLinkFragmentClass;

	TArray<ALinkFragment*> m_linkFragments;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AminoAcidInterface)
		TEnumAsByte<EAminoAcidType::Type> m_typeOfAminoAcid;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		float m_lengthOfLinkFragment;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		TSubobjectPtr<USphereComponent> BaseCollisionComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		TSubobjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Particles)
		TSubobjectPtr<UParticleSystemComponent> BeamParticleTemplate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		UBlueprint* m_linkBlueprint;
};
