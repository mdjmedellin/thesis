#include "ThesisTest.h"
#include "ProteinModel.h"
#include "SecondaryStructure.h"
#include "AminoAcid.h"
#include "LinkFragment.h"

//======================BetaSheet===========================
BetaSheet::BetaSheet(SecondaryStructure* strand1, SecondaryStructure* strand2, GHProtein::ProteinModel* parentModel)
: m_proteinModel(parentModel)
{
	m_strands.Add(strand1);
	m_strands.Add(strand2);
}

void BetaSheet::SpawnHydrogenBonds()
{
	//we are going to need to iterate through all the strands
	SecondaryStructure* outerStrand = nullptr;
	AAminoAcid* currentOuterResidue = nullptr;
	AAminoAcid* endOuterResidue = nullptr;
	for (int outerIndex = 0; outerIndex < m_strands.Num(); ++outerIndex)
	{
		//get the current strand and iterate and spawn the hydrogen bonds of all its residues
		outerStrand = m_strands[outerIndex];
		currentOuterResidue = outerStrand->GetHeadResidue();
		endOuterResidue = outerStrand->GetEndResidue();
		bool continueIteratingOverResidues = currentOuterResidue != nullptr;
		while (continueIteratingOverResidues)
		{
			//spawn the hydrogen bond of the current residue
			SpawnHydrogenBondsOfSpecifiedResidue(currentOuterResidue);

			//check end condition
			if (currentOuterResidue == endOuterResidue)
			{
				continueIteratingOverResidues = false;
			}
			else
			{
				currentOuterResidue = currentOuterResidue->GetNextAminoAcidPtr();
			}
		}
	}
}

void BetaSheet::SpawnHydrogenBondsOfSpecifiedResidue(AAminoAcid* residue)
{
	const Residue* residueInfo = residue->GetResidueInformation();

	//a residue can only have to bridge partners
	//one on each lateral side
	for (int i = 0; i < 2; ++i)
	{
		BridgePartner partner = residueInfo->GetBetaPartner(i);

		if (partner.residue)
		{
			AAminoAcid* betaPartner = m_proteinModel->GetAminoAcidWithSpecifiedId(partner.number);

			//if the residue has a valid partner and a bond between them has not been created
			//spawn a bond between them
			if (betaPartner && !residue->BondWithResidueExists(betaPartner))
			{
				//if the bond has not yet been created, then we create it
				AHydrogenBond* hBond = m_proteinModel->SpawnHydrogenBond(residue, betaPartner);
			}
		}
	}
}
//========================================================================================

//==========================SECONDARY STRUCTURE===========================================
SecondaryStructure::SecondaryStructure(ESecondaryStructure::Type secondaryStructureType,
	GHProtein::ProteinModel* parentModel)
	: m_secondaryStructureType(secondaryStructureType)
	, m_nextSecondaryStructure(nullptr)
	, m_headAminoAcid(nullptr)
	, m_tailAminoAcid(nullptr)
	, m_parentModel(parentModel)
	, m_irreversibleChangeTemperatureCelsius(0.f)
	, m_breakTemperatureCelsius(0.f)
	, m_regularTemperatureCelsius(0.f)
	, m_prevTemperatureCelsius(0.f)
	, m_canReverseChange(true)
	, m_temperatureState(ETemperatureState::ETemperatureState_Stable)
{}

SecondaryStructure::~SecondaryStructure()
{}

AAminoAcid* SecondaryStructure::GetAminoAcidWithSpecifiedId(int sequenceNumber)
{
	AAminoAcid* foundResidue = nullptr;
	AAminoAcid* currentResidue = m_headAminoAcid;

	while (currentResidue && currentResidue != m_tailAminoAcid->GetNextAminoAcidPtr())
	{
		if (currentResidue->GetSequenceNumber() == sequenceNumber)
		{
			foundResidue = currentResidue;
			break;
		}
		else
		{
			currentResidue = currentResidue->GetNextAminoAcidPtr();
		}
	}

	return foundResidue;
}

void SecondaryStructure::SetNextStructurePtr(SecondaryStructure* nextStructure)
{
	//check if we alreay have a vaird ptr to the next secondary structure
	if (m_nextSecondaryStructure)
	{
		SecondaryStructure* tmp = m_nextSecondaryStructure;
		nextStructure->SetNextStructurePtr(tmp);
	}
	
	m_nextSecondaryStructure = nextStructure;
}

void SecondaryStructure::SetEnviromentalProperties(float currentTemperatureCelsius, float regularTemperatureCelsius,
	float breakTemperatureCelsius, float irreversibleChangeTemperatureCelsius)
{
	m_prevTemperatureCelsius = currentTemperatureCelsius;
	m_regularTemperatureCelsius = regularTemperatureCelsius;
	m_breakTemperatureCelsius = breakTemperatureCelsius;
	m_irreversibleChangeTemperatureCelsius = irreversibleChangeTemperatureCelsius;

	UpdateStructureAccordingToSpecifiedTemperature(currentTemperatureCelsius);
}

void SecondaryStructure::AppendAminoAcid(AAminoAcid* residue, bool isCustomChain)
{
	//Get the residue information
	const Residue* residueInfo = residue->GetResidueInformation();

	residue->SetSecondaryStructure(this);

	//if we do not have an amino acid, then we take the first amino acid as the head amino acid
	if (m_headAminoAcid == nullptr)
	{
		m_headAminoAcid = residue;
		m_tailAminoAcid = residue;
		//set the structure type from the head amino acid

		if (!isCustomChain)
		m_secondaryStructureType = residueInfo->GetSecondaryStructure();
	}
	else
	{
		//if we already have a head amino acid, then we take the latest appended amino acid as the tail
		m_tailAminoAcid = residue;
	}

	//if this is a beta strand, make sure to save the partners
	if (m_secondaryStructureType == ESecondaryStructure::ssStrand && !isCustomChain)
	{
		//extract the character that indicates the partner strand
		BridgePartner currentPartner;
		for (int betaPartnerIndex = 0; betaPartnerIndex < 2; ++betaPartnerIndex)
		{
			currentPartner = residueInfo->GetBetaPartner(betaPartnerIndex);
			if (currentPartner.ladder != ' ')
			{
				AddBridgeLabel(currentPartner.ladder);
			}
		}
	}
}

void SecondaryStructure::AddBridgeLabel(uint32 bridgeLabel)
{
	//if we do not have the label associated with this strand, then we add it to the list of associated labels
	if (m_bridgeLabels.Find(bridgeLabel) == INDEX_NONE)
	{
		m_bridgeLabels.Add(bridgeLabel);
	}
}

bool SecondaryStructure::IsPartOfSpecifiedBridgeLabels(const TArray<uint32>& bridgeLabels) const
{
	//check if this secondary structure shares at least one of the bridge labels passed in
	for (int j = 0; j < bridgeLabels.Num(); ++j)
	{
		if (bridgeLabels[j] == ' ')
		{
			continue;
		}
		else
		{
			for (int i = 0; i < m_bridgeLabels.Num(); ++i)
			{
				if (bridgeLabels[j] == m_bridgeLabels[i])
				{
					return true;
				}
			}
		}
	}

	return false;
}

void SecondaryStructure::GetBridgeLabels(TArray<uint32>& out_bridgeLabels) const
{
	out_bridgeLabels = m_bridgeLabels;
}

ESecondaryStructure::Type SecondaryStructure::GetSecondaryStructureType() const
{
	return m_secondaryStructureType;
}

SecondaryStructure* SecondaryStructure::GetNextStructurePtr()
{
	return m_nextSecondaryStructure;
}

AAminoAcid* SecondaryStructure::GetHeadResidue()
{
	return m_headAminoAcid;
}

AAminoAcid* SecondaryStructure::GetEndResidue()
{
	return m_tailAminoAcid;
}

int SecondaryStructure::GetAminoAcidCount()
{
	int count = 0;
	for (AAminoAcid* currentResidue = m_headAminoAcid;
		currentResidue != nullptr && (m_tailAminoAcid && currentResidue != m_tailAminoAcid->GetNextAminoAcidPtr());
		currentResidue = currentResidue->GetNextAminoAcidPtr())
	{
		++count;
	}

	return count;
}

bool SecondaryStructure::ContainsSpecifiedResidue(AAminoAcid* residue)
{
	//iterate through the list of amino acids and see if the specified residue is there
	for (AAminoAcid* currentResidue = m_headAminoAcid;
		currentResidue != nullptr && (m_tailAminoAcid && currentResidue != m_tailAminoAcid->GetNextAminoAcidPtr());
		currentResidue = currentResidue->GetNextAminoAcidPtr())
	{
		//if both pointers are pointing to the same address then we are pointing to the same residue
		if (currentResidue == residue)
		{
			return true;
		}
	}

	return false;
}

void SecondaryStructure::SpawnHydrogenBonds()
{
	//we only spawn for alpha helix structures
	if (m_secondaryStructureType != ESecondaryStructure::ssAlphaHelix)
	{
		return;
	}

	bool keepLooping = true;
	for (AAminoAcid* currentResidue = m_headAminoAcid;
		keepLooping && currentResidue != m_tailAminoAcid;
		currentResidue = currentResidue->GetNextAminoAcidPtr())
	{
		//look for the residues that make up the hydrogen bond
		int index = 0;
		AAminoAcid* partnerResidue = nullptr;
		for (partnerResidue = currentResidue;
			partnerResidue != m_tailAminoAcid && index < 4;
			++index, partnerResidue = partnerResidue->GetNextAminoAcidPtr())
		{}

		if (index == 4)
		{
			//we found a valid partner residue to create a hydrogen bond
			AHydrogenBond* newlyCreatedBond = m_parentModel->SpawnHydrogenBond(currentResidue, partnerResidue);
			m_hydrogenBonds.Add(newlyCreatedBond);
		}
		else
		{
			//the first time we cannot find a partner indicates where we should stop spawning hydrogen bonds
			keepLooping = false;
		}
	}
}

bool SecondaryStructure::SetTemperature(float temperatureCelsius)
{
	bool changeInTemperatureTriggeredChange = false;
	//temperature changes only affect secondary structures
	//to be more specific, it only affects alpha helices and beta strands
	if (m_secondaryStructureType == ESecondaryStructure::ssAlphaHelix
		|| m_secondaryStructureType == ESecondaryStructure::ssStrand)
	{
		//check if the structure can reverse the change
		if (m_canReverseChange)
		{
			//modify the secondary structure accordingly
			changeInTemperatureTriggeredChange =  UpdateStructureAccordingToSpecifiedTemperature(temperatureCelsius);
		}
	}

	m_prevTemperatureCelsius = temperatureCelsius;
	return changeInTemperatureTriggeredChange;
}

bool SecondaryStructure::UpdateStructureAccordingToSpecifiedTemperature(float temperatureCelsius)
{
	m_canReverseChange = true;

	bool structureIsBeingModified = false;

	if (temperatureCelsius > m_irreversibleChangeTemperatureCelsius)
	{
		if (m_temperatureState != ETemperatureState::ETemperatureState_Melting)
		{
			m_temperatureState = ETemperatureState::ETemperatureState_Melting;
			TArray<AAminoAcid*> residues;
			ExtractResidues(residues);
			BreakStructure(residues);
			structureIsBeingModified = true;

			//add the secondary structure to the list of sturctures being modified
			this->m_parentModel->AddToListOfModifiedSecondaryStructures(this);
		}

		if (m_canReverseChange)
		{
			m_canReverseChange = false;
		}
	}
	else if (temperatureCelsius > m_breakTemperatureCelsius)
	{
		if (m_temperatureState != ETemperatureState::ETemperatureState_Melting)
		{
			TArray<AAminoAcid*> residues;
			ExtractResidues(residues);
			BreakStructure(residues);
			structureIsBeingModified = true;

			this->m_parentModel->AddToListOfModifiedSecondaryStructures(this);
			m_temperatureState = ETemperatureState::ETemperatureState_Melting;
		}
	}
	else if (temperatureCelsius > m_regularTemperatureCelsius)
	{
		if (m_temperatureState != ETemperatureState::ETemperatureState_Stable)
		{
			//technically, this should be able to stabilize also
			TArray<AAminoAcid*> residues;
			ExtractResidues(residues);
			StabilizeResidues(residues);
			structureIsBeingModified = true;

			this->m_parentModel->AddToListOfModifiedSecondaryStructures(this);
			m_temperatureState = ETemperatureState::ETemperatureState_Stable;
		}
	}
	else
	{
		if (m_temperatureState != ETemperatureState::ETemperatureState_Stable)
		{
			//stabilize the residues
			TArray<AAminoAcid*> residues;
			ExtractResidues(residues);
			StabilizeResidues(residues);
			structureIsBeingModified = true;

			this->m_parentModel->AddToListOfModifiedSecondaryStructures(this);
			m_temperatureState = ETemperatureState::ETemperatureState_Stable;
		}
	}

	return structureIsBeingModified;
}

void SecondaryStructure::StabilizeResidues(TArray<AAminoAcid*>& residues)
{
	//we can only go back to our original arrangment if we did not expose the
	//protein to such a high temperature that it cannot renature
	if (m_canReverseChange)
	{
		for (int i = 0; i < residues.Num(); ++i)
		{
			residues[i]->Stabilize(m_secondaryStructureType);
		}
	}
}

void SecondaryStructure::ShakeResidues(TArray<AAminoAcid*>& residues)
{
	for (int i = 0; i < residues.Num(); ++i)
	{
		residues[i]->Shake();
	}
}

void SecondaryStructure::ExtractResidues(TArray<AAminoAcid*>& out_residueContainer)
{
	//iterate through the residues that make up this structure and store them in the container
	AAminoAcid* currentResidue = m_headAminoAcid;
	bool keepIterating = true;

	//group all the residues into a container
	while (keepIterating)
	{
		out_residueContainer.Add(currentResidue);
		if (currentResidue != m_tailAminoAcid)
		{
			currentResidue = currentResidue->GetNextAminoAcidPtr();
		}
		else
		{
			keepIterating = false;
		}
	}
}

