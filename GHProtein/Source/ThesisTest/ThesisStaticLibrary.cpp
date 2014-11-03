// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "ThesisStaticLibrary.h"

//=======================INTERPOLATOR===========================
int Interpolator::s_id = 0;
TArray<Interpolator*> Interpolator::s_interpolatorList = TArray<Interpolator*>();

Interpolator::Interpolator()
: m_originalGoal(FVector::ZeroVector)
, m_originalStart(FVector::ZeroVector)
, m_currentGoal(FVector::ZeroVector)
, m_currentStart(FVector::ZeroVector)
, m_current(FVector::ZeroVector)
, m_interpSpeed(0.f)
, m_play(false)
, m_loop(false)
, m_isInList(false)
, m_playCount(0)
, m_maxPlayCount(0)
, m_id(s_id++)
{
	AddToListOfInterpolators(this);
}

Interpolator::~Interpolator()
{
	//remove this from the list of interpolators
	RemoveInterpolatorFromList(this);
}

void Interpolator::Update(float deltaTime)
{
	if (m_play)
	{
		//check if we have completed one iteration
		if (m_current == m_currentGoal)
		{
			++m_playCount;

			//check if we should continue playing
			if (m_playCount >= m_maxPlayCount && m_maxPlayCount > 0)
			{
				m_play = false;
				return;
			}

			//check if we are looping or just jumping back
			if (m_loop)
			{
				m_currentGoal = m_currentStart;
				m_currentStart = m_current;
			}
			else
			{
				m_current = m_currentStart;
			}
		}

		m_current = FMath::VInterpConstantTo(m_current, m_currentGoal, deltaTime, m_interpSpeed);
	}
}

void Interpolator::OffsetValues(const FVector& offset)
{
	m_originalGoal += offset;
	m_originalStart += offset;

	m_currentGoal += offset;
	m_currentStart += offset;

	m_current += offset;
}

void Interpolator::RotateValuesFromSpecifiedPoint(const FRotationMatrix& rotationMatrix, const FVector& rotationPoint)
{
	UThesisStaticLibrary::RotateVectorAroundSpecifiedPoint(m_originalGoal, rotationMatrix, rotationPoint);
	UThesisStaticLibrary::RotateVectorAroundSpecifiedPoint(m_originalStart, rotationMatrix, rotationPoint);
	UThesisStaticLibrary::RotateVectorAroundSpecifiedPoint(m_currentGoal, rotationMatrix, rotationPoint);
	UThesisStaticLibrary::RotateVectorAroundSpecifiedPoint(m_currentStart, rotationMatrix, rotationPoint);
	UThesisStaticLibrary::RotateVectorAroundSpecifiedPoint(m_current, rotationMatrix, rotationPoint);
}

FVector Interpolator::Poll() const
{
	return m_current;
}

bool Interpolator::IsPlaying() const
{
	return m_play;
}

void Interpolator::ResetInterpolator(const FVector& start, const FVector& goal, float interpSpeed,
	bool loop, bool startRandomlyWithinRange, int maxPlayCount)
{
	m_originalStart = start;
	m_originalGoal = goal;

	m_currentStart = start;
	m_currentGoal = goal;

	m_maxPlayCount = maxPlayCount;
	m_playCount = 0;

	m_interpSpeed = interpSpeed;

	m_loop = loop;
	if (loop)
	{
		//double play count because we are basically doing two runs per play
		m_maxPlayCount *= 2;
	}

	if (startRandomlyWithinRange)
	{
		float randVal = FMath::SRand();
		m_current = m_currentStart * (1.f - randVal) + m_currentGoal * randVal;
	}
	else
	{
		m_current = m_currentStart;
	}

	m_play = true;
}

void Interpolator::TogglePlay()
{
	m_play = !m_play;
}

void Interpolator::AddToListOfInterpolators(Interpolator* interpolatorToAdd)
{
	if (!interpolatorToAdd->m_isInList)
	{
		s_interpolatorList.Add(interpolatorToAdd);
		interpolatorToAdd->m_isInList = true;
	}
}

void Interpolator::UpdateInterpolators(float deltaTime)
{
	//iterate through the list of interpolators and update them
	for (int i = 0; i < s_interpolatorList.Num(); ++i)
	{
		s_interpolatorList[i]->Update(deltaTime);
	}
}

void Interpolator::RemoveInterpolatorFromList(Interpolator* interpolatorToRemove)
{
	for (int i = 0; i < s_interpolatorList.Num(); ++i)
	{
		if (s_interpolatorList[i]->m_id == interpolatorToRemove->m_id)
		{
			//found the interpolator
			//now we remove it
			s_interpolatorList.RemoveAt(i);
			break;
		}
	}
}
//==============================================================

UThesisStaticLibrary::UThesisStaticLibrary(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{}

ELinkType::Type UThesisStaticLibrary::GetLinkTypeFromSecondaryStructure(ESecondaryStructure::Type secondaryStructureType)
{
	switch (secondaryStructureType)
	{
	case ESecondaryStructure::ssAlphaHelix:
		return ELinkType::ELink_Helix;

	case ESecondaryStructure::ssStrand:
		return ELinkType::ELink_BetaStrand;
	
	default:
		return ELinkType::ELink_Backbone;
	}
}

ESecondaryStructure::Type UThesisStaticLibrary::GetSecondaryStructureTypeFromVector(const TArray<double>& secondaryStructureTypeVector)
{
	ESecondaryStructure::Type secondaryStructureType = ESecondaryStructure::ssLoop;

	//the vector representation of the the secondary structure type cannot have more than
	//3 elements
	if (secondaryStructureTypeVector.Num() <= 3
		&& secondaryStructureTypeVector.Num() > 0)
	{
		int highestIndex = 0;
		double highestValue = secondaryStructureTypeVector[highestIndex];
		for (int i = highestIndex + 1; i < secondaryStructureTypeVector.Num(); ++i)
		{
			if (highestValue < secondaryStructureTypeVector[i])
			{
				highestIndex = i;
				highestValue = secondaryStructureTypeVector[highestIndex];
			}
		}

		secondaryStructureType = ESecondaryStructure::Type(highestIndex);
	}

	return secondaryStructureType;
}

void UThesisStaticLibrary::RotateVectorAroundSpecifiedPoint(FVector& out_vectorToRotate, const FMatrix& rotationMatrix,
	const FVector& rotationPoint)
{
	FVector distanceFromRotationPoint = out_vectorToRotate - rotationPoint;
	distanceFromRotationPoint = rotationMatrix.TransformVector(distanceFromRotationPoint);
	out_vectorToRotate = distanceFromRotationPoint + rotationPoint;
}