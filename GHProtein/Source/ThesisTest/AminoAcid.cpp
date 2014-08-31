// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "AminoAcid.h"
#include "ThesisStaticLibrary.h"
#include "LinkFragment.h"
#include "ParticleDefinitions.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"

float AAminoAcid::s_tangentTension = 0.0;

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

	//Attach the static mesh component to the root
	MeshComponent->AttachTo(RootComponent);

	BeamParticleTemplate = PCIP.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("BeamParticleTemplate"));
	BeamParticleTemplate->DeactivateSystem();			//no matter what, we do not want the template particle to be active
}

bool AAminoAcid::SpawnLinkParticleToNextAminoAcid()
{
	//we only spawn the link particles if we have an amino acid to connect to
	//and we have not spawned them before
	if (m_nextAminoAcid && m_linkFragments.Num() == 0)
	{
		int numberOfMeshes = 20;
		float s = 0.f;
		float t = 0.f;
		float t_squared = 0.f;
		float s_squared = 0.f;
		float step = 1.0 / numberOfMeshes;

		FVector linkScale3D(1.f, 1.f, 1.f);
		FVector startTangent = FVector::ZeroVector;
		GetTangent(startTangent);
		FVector endTangent = FVector::ZeroVector;
		m_nextAminoAcid->GetTangent(endTangent);

		FVector linkStartLocation = GetActorLocation();
		FVector linkEndLocation = m_nextAminoAcid->GetActorLocation();

		FVector A = FVector::ZeroVector;
		FVector D = FVector::ZeroVector;
		FVector U = FVector::ZeroVector;
		FVector V = FVector::ZeroVector;

		FVector startLocation = FVector::ZeroVector;
		FVector endLocation = FVector::ZeroVector;

		FVector linkFragmentDirection = FVector::ZeroVector;
		float linkFragmentLength = 0.f;

		ALinkFragment* linkFragment = nullptr;

		for (int i = 0; i < numberOfMeshes; ++i)
		{
			//calculate the start and end position of the link fragment
			if (i != 0)
			{
				startLocation = endLocation;
			}
			else
			{
				s = (1.f - t);
				t_squared = t * t;
				s_squared = s * s;

				A = s_squared * (1 + (2 * t)) * linkStartLocation;
				D = t_squared * (1 + (2 * s)) * linkEndLocation;
				U = (s_squared * t) * startTangent;
				V = (t_squared * s) * endTangent;

				startLocation = A + D + U - V;
			}

			//calculate the end location
			t += step;
			s = (1.f - t);
			t_squared = t * t;
			s_squared = s * s;

			A = s_squared * (1 + (2 * t)) * linkStartLocation;
			D = t_squared * (1 + (2 * s)) * linkEndLocation;
			U = (s_squared * t) * startTangent;
			V = (t_squared * s) * endTangent;

			endLocation = A + D + U - V;

			//calculate the size
			(endLocation - startLocation).ToDirectionAndLength(linkFragmentDirection, linkFragmentLength);
			linkScale3D.Z = linkFragmentLength * m_linkFragmentScalePerUnrealUnit;
			FRotator linkFragmentRotation = linkFragmentDirection.Rotation();
			linkFragmentRotation.Pitch += 90;

			linkFragment = UThesisStaticLibrary::SpawnBP<ALinkFragment>(GetWorld(), DefaultLinkFragmentClass, startLocation, linkFragmentRotation);
			linkFragment->SetActorScale3D(linkScale3D);

			m_linkFragments.Add(linkFragment);
		}

		return true;
	}

	return false;
}

