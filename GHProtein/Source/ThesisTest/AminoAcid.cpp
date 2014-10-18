// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "SecondaryStructure.h"
#include "AminoAcid.h"
#include "LinkFragment.h"
#include "HydrogenBond.h"
#include "ProteinModel.h"

AAminoAcid::AAminoAcid(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
	, m_nextAminoAcid(nullptr)
	, m_previousAminoAcid(nullptr)
	, m_dynamicMaterial(nullptr)
	, m_defaultLinkFragmentClass(nullptr)
	, m_linkFragment(nullptr)
	, m_residueInformation(nullptr)
	, m_secondaryStructure(nullptr)
	, m_model(nullptr)
	, m_secondaryStructureType(ESecondaryStructure::ssCount)
	, m_normalColor(FColor::White)
	, m_helixColor(FColor::White)
	, m_betaStrandColor(FColor::White)
	, m_hydrogenBondColor(FColor::White)
	, m_normalHeight(0.f)
	, m_normalWidth(0.f)
	, m_helixWidth(0.f)
	, m_betaStrandWidth(0.f)
	, m_hydrogenBondLinkWidth(0.f)
	, m_linkFragmentScalePerUnrealUnit(0.f)
	, m_isAnimating(false)
	, m_locationToKeepTrackOf(FVector::ZeroVector)
	, m_locationInterpolator(Interpolator())
	, m_locationInterpolationSpeed(1.f)
{
	PrimaryActorTick.bCanEverTick = true;

	//Create the root SphereComponent to handle collision
	BaseCollisionComponent = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("BaseSphereComponent"));

	//Set the SphereComponent as the root component
	RootComponent = BaseCollisionComponent;

	//Create the static mesh component
	MeshComponent = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComponent"));

	//Attach the static mesh component to the root
	MeshComponent->AttachTo(RootComponent);

	//Create the text render component
	TextRenderComponent = PCIP.CreateDefaultSubobject<UTextRenderComponent>(this, TEXT("TextRenderComponent"));

	//attach the text render component to the root
	TextRenderComponent->AttachTo(RootComponent);
}

void AAminoAcid::Tick(float DeltaSeconds)
{
	if (m_locationInterpolator.IsPlaying())
	{
		//because the residue is moving independently, we are not going to translate it
		MoveTo(m_locationInterpolator.Poll());
	}

	if (m_isAnimating)
	{
		//check if it is done animating the residue and the link fragment
		if (!m_locationInterpolator.IsPlaying()
			&& !m_linkFragment->IsAnimating())
		{
			m_isAnimating = false;
			m_secondaryStructure->RemoveFromListOfModifiedResidues(this);
		}
	}
}

bool AAminoAcid::SpawnLinkParticleToNextAminoAcid()
{
	//we only spawn the link particles if we have an amino acid to connect to
	//and we have not spawned them before
	if (m_nextAminoAcid && (m_linkFragment == nullptr))
	{
		//get the start and end tangent
		FVector startTangent = FVector::ZeroVector;
		GetTangent(startTangent);
		FVector endTangent = FVector::ZeroVector;
		m_nextAminoAcid->GetTangent(endTangent);

		//get the endpoint location of the link fragment
		FVector linkStartLocation = GetActorLocation();
		FVector linkEndLocation = m_nextAminoAcid->GetActorLocation();

		//spawn the link fragment
		ALinkFragment* linkFragment = nullptr;
		linkFragment = UThesisStaticLibrary::SpawnBP<ALinkFragment>(GetWorld(), m_defaultLinkFragmentClass, FVector::ZeroVector, FRotator::ZeroRotator);
		//set the render properties of this link fragment
		linkFragment->UpdateRenderProperties(m_normalColor, m_helixColor, m_betaStrandColor, m_hydrogenBondColor,
			m_normalWidth, m_helixWidth, m_betaStrandWidth, m_hydrogenBondLinkWidth, m_normalHeight);

		linkFragment->ChangeLinkType(m_residueInformation->GetSecondaryStructure());

		linkFragment->SplineMeshComponent->SetStartAndEnd(linkStartLocation, startTangent, linkEndLocation, endTangent);
		m_linkFragment = linkFragment;

		return true;
	}

	return false;
}

void AAminoAcid::ChangeSecondaryStructureType(ESecondaryStructure::Type typeOfStructure, bool smoothTranslation)
{
	if (m_secondaryStructureType != typeOfStructure)
	{
		m_secondaryStructureType = typeOfStructure;
		
		if (m_linkFragment)
		{
			m_linkFragment->ChangeLinkType(m_secondaryStructureType, smoothTranslation);
		}
	}
}

const Residue* AAminoAcid::GetResidueInformation() const
{
	return m_residueInformation;
}

void AAminoAcid::AddHydrogenBond(AHydrogenBond* newBond)
{
	m_hydrogenBonds.Add(newBond);
}

bool AAminoAcid::BondWithResidueExists(const AAminoAcid* residue) const
{
	for (int i = 0; i < m_hydrogenBonds.Num(); ++i)
	{
		if (m_hydrogenBonds[i]->ContainsSpecifiedResidue(residue))
		{
			return true;
		}
	}

	return false;
}

UClass* AAminoAcid::GetDetaultLinkFragmentClass()
{
	return m_defaultLinkFragmentClass;
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

int AAminoAcid::GetSequenceNumber()
{
	if (m_residueInformation)
	{
		return m_residueInformation->GetSeqNumber();
	}
	else
	{
		return -1;
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

void AAminoAcid::SetParentModel(GHProtein::ProteinModel* parentModel)
{
	m_model = parentModel;
}

void AAminoAcid::SetSecondaryStructure(SecondaryStructure* secondaryStructure)
{
	m_secondaryStructure = secondaryStructure;
}

void AAminoAcid::SetResidueInformation(Residue* residueInformation)
{
	m_residueInformation = residueInformation;
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

AAminoAcid* AAminoAcid::GetPreviousAminoAcidPtr()
{
	return m_previousAminoAcid;
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
		m_defaultLinkFragmentClass = (UClass*)m_linkBlueprint->GeneratedClass;
	}
	else
	{
		m_defaultLinkFragmentClass = nullptr;
	}

	m_linkFragmentScalePerUnrealUnit = 1.0 / m_lengthOfLinkFragment;
	
	//create the dynamic material
	m_dynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
}

void AAminoAcid::UpdateLinkToNextAminoAcid()
{
	if (m_nextAminoAcid && m_linkFragment)
	{
		FRotator fragmentRotation = m_linkFragment->GetActorRotation();
		FVector fragmentLocation = m_linkFragment->GetActorLocation();

		m_linkFragment->SetActorLocation(FVector::ZeroVector);
		m_linkFragment->SetActorRotation(FRotator::ZeroRotator);

		FVector startTangent = FVector::ZeroVector;
		GetTangent(startTangent);
		startTangent = fragmentRotation.UnrotateVector(startTangent);
		
		FVector endTangent = FVector::ZeroVector;
		m_nextAminoAcid->GetTangent(endTangent);
		endTangent = fragmentRotation.UnrotateVector(endTangent);

		FVector linkStartLocation = GetActorLocation();
		FVector linkEndLocation = m_nextAminoAcid->GetActorLocation();

		m_linkFragment->SplineMeshComponent->SetStartPosition(fragmentRotation.UnrotateVector(linkStartLocation));
		m_linkFragment->SplineMeshComponent->SetEndPosition(fragmentRotation.UnrotateVector(linkEndLocation));
		m_linkFragment->SplineMeshComponent->SetStartTangent(startTangent);
		m_linkFragment->SplineMeshComponent->SetEndTangent(endTangent);
		//m_linkFragment->SplineMeshComponent->SetStartAndEnd(linkStartLocation,startTangent, linkEndLocation, endTangent);
		//m_linkFragment->SetActorLocation(fragmentLocation);
		m_linkFragment->SetActorRotation(fragmentRotation);

		//m_linkFragment->SplineMeshComponent->SetStartTangent(startTangent);
		//m_linkFragment->SplineMeshComponent->SetEndTangent(endTangent);
	}
}

void AAminoAcid::UpdateLinkFragmentTangents()
{
	FRotator rotation = m_linkFragment->GetActorRotation();
	FVector startTangent = FVector::ZeroVector;
	GetTangent(startTangent);
	FVector endTangent = FVector::ZeroVector;
	m_nextAminoAcid->GetTangent(endTangent);

	m_linkFragment->UpdateTangents(rotation.UnrotateVector(startTangent), rotation.UnrotateVector(endTangent));
}

void AAminoAcid::HideLinkFragment()
{
	m_linkFragment->Hide();
}

void AAminoAcid::Translate(const FVector& deltaLocation)
{
	MoveTo(GetActorLocation() + deltaLocation, true);
	/*
	SetActorLocation(GetActorLocation() + deltaLocation);
	TranslateLinkFragment(deltaLocation);
	m_locationToKeepTrackOf += deltaLocation;
	*/

	//AAminoAcid* tempResidue = nullptr;

	//update the chains of the amino acids
	/*
	if (m_previousAminoAcid)
	{
		//update the chain handled by the previous amino acid of the previous amino acid
		tempResidue = m_previousAminoAcid->GetPreviousAminoAcidPtr();
		if (tempResidue)
		{
			tempResidue->UpdateLinkToNextAminoAcid();
		}

		//update the chain handled by the previous amino acid
		m_previousAminoAcid->UpdateLinkToNextAminoAcid();
	}

	//update the cahin handled by this amino acid
	UpdateLinkToNextAminoAcid();
	*/
	//TranslateLinkFragment(deltaLocation);

	/*
	//update the chain handled by the next amino acid
	if (m_nextAminoAcid)
	{
		//m_nextAminoAcid->UpdateLinkToNextAminoAcid();
	}
	*/
}

void AAminoAcid::TranslateLinkFragment(const FVector& deltaLocation)
{
	if (m_linkFragment)
	{
		m_linkFragment->Translate(deltaLocation);
	}
}

void AAminoAcid::RotateAminoAcidFromSpecifiedPoint(const FRotationMatrix& rotationMatrix, const FVector& rotationPoint)
{
	//rotate the amino acid and the chain
	FVector distanceFromRotationPoint = GetActorLocation() - rotationPoint;
	distanceFromRotationPoint = rotationMatrix.TransformVector(distanceFromRotationPoint);

	SetActorLocation(distanceFromRotationPoint + rotationPoint);

	distanceFromRotationPoint = m_locationToKeepTrackOf - rotationPoint;
	distanceFromRotationPoint = rotationMatrix.TransformVector(distanceFromRotationPoint);
	m_locationToKeepTrackOf = distanceFromRotationPoint + rotationPoint;

	//if this amino acid has a link fragment then we rotate it also
	RotateLinkFragmentAboutSpecifiedPoint(rotationMatrix, rotationPoint);

	//if we are interpolating the location, update the values the interpolator is using
	if (m_locationInterpolator.IsPlaying())
	{
		m_locationInterpolator.RotateValuesFromSpecifiedPoint(rotationMatrix, rotationPoint);
	}
}

void AAminoAcid::RotateLinkFragmentAboutSpecifiedPoint(const FRotationMatrix& rotationMatrix, const FVector& rotationPoint)
{
	if (m_linkFragment)
	{
		FVector distanceFromPivotPoint = m_linkFragment->GetActorLocation() - rotationPoint;

		//rotate the distance
		distanceFromPivotPoint = rotationMatrix.TransformVector(distanceFromPivotPoint);
		//now set the new location to the fragment
		m_linkFragment->SetActorLocation(rotationPoint + distanceFromPivotPoint);

		//rotate the object
		FRotationMatrix currentRotationMatrix(m_linkFragment->GetActorRotation());
		currentRotationMatrix *= rotationMatrix;
		m_linkFragment->SetActorRotation(currentRotationMatrix.Rotator());
	}
}

ESecondaryStructure::Type AAminoAcid::GetSecondaryStructure()
{
	return m_secondaryStructureType;
}

void AAminoAcid::SetRenderProperties(const FColor& normalColor, const FColor& helixColor, const FColor& betaStrandColor,
	const FColor& hydrogenBondColor, float normalWidth, float helixLinkWidth, float betaStrandLinkWidth, float hydrogenBondLinkWidth,
	float normalLinkHeight)
{
	m_normalColor = normalColor;
	m_helixColor = helixColor;
	m_betaStrandColor = betaStrandColor;
	m_hydrogenBondColor = hydrogenBondColor;

	m_normalWidth = normalWidth;
	m_helixWidth = helixLinkWidth;
	m_betaStrandWidth = betaStrandLinkWidth;
	m_hydrogenBondLinkWidth = hydrogenBondLinkWidth;

	m_normalHeight = normalLinkHeight;

	UpdateLinkFragmentRenderProperties();
}

void AAminoAcid::UpdateLinkFragmentRenderProperties()
{
	if (m_linkFragment)
	{
		m_linkFragment->UpdateRenderProperties(m_normalColor, m_helixColor, m_betaStrandColor,
			m_hydrogenBondColor, m_normalWidth, m_helixWidth, m_betaStrandWidth, m_hydrogenBondLinkWidth,
			m_normalHeight);
	}
}

//function maps the type of amino acid to a string that is returned
FString AAminoAcid::GetResidueTypeString() const
{
	if (m_residueInformation)
	{
		return MapResidueInfo(m_residueInformation->GetType()).fullName;
	}
	else
	{
		return "INVALID DATA";
	}
}

ResidueInfo AAminoAcid::GetAminoAcidInfo() const
{
	if (m_residueInformation)
	{
		return MapResidueInfo(m_residueInformation->GetType());
	}
	return kResidueInfo[0];
}

void AAminoAcid::SetAminoAcidType(TEnumAsByte<EResidueType::Type> aminoAcidType)
{
	if (!m_residueInformation)
	{
		m_residueInformation = new Residue();
	}

	m_residueInformation->SetType(aminoAcidType);
}

void AAminoAcid::KeepTrackOfLocation(const FVector& locationToKeepTrackOf)
{
	m_locationToKeepTrackOf = locationToKeepTrackOf;
}

void AAminoAcid::MoveTo(const FVector& finalLocation, bool translateLinkFragment, bool interpolate)
{
	if (interpolate)
	{
		m_locationInterpolator.ResetInterpolator(GetActorLocation(), finalLocation, m_locationInterpolationSpeed, false, false, 1);
	}
	else
	{
		//check the displacement
		FVector displacement = finalLocation - GetActorLocation();

		SetActorLocation(finalLocation);

		//check how we are supposed to update the linkFragment
		//we translate whenever we are translating the entire protein
		if (translateLinkFragment)
		{
			TranslateLinkFragment(displacement);
			
			//if we have a location interpolator, we update it with the new locations we are trying to approach
			if (m_locationInterpolator.IsPlaying())
			{
				m_locationInterpolator.OffsetValues(displacement);
			}
			m_locationToKeepTrackOf += displacement;
		}
		else
		{
			//if we are not translating, then update the link fragments that are affected by
			//moving this residue
			UpdateLinkToNextAminoAcid();
			if (m_previousAminoAcid)
			{
				m_previousAminoAcid->UpdateLinkToNextAminoAcid();

				if (m_previousAminoAcid->GetPreviousAminoAcidPtr())
				{
					//we do this because the tangent of this link fragment is affected by the change in position of
					//the current residue
					m_previousAminoAcid->GetPreviousAminoAcidPtr()->UpdateLinkFragmentTangents();
				}
			}
		}

		//update the hydrogen bonds associated with this residue
		for (int i = 0; i < m_hydrogenBonds.Num(); ++i)
		{
			m_hydrogenBonds[i]->ChangeLocationOfAssociatedEnd(this, finalLocation);
		}
	}
}

void AAminoAcid::Stabilize(ESecondaryStructure::Type structureType)
{
	//do nothing for the moment
	//go back to the location we were keeping track of
	MoveTo(m_locationToKeepTrackOf, false, true);
	ChangeSecondaryStructureType(structureType, true);
	m_secondaryStructure->AddToListOfModifiedResidues(this);
	m_isAnimating = true;
}

void AAminoAcid::Break(const FVector& newLocation)
{
	MoveTo(newLocation, false, true);
	ChangeSecondaryStructureType(ESecondaryStructure::ssLoop, true);
	m_secondaryStructure->AddToListOfModifiedResidues(this);
	m_isAnimating = true;
}

void AAminoAcid::Break()
{
	ChangeSecondaryStructureType(ESecondaryStructure::ssLoop, true);
	m_secondaryStructure->AddToListOfModifiedResidues(this);
	//make sure it does not move
	//MoveTo(m_locationToKeepTrackOf, false, true);		//location to keep track of should be the original location
	m_isAnimating = true;
}

void AAminoAcid::Shake()
{
	//do nothing for the moment
}