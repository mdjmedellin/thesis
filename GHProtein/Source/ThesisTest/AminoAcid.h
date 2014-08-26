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

UCLASS()
class AAminoAcid : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	/* constructor */
	AAminoAcid();

	//testing the clickable interface
	virtual void ReceiveActorOnClicked();

public:
	void SpawnBeamParticle(AAminoAcid* target);

private:
	//private data members
	TArray<TSubobjectPtr<UParticleSystemComponent*>> ParticleComponentArray;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AminoAcidInterface)
		TEnumAsByte<EAminoAcidType::Type> m_typeOfAminoAcid;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		TSubobjectPtr<USphereComponent> BaseCollisionComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		TSubobjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Particles)
		TSubobjectPtr<UParticleSystemComponent> BeamParticleTemplate;
};