void AAminoAcid::GetTangent(FVector& out_vector)
{
	//we use cardinal splines to calculate the tangent
	//Since cardinal splines need the previous and next point
	//if this amino acid is the head or the tail of the chain, then
	//we set the out_vector to zero
	if (!m_previousAminoAcid || !m_nextAminoAcid)
	{
		out_vector = FVector::ZeroVector;
	}
	else
	{
		out_vector = m_nextAminoAcid->GetActorLocation() - m_previousAminoAcid->GetActorLocation();
		out_vector *= .5f;
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

void AAminoAcid::SetTangentTension(float newTension)
{
	s_tangentTension = newTension;
}

void AAminoAcid::BeginPlay()
{
	if (m_linkBlueprint)
	{
		DefaultLinkFragmentClass = (UClass*)m_linkBlueprint->GeneratedClass;
	}
	else
	{
		DefaultLinkFragmentClass = nullptr;
	}

	m_linkFragmentScalePerUnrealUnit = 1.0 / m_lengthOfLinkFragment;
}

void AAminoAcid::UpdateLinkToNextAminoAcid()
{
	if (m_nextAminoAcid)
	{
		int numberOfMeshes = m_linkFragments.Num();
		float s = 0.f;
		float t = 0.f;
		float t_squared = 0.f;
		float s_squared = 0.f;
		float step = 1.0 / numberOfMeshes;

		FVector scale3D(1.f, 1.f, 1.f);

		FVector startTangent = FVector::ZeroVector;
		GetTangent(startTangent);

		FVector endTangent = FVector::ZeroVector;
		m_nextAminoAcid->GetTangent(endTangent);

		FVector linkStartLocation = GetActorLocation();
		FVector linkEndLocation = m_nextAminoAcid->GetActorLocation();

		FVector A = FVector::ZeroVector;
		FVector D = FVector::ZeroVector;
		FVector U = FVector::ZeroVector;
		FVector V = FVector::ZeroVector;

		FVector startLocation = FVector::ZeroVector;
		FVector endLocation = FVector::ZeroVector;

		FVector linkFragmentDirection = FVector::ZeroVector;
		float linkFragmentLength = 0.f;

		ALinkFragment* linkFragment = nullptr;

		for (int i = 0; i < numberOfMeshes; ++i)
		{
			//calculate the start and end position of the link fragment
			if (i != 0)
			{
				startLocation = endLocation;
			}
			else
			{
				s = (1.f - t);
				t_squared = t * t;
				s_squared = s * s;

				A = s_squared * (1 + (2 * t)) * linkStartLocation;
				D = t_squared * (1 + (2 * s)) * linkEndLocation;
				U = (s_squared * t) * startTangent;
				V = (t_squared * s) * endTangent;

				startLocation = A + D + U - V;
			}

			//calculate the end location
			t += step;
			s = (1.f - t);
			t_squared = t * t;
			s_squared = s * s;

			A = s_squared * (1 + (2 * t)) * linkStartLocation;
			D = t_squared * (1 + (2 * s)) * linkEndLocation;
			U = (s_squared * t) * startTangent;
			V = (t_squared * s) * endTangent;

			endLocation = A + D + U - V;

			//calculate the size
			(endLocation - startLocation).ToDirectionAndLength(linkFragmentDirection, linkFragmentLength);
			scale3D.Z = linkFragmentLength * m_linkFragmentScalePerUnrealUnit;
			FRotator linkFragmentRotation = linkFragmentDirection.Rotation();
			linkFragmentRotation.Pitch += 90;

			linkFragment = m_linkFragments[i];
			linkFragment->SetActorLocationAndRotation(startLocation, linkFragmentRotation);
			linkFragment->SetActorScale3D(scale3D);
		}
	}
}

void AAminoAcid::RotateFromSpecifiedPoint(const FVector& rotationPoint, const FRotator& rotation)
{
	//rotate the amino acid and the chain
	FVector distanceFromRotationPoint = GetActorLocation() - rotationPoint;
	distanceFromRotationPoint = rotation.RotateVector(distanceFromRotationPoint);

	SetActorLocation(rotationPoint + distanceFromRotationPoint);

	FVector rotationVector = FVector::ZeroVector;
	FVector prevLocation = FVector::ZeroVector;
	ALinkFragment* prevLinkFragment = nullptr;
	ALinkFragment* linkFragment = nullptr;
	for (int i = 0; i < m_linkFragments.Num(); ++i)
	{
		linkFragment = m_linkFragments[i];
		distanceFromRotationPoint = rotation.RotateVector(linkFragment->GetActorLocation() - rotationPoint);
		distanceFromRotationPoint += rotationPoint;
		linkFragment->SetActorLocation(distanceFromRotationPoint);

		if (prevLinkFragment)
		{
			rotationVector = distanceFromRotationPoint - prevLocation;
			//set rotation for the previous link fragment
			prevLinkFragment->SetActorRotation(rotationVector.Rotation().Add(90,0,0));
		}

		prevLocation = distanceFromRotationPoint;
		prevLinkFragment = linkFragment;
	}

	if (prevLinkFragment)
	{
		prevLinkFragment->SetActorRotation(rotationVector.Rotation().Add(90, 0, 0));
	}
}