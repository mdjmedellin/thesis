// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "AminoAcid.h"
#include "ThesisStaticLibrary.h"
#include "LinkFragment.h"
#include "Residue.h"

float AAminoAcid::s_tangentTension = 0.0;

AAminoAcid::AAminoAcid(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
	, m_nextAminoAcid(nullptr)
	, m_previousAminoAcid(nullptr)
	, m_linkFragment(nullptr)
	, m_secondaryStructure(ESecondaryStructure::ssCount)
	, m_helixColor(FColor::White)
	, m_betaStrandColor(FColor::White)
{
	//Create the root SphereComponent to handle collision
	BaseCollisionComponent = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("BaseSphereComponent"));

	//Set the SphereComponent as the root component
	RootComponent = BaseCollisionComponent;

	//Create the static mesh component
	MeshComponent = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComponent"));

	//Attach the static mesh component to the root
	MeshComponent->AttachTo(RootComponent);
}

bool AAminoAcid::SpawnLinkParticleToNextAminoAcid(float width, float height)
{
	//we only spawn the link particles if we have an amino acid to connect to
	//and we have not spawned them before
	if (m_nextAminoAcid && (m_linkFragment == nullptr))
	{
		FVector startTangent = FVector::ZeroVector;
		GetTangent(startTangent);
		FVector endTangent = FVector::ZeroVector;
		m_nextAminoAcid->GetTangent(endTangent);

		FVector linkStartLocation = GetActorLocation();
		FVector linkEndLocation = m_nextAminoAcid->GetActorLocation();

		ALinkFragment* linkFragment = nullptr;
		linkFragment = UThesisStaticLibrary::SpawnBP<ALinkFragment>(GetWorld(), DefaultLinkFragmentClass, FVector::ZeroVector, FRotator::ZeroRotator);
		FVector size = linkFragment->SplineMeshComponent->StaticMesh->GetBounds().GetBox().GetSize();
		FVector2D scale(1.f, 1.f);
		scale.X = width / size.X;
		scale.Y = height / size.Y;

		linkFragment->SplineMeshComponent->SetStartScale(scale);
		linkFragment->SplineMeshComponent->SetEndScale(scale);

		linkFragment->SplineMeshComponent->SetStartAndEnd(linkStartLocation, startTangent, linkEndLocation, endTangent);
		m_linkFragment = linkFragment;

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

void AAminoAcid::SetTangentTension(float newTension)
{
	s_tangentTension = newTension;
}

void AAminoAcid::SetAminoAcidSize(float aminoAcidSize)
{
	FVector dimensions = GetComponentsBoundingBox().GetSize();
	if (dimensions.X > 0.f && dimensions.Y > 0.f && dimensions.Z > 0.f)
	{
		FVector scale = FVector::ZeroVector;
		scale.X = aminoAcidSize / dimensions.X;
		scale.Y = aminoAcidSize / dimensions.Y;
		scale.Z = aminoAcidSize / dimensions.Z;
		this->SetActorScale3D(scale);
	}
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
	
	//create the dynamic material
	m_dynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
}

void AAminoAcid::UpdateLinkToNextAminoAcid()
{
	if (m_nextAminoAcid && m_linkFragment)
	{
		FVector startTangent = FVector::ZeroVector;
		GetTangent(startTangent);
		FVector endTangent = FVector::ZeroVector;
		m_nextAminoAcid->GetTangent(endTangent);

		FVector linkStartLocation = GetActorLocation();
		FVector linkEndLocation = m_nextAminoAcid->GetActorLocation();

		m_linkFragment->SplineMeshComponent->SetStartAndEnd(linkStartLocation, startTangent, linkEndLocation, endTangent);
	}
}

void AAminoAcid::Translate(const FVector& deltaLocation)
{
	SetActorLocation(GetActorLocation() + deltaLocation);

	//update the chains of the amino acids
	if (m_previousAminoAcid)
	{
		m_previousAminoAcid->UpdateLinkToNextAminoAcid();
	}

	UpdateLinkToNextAminoAcid();
}

void AAminoAcid::RotateAminoAcidFromSpecifiedPoint(const FVector& rotationPoint, const FRotator& rotation)
{
	//rotate the amino acid and the chain
	FVector distanceFromRotationPoint = GetActorLocation() - rotationPoint;
	distanceFromRotationPoint = rotation.RotateVector(distanceFromRotationPoint);

	SetActorLocation(distanceFromRotationPoint + rotationPoint);
}

void AAminoAcid::SetSecondaryStructure(ESecondaryStructure::Type secondaryStructure)
{
	m_secondaryStructure = secondaryStructure;
}

void AAminoAcid::SetRenderProperties(const FColor& helixColor, const FColor& betaStrandColor, float helixLinkWidth
	, float betaStrandLinkWidth)
{
	m_helixColor = helixColor;
	m_betaStrandColor = betaStrandColor;

	UpdateLinkFragmentRenderProperties(helixLinkWidth, betaStrandLinkWidth);
}

void AAminoAcid::UpdateLinkFragmentRenderProperties(float helixLinkWidth, float betaStrandLinkWidth)
{
	if (m_linkFragment)
	{
		FVector size = m_linkFragment->SplineMeshComponent->StaticMesh->GetBounds().GetBox().GetSize();
		FVector2D scale = m_linkFragment->SplineMeshComponent->GetStartScale();
		FColor renderColor = FColor::White;

		switch (m_secondaryStructure)
		{
		case ESecondaryStructure::ssAlphaHelix:
			renderColor = m_helixColor;
			scale.Y = helixLinkWidth / size.Y;
			break;
		case ESecondaryStructure::ssStrand:
			renderColor = m_betaStrandColor;
			scale.Y = betaStrandLinkWidth / size.Y;
			break;
		default:
			break;
		}
		
		m_linkFragment->SplineMeshComponent->SetStartScale(scale);
		m_linkFragment->SplineMeshComponent->SetEndScale(scale);
		m_linkFragment->setColor(renderColor);
	}
}