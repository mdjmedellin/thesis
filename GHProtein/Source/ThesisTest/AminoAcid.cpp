// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "AminoAcid.h"
#include "ParticleDefinitions.h"

AAminoAcid::AAminoAcid(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
	, m_nextAminoAcid(nullptr)
	, m_previousAminoAcid(nullptr)
	, m_linkParticleToNextAminoAcid(nullptr)
{

	//Create the root SphereComponent to handle collision
	BaseCollisionComponent = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("BaseSphereComponent"));

	//Set the SphereComponent as the root component
	RootComponent = BaseCollisionComponent;

	//Create the static mesh component
	MeshComponent = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComponent"));

	//turn physics on for the static mesh
	//MeshComponent->SetSimulatePhysics(false);

	//Attach the static mesh component to the root
	MeshComponent->AttachTo(RootComponent);

	BeamParticleTemplate = PCIP.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("BeamParticleTemplate"));
	BeamParticleTemplate->DeactivateSystem();			//no matter what, we do not want the template particle to be active
}

bool AAminoAcid::SpawnLinkParticleToNextAminoAcid()
{
	if (m_linkParticleToNextAminoAcid || !m_nextAminoAcid)
	{
		//we already have spawned the link particle to the next amino acid
		//or we cannot spawn this type of particle because we do not have a valid ptr to the next amino acid on the chain
		return false;
	}
	else
	{
		m_linkParticleToNextAminoAcid = UGameplayStatics::SpawnEmitterAttached(BeamParticleTemplate->Template,
			RootComponent,
			NAME_None,
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false);

		FVector beamSourceLocation = GetActorLocation();
		FVector beamTargetLocation = m_nextAminoAcid->GetActorLocation();
		FVector tangentVector = FVector::ZeroVector;
		tangentVector.X = 1.0;

		//The zero vector for the particle system is the place where the particle system is spawned
		//In this case, the particle system is spawned at the location of the ball
		//Therefore, in order to get the beam to point to the right location, we need to get the displacement
		//vector from this actor's location to the targetActor's location
		m_linkParticleToNextAminoAcid->SetVectorParameter("BeamTargetLocation", (beamTargetLocation - beamSourceLocation));

		//Set the point tangents
		GetDistanceToNextAminoAcid(tangentVector);
		tangentVector.Normalize();
		m_linkParticleToNextAminoAcid->SetVectorParameter("BeamSourceTangent", tangentVector);

		m_nextAminoAcid->GetDistanceToNextAminoAcid(tangentVector);
		tangentVector.Normalize();
		m_linkParticleToNextAminoAcid->SetVectorParameter("BeamTargetTangent", tangentVector);

		return true;
	}
}

bool AAminoAcid::GetDistanceToNextAminoAcid(FVector& out_vector)
{
	if (m_nextAminoAcid)
	{
		out_vector = m_nextAminoAcid->GetActorLocation() - GetActorLocation();
		return true;
	}
	else
	{
		return false;
	}
}

void AAminoAcid::SetNextAminoAcid(AAminoAcid* nextAminoAcid)
{
	if (m_nextAminoAcid)
	{
		//check if we are already pointing to the desired amino acid
		if (m_nextAminoAcid == nextAminoAcid)
		{
			//no change
			return;
		}
		else
		{
			m_nextAminoAcid->ClearPreviousAminoAcidPtr();
			m_nextAminoAcid->SetPreviousAminoAcid(nextAminoAcid);
		}
	}

	m_nextAminoAcid = nextAminoAcid;
	m_nextAminoAcid->SetPreviousAminoAcid(this);
}

void AAminoAcid::SetPreviousAminoAcid(AAminoAcid* previousAminoAcid)
{
	if (m_previousAminoAcid)
	{
		//check if we are alredy pointing to teh desired amino acid
		if (m_previousAminoAcid == previousAminoAcid)
		{
			//produce no change on the pointer chain
			return;
		}
		else
		{
			m_previousAminoAcid->ClearNextAminoAcidPtr();
			m_previousAminoAcid->SetNextAminoAcid(previousAminoAcid);
		}
	}

	m_previousAminoAcid = previousAminoAcid;
	m_previousAminoAcid->SetNextAminoAcid(this);
}

void AAminoAcid::ClearNextAminoAcidPtr()
{
	m_nextAminoAcid = nullptr;
}

void AAminoAcid::ClearPreviousAminoAcidPtr()
{
	m_previousAminoAcid = nullptr;
}

AAminoAcid* AAminoAcid::GetNextAminoAcidPtr()
{
	return m_nextAminoAcid;
}

void AAminoAcid::ReceiveActorOnClicked()
{
	int x = 1;
}