void SecondaryStructure::BreakStructure(const TArray<AAminoAcid*>& residues)
{
	if (m_secondaryStructureType == ESecondaryStructure::ssStrand)
	{
		AAminoAcid* currentResidue = m_headAminoAcid;
		bool keepLooping = true;
		while (currentResidue && keepLooping)
		{
			if (currentResidue == m_tailAminoAcid)
			{
				keepLooping = false;
			}

			currentResidue->KeepTrackOfLocation(currentResidue->GetActorLocation());
			currentResidue->Break();
			currentResidue = currentResidue->GetNextAminoAcidPtr();
		}
	}
	else
	{
		//This line fitting will only work appropriately for spirals, it makes assumptions other type of data
		//will not meet
		int num = residues.Num();
		int axisToUse = -1;
		double inverseNum = 1.f / num;

		//extract the data we need from the cluster of points
		FVector means = FVector::ZeroVector;
		FVector sums = FVector::ZeroVector;
		FVector sumsSquared = FVector::ZeroVector;
		double sum_xy = 0.0;
		double sum_xz = 0.0;
		double sum_yz = 0.0;

		FVector location;
		for (int i = 0; i < residues.Num(); ++i)
		{
			location = residues[i]->GetActorLocation();

			//we want to make dure the residue keeps track of its original location
			//this is so that it can later return to its original location if the
			//change is reversible
			residues[i]->KeepTrackOfLocation(location);

			sums.X += location.X;
			sums.Y += location.Y;
			sums.Z += location.Z;

			sumsSquared.X += (location.X * location.X);
			sumsSquared.Y += (location.Y * location.Y);
			sumsSquared.Z += (location.Z * location.Z);

			sum_xy += (location.X * location.Y);
			sum_xz += (location.X * location.Z);
			sum_yz += (location.Y * location.Z);
		}

		if (m_secondaryStructureType == ESecondaryStructure::ssStrand)
		{
			for (int i = 0; i < residues.Num(); ++i)
			{
				residues[i]->ChangeSecondaryStructureType(ESecondaryStructure::ssLoop, true);
			}

			return;
		}

		//store the multiplicative sums in their appropriate place
		TArray<FVector> multiplicativeSums;
		multiplicativeSums.SetNum(3);
		multiplicativeSums[0].Set(0.f, sum_xy, sum_xz);
		multiplicativeSums[1].Set(sum_xy, 0.f, sum_yz);
		multiplicativeSums[2].Set(sum_xz, sum_yz, 0.f);

		//calculate the means
		for (int i = 0; i < 3; ++i)
		{
			means[i] = sums[i] * inverseNum;
		}

		//calculate the possible run values of the slope formula
		FVector possibleRunVariables = FVector::ZeroVector;
		for (int i = 0; i < 3; ++i)
		{
			possibleRunVariables[i] = sumsSquared[i] - (sums[i] * means[i]);
		}

		//precalculate some of the possible values we will need for the point slope formula
		TArray<FVector> possibleSlopes;
		TArray<FVector> possibleIntercepts;
		possibleSlopes.SetNum(3);
		possibleIntercepts.SetNum(3);

		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				if (i != j)
				{
					//at the moment we are only calculating the possible rise values of the slope formula
					possibleSlopes[i][j] = (multiplicativeSums[i][j] - sums[i] * means[j]);
				}
			}
		}

		double currentMax = possibleRunVariables[0];
		axisToUse = 0;
		for (int i = 1; i < 3 && currentMax == 0.0; ++i)
		{
			currentMax = abs(possibleRunVariables[i]);
			axisToUse = i;
		}

		//check which axis we are going to use as the independent variable
		for (int i = 0; i < 3; ++i)
		{
			if (possibleRunVariables[i] != 0.0
				&& currentMax < abs(possibleRunVariables[i]))
			{
				//we can use this variable as the independent variable
				currentMax = abs(possibleRunVariables[i]);
				axisToUse = i;
			}
		}

		//finish constructing the slopes
		for (int i = 0; i < 3; ++i)
		{
			if (i != axisToUse)
			{
				possibleSlopes[axisToUse][i] /= possibleRunVariables[axisToUse];

				//calculate the intercepts
				possibleIntercepts[axisToUse][i] = means[i] - (possibleSlopes[axisToUse][i] * means[axisToUse]);
			}
		}

		//find the min and max distance from the projected position to the actual position
		FVector actorLocation = FVector::ZeroVector;
		FVector projectedLocation = FVector::ZeroVector;
		float shortestDistanceSquared = 0.f;
		float longestDistanceSquared = 0.f;

		for (int i = 0; i < residues.Num(); ++i)
		{
			actorLocation = residues[i]->GetActorLocation();
			projectedLocation = actorLocation;

			for (int j = 0; j < 3; ++j)
			{
				if (j != axisToUse)
				{
					projectedLocation[j] = (possibleSlopes[axisToUse][j] * projectedLocation[axisToUse]) + possibleIntercepts[axisToUse][j];
				}
			}

			float currentDistance = FVector::DistSquared(actorLocation, projectedLocation);
			if (i != 0)
			{
				if (currentDistance > longestDistanceSquared)
				{
					longestDistanceSquared = currentDistance;
				}
				else if (currentDistance < shortestDistanceSquared)
				{
					shortestDistanceSquared = currentDistance;
				}
			}
			else
			{
				longestDistanceSquared = FVector::DistSquared(actorLocation, projectedLocation);
				shortestDistanceSquared = longestDistanceSquared;
			}
		}

		float shortestDistance = sqrt(shortestDistanceSquared);
		float longestDistance = sqrt(longestDistanceSquared);


		//calculate the end and start points of the line we use to visualize the best fit line
		FVector startLocation = FVector::ZeroVector;
		FVector endLocation = FVector::ZeroVector;

		actorLocation = residues[0]->GetActorLocation();
		projectedLocation = actorLocation;
		for (int j = 0; j < 3; ++j)
		{
			if (j != axisToUse)
			{
				projectedLocation[j] = (possibleSlopes[axisToUse][j] * projectedLocation[axisToUse]) + possibleIntercepts[axisToUse][j];
			}
		}
		startLocation = projectedLocation;

		actorLocation = residues.Last()->GetActorLocation();
		projectedLocation = actorLocation;
		for (int j = 0; j < 3; ++j)
		{
			if (j != axisToUse)
			{
				projectedLocation[j] = (possibleSlopes[axisToUse][j] * projectedLocation[axisToUse]) + possibleIntercepts[axisToUse][j];
			}
		}
		endLocation = projectedLocation;

		FVector startToEnd = endLocation - startLocation;
		float distanceStartToEnd = FVector::Dist(endLocation, startLocation);
		startToEnd.Normalize();

		//apply the gram schmidtt process for getting a perpendicular line
		//start

		//start to end is our direction
		//get index of min
		float minMagnitude = startToEnd.X;
		int indexOfMin = 0;

		for (int i = 1; i < 3; ++i)
		{
			if (abs(minMagnitude) > abs(startToEnd[i]))
			{
				minMagnitude = startToEnd[i];
				indexOfMin = i;
			}
		}

		//create a vector where the min index is set to 1 and the rest to 0
		FVector baseVector = FVector::ZeroVector;
		baseVector[indexOfMin] = 1.f;

		//now project the new base vector onto the line we have
		//since we know that startToEnd is a normalized vector
		//we do not have to divide in our projection equation
		FVector projectedVector = FVector::DotProduct(baseVector, startToEnd) * startToEnd;

		//subtract the portion that was projected from the base vector
		baseVector -= projectedVector;

		//cross product should give us the other basis vector
		FVector baseVector2 = FVector::CrossProduct(baseVector, startToEnd);

		//Gram-Schmitt process end
		//end

		//randomly move all of the residues that conform the alpha helix
		float  distanceBetweenIncrements = distanceStartToEnd / (num - 1);
		FVector tempLocation = FVector::ZeroVector;
		for (int i = 1; i < num - 1; ++i)
		{
			float rads = (PI * 0.5 * i) + (.5 * PI);
			float sinVal = sinf(rads);

			tempLocation = startLocation + (i * distanceBetweenIncrements * startToEnd) + (sinVal * longestDistance * baseVector);
			residues[i]->Break(tempLocation);
		}

		//change the structure type of the first and last residue in the structure
		residues[0]->Break();
		residues.Last()->Break();
	}
}

void SecondaryStructure::AddToListOfModifiedResidues(AAminoAcid* residueModified)
{
	m_modifiedResidues.AddUnique(residueModified);
}

void SecondaryStructure::RemoveFromListOfModifiedResidues(AAminoAcid* residueModified)
{
	m_modifiedResidues.Remove(residueModified);

	if (m_modifiedResidues.Num() == 0)
	{
		//if all the residues finished animating
		//remove the secondary structure from the list of structures being modified
		m_parentModel->RemoveFromListOfModifiedSecondaryStructures(this);
	}
}