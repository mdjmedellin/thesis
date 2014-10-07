#include "ThesisTest.h"
#include "ProteinModel.h"
#include "SecondaryStructure.h"
#include "AminoAcid.h"
#include "LinkFragment.h"

//========================HydrogenBond========================
void HydrogenBond::Translate(const FVector& displacement)
{
	m_linkFragment->SetActorLocation(m_linkFragment->GetActorLocation() + displacement);
}

void HydrogenBond::RotateAboutSpecifiedPoint(const FRotationMatrix& rotationMatrix, const FVector& rotationPoint)
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

	/*
	FVector distanceFromPivotPoint = m_linkFragment->GetActorLocation() - rotationPoint;

	//rotate the distance
	distanceFromPivotPoint = rotation.RotateVector(distanceFromPivotPoint);
	//now set the new location to the fragment
	m_linkFragment->SetActorLocation(rotationPoint + distanceFromPivotPoint);

	//rotate the object
	FRotationMatrix currentRotationMatrix(m_linkFragment->GetActorRotation());
	FRotationMatrix newRotation(rotation);
	currentRotationMatrix *= newRotation;
	m_linkFragment->SetActorRotation(currentRotationMatrix.Rotator());
	*/
}

void HydrogenBond::ToggleShake()
{
	m_linkFragment->ToggleShake();
}

void HydrogenBond::ToggleBreaking()
{
	m_linkFragment->ToggleBreaking();
}
//============================================================

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
				HydrogenBond* hBond = m_proteinModel->SpawnHydrogenBond(residue, betaPartner);
				residue->AddHydrogenBond(hBond);
				betaPartner->AddHydrogenBond(hBond);

				//add bond to the array of hydrogen bonds in the beta sheet

			}
		}
	}
}
//==========================================================

SecondaryStructure* SecondaryStructure::s_selectedStructure = nullptr;

SecondaryStructure::SecondaryStructure(ESecondaryStructure::Type secondaryStructureType,
	GHProtein::ProteinModel* parentModel)
: m_secondaryStructureType(secondaryStructureType)
, m_nextSecondaryStructure(nullptr)
, m_headAminoAcid(nullptr)
, m_tailAminoAcid(nullptr)
, m_parentModel(parentModel)
{}

SecondaryStructure::~SecondaryStructure()
{}

/*
void SecondaryStructure::SetSelected()
{
	if (s_selectedStructure)
	{
		s_selectedStructure->Deselect();
	}

	s_selectedStructure = this;

	//perform other selected operations here, such as changing colors of the materials
	ChangeRibbonColor(FColor::Red);
}
*/

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

/*
void SecondaryStructure::Deselect()
{
	if (this == s_selectedStructure)
	{
		ResetRibbonColor();
	}
}


void SecondaryStructure::ResetRibbonColor()
{
	//iterate over all of the amino acids on this structure and set the color to the ribbon color
	for (AAminoAcid* currentResidue = m_headAminoAcid;
		currentResidue != nullptr && (m_tailAminoAcid && currentResidue != m_tailAminoAcid->GetNextAminoAcidPtr());
		currentResidue = currentResidue->GetNextAminoAcidPtr())
	{
		currentResidue->ResetLinkFragmentColorToDefault();
	}
}


void SecondaryStructure::ChangeRibbonColor(const FColor& ribbonColor)
{
	//iterate over all of the amino acids on this structure and set the color to the ribbon color
	for (AAminoAcid* currentResidue = m_headAminoAcid; 
		currentResidue != nullptr && (m_tailAminoAcid && currentResidue != m_tailAminoAcid->GetNextAminoAcidPtr());
		currentResidue = currentResidue->GetNextAminoAcidPtr())
	{
		currentResidue->SetLinkFragmentColor(ribbonColor);
	}
}
*/
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

void SecondaryStructure::AppendAminoAcid(AAminoAcid* residue)
{
	//Get the residue information
	const Residue* residueInfo = residue->GetResidueInformation();

	//if we do not have an amino acid, then we take the first amino acid as the head amino acid
	if (m_headAminoAcid == nullptr)
	{
		m_headAminoAcid = residue;
		//set the structure type from the head amino acid
		m_secondaryStructureType = residueInfo->GetSecondaryStructure();
	}
	else
	{
		//if we already have a head amino acid, then we take the latest appended amino acid as the tail
		m_tailAminoAcid = residue;
	}

	//if this is a beta strand, make sure to save the partners
	if (m_secondaryStructureType == ESecondaryStructure::ssStrand)
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
			HydrogenBond* newlyCreatedBond = m_parentModel->SpawnHydrogenBond(currentResidue, partnerResidue);
			m_hydrogenBonds.Add(newlyCreatedBond);
			currentResidue->AddHydrogenBond(newlyCreatedBond);
			partnerResidue->AddHydrogenBond(newlyCreatedBond);
		}
		else
		{
			//the first time we cannot find a partner indicates where we should stop spawning hydrogen bonds
			keepLooping = false;
		}
	}
}

void SecondaryStructure::BreakStructure()
{
	//iterate throguh the residues that make up this structure
	AAminoAcid* currentResidue = m_headAminoAcid;
	int counter = 0;
	TArray<AAminoAcid*> residues;
	bool keepIterating = true;

	while (keepIterating)
	{
		++counter;
		residues.Add(currentResidue);
		if (currentResidue != m_tailAminoAcid)
		{
			currentResidue = currentResidue->GetNextAminoAcidPtr();
		}
		else
		{
			keepIterating = false;
		}
		
	}

	for (int i = 0; i < 1; ++i)
	{
		TestLineFitting2(residues);

		for (int index = 0; index < residues.Num(); ++index)
		{
			currentResidue = residues[index];
			currentResidue->UpdateLinkToNextAminoAcid();
			//currentResidue->HideLinkFragment();
		}
	}

	//get the first and last position
	FVector startPosition = m_headAminoAcid->GetActorLocation();
	FVector endPosition = m_tailAminoAcid->GetActorLocation();

	/*
	//at first lets just put the residues in a straight line
	FVector distance = endPosition - startPosition;

	if (counter > 1)
	{
		distance /= (counter - 1);
	}

	for (int index = 0; index < residues.Num(); ++index)
	{
		currentResidue = residues[index];
		currentResidue->SetActorLocation(startPosition + (distance * index));
	}

	//update the link fragments for all of the residues in the struture
	for (int index = 0; index < residues.Num(); ++index)
	{
		currentResidue = residues[index];
		currentResidue->UpdateLinkToNextAminoAcid();
		currentResidue->HideLinkFragment();
	}
	*/
}

void SecondaryStructure::TestLineFitting2(TArray<AAminoAcid*>& residues)
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

	//calculate the possible independent variables
	//in other words the run value on the slope formula
	FVector possibleIndependentVariables = FVector::ZeroVector;
	for (int i = 0; i < 3; ++i)
	{
		possibleIndependentVariables[i] = sumsSquared[i] - (sums[i] * means[i]);
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
				possibleSlopes[i][j] = (multiplicativeSums[i][j] - sums[i] * means[j]);
			}
		}
	}

	double currentMin = possibleIndependentVariables[0];
	axisToUse = 0;
	for (int i = 1; i < 3 && currentMin == 0.0; ++i)
	{
		currentMin = possibleIndependentVariables[i];
		axisToUse = i;
	}

	//check which variable we are going to use as the independent variable
	for (int i = 0; i < 3; ++i)
	{
		if (possibleIndependentVariables[i] != 0.0
			&& abs(currentMin) < abs(possibleIndependentVariables[i]))
		{
			//we can use this variable as the independent variable
			currentMin = possibleIndependentVariables[i];
			axisToUse = i;
		}
	}

	//finish constructing the slopes
	for (int i = 0; i < 3; ++i)
	{
		if (i != axisToUse)
		{
			possibleSlopes[axisToUse][i] /= possibleIndependentVariables[axisToUse];

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

	FVector endToStart = startLocation - endLocation;
	endToStart.Normalize();

	//randomly move all of the residues that conform the alpha helix
	float  distanceBetweenIncrements = distanceStartToEnd / (num - 1);
	FVector prevLocation = startLocation;
	for (int i = 1; i < num-1; ++i)
	{
		prevLocation = startLocation + (i * distanceBetweenIncrements * startToEnd);
		residues[i]->SetActorLocation(prevLocation);
	}

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

	//now we should have three vectors perpendicular to each other
	float testDot = FVector::DotProduct(baseVector, startToEnd);
	testDot = FVector::DotProduct(baseVector2, startToEnd);
	testDot = FVector::DotProduct(baseVector, baseVector2);

	/*
	//render to see if they are perpendicular to each other
	DrawDebugLine(
		m_parentModel->GetWorld(),
		startLocation,
		endLocation,
		FColor(255, 0, 0),
		true, -1, 0,
		12
		);

	endLocation = startLocation + 120 * baseVector;
	DrawDebugLine(
		m_parentModel->GetWorld(),
		startLocation,
		endLocation,
		FColor(0, 255, 0),
		true, -1, 0,
		12
		);

	endLocation = startLocation + 120 * baseVector2;
	DrawDebugLine(
		m_parentModel->GetWorld(),
		startLocation,
		endLocation,
		FColor(0, 0, 255),
		true, -1, 0,
		12
		);
	*/
	//end

	//use sin function to shake the residues away from each other
	for (int i = 1; i < num-1; ++i)
	{
		float rads = (PI * 0.5 * i) + (.5 * PI);
		float sinVal = sinf(rads);

		actorLocation = residues[i]->GetActorLocation() + sinVal * longestDistance * baseVector;
		residues[i]->SetActorLocation(actorLocation);

		//see how much we should rotate the direction of the vector
	}
}

//this does not work
void SecondaryStructure::TestLineFitting(TArray<AAminoAcid*>& residues)
{
	int num = residues.Num();
	double inverseNum = 1.0 / num;
	double oneThird = 1.0 / 3.0;

	double x_m = 0.0;
	double y_m = 0.0;
	double z_m = 0.0;
	double sum_x_squared_m = 0.0;
	double sum_y_squared_m = 0.0;
	double sum_z_squared_m = 0.0;
	double sum_xy = 0.0;
	double sum_xz = 0.0;
	double sum_yz = 0.0;

	FVector location;
	for (int i = 0; i < residues.Num(); ++i)
	{
		location = residues[i]->GetActorLocation();

		x_m += location.X;
		y_m += location.Y;
		z_m += location.Z;

		sum_x_squared_m += (location.X * location.X);
		sum_y_squared_m += (location.Y * location.Y);
		sum_z_squared_m += (location.Z * location.Z);

		sum_xy += (location.X * location.Y);
		sum_xz += (location.X * location.Z);
		sum_yz += (location.Y * location.Z);
	}

	x_m *= inverseNum;
	y_m *= inverseNum;
	z_m *= inverseNum;

	sum_x_squared_m *= inverseNum;
	sum_y_squared_m *= inverseNum;
	sum_z_squared_m *= inverseNum;

	sum_xy *= inverseNum;
	sum_xz *= inverseNum;
	sum_yz *= inverseNum;

	double s_xx = 0.0;
	double s_yy = 0.0;
	double s_zz = 0.0;
	
	s_xx = -(x_m * x_m) + (sum_x_squared_m);
	s_yy = -(y_m * y_m) + (sum_y_squared_m);
	s_zz = -(z_m * z_m) + (sum_z_squared_m);

	double s_xz = 0.0;
	double s_xy = 0.0;
	double s_yz = 0.0;

	s_xy = -(x_m * y_m) + (sum_xy);
	s_xz = -(x_m * z_m) + (sum_xz);
	s_yz = -(y_m * z_m) + (sum_yz);

	double theta = 0.0;

	theta = FMath::Atan2((s_xx - s_yy), (2.0 * s_xy)) * 0.5;

	double cosTheta = cos(theta);
	double sinTheta = sin(theta);
	double cosThetaSquared = cosTheta * cosTheta;
	double sinThetaSquared = sinTheta * sinTheta;

	double k11 = ((s_yy + s_zz) * cosThetaSquared) + ((s_xx + s_zz) * sinThetaSquared) - (2.0 * s_xy * cosTheta * sinTheta);
	double k22 = ((s_yy + s_zz) * sinThetaSquared) + ((s_xx + s_zz) * cosThetaSquared) + (2.0 * s_xy * cosTheta * sinTheta);
	double k12 = ((-s_xy) * (cosThetaSquared - sinThetaSquared)) + ((s_xx - s_yy) * cosTheta * sinTheta);
	double k10 = (s_xz * cosTheta) + (s_yz * sinTheta);
	double k01 = (-s_xz * sinTheta) + (s_yz * cosTheta);
	double k00 = s_xx + s_yy;

	double c2 = -k00 - k11 - k22;
	double c1 = (k00 * k11) + (k00 * k22) + (k11*k22) - (k01 * k01) - (k10 * k10);
	double c0 = ((k01 * k01) * k11) + ((k10 * k10) * k22) - (k00*k11*k22);

	double p = c1 - (oneThird * c2 * c2);
	double q = ((2.0 / 27.0) * c2 *c2 * c2) - (oneThird * c1 * c2) + c0;
	double R = (0.25 * q * q) + ((1.0 / 27.0) * p * p * p);

	double phetaSquared = 0.0;

	if (R > 0.0)
	{
		double sqrtR = sqrt(R);
		double term1 = (-oneThird) * c2;
		double innerTest = -0.5 * q;
		innerTest += sqrtR;
		double term2 = cbrt((-0.5 * q) + sqrtR);
		double term3 = cbrt((-0.5 * q) - sqrtR);

		phetaSquared = term1 + term2 + term3;
	}
	else
	{
		double kappa = sqrt((-1.0 / 27.0) * p * p * p);
		double gamma = acos(-q / (2.0 * kappa));

		double cubicRootKappa = cbrt(kappa);

		FVector values = FVector::ZeroVector;
		values.X = ((-oneThird) * c2) + (2.0 * cubicRootKappa * cos((oneThird)*gamma));
		values.Y = ((-oneThird) * c2) + (2.0 * cubicRootKappa * cos((oneThird)*(gamma + (2.0 * PI))));
		values.Z = ((-oneThird) * c2) + (2.0 * cubicRootKappa * cos((oneThird)*(gamma + (4.0 * PI))));

		phetaSquared = values.GetMin();
	}

	double division1 = (k10 / (k11 - phetaSquared));
	double division2 = (k01 / (k22 - phetaSquared));

	double a = (-division1 * cosTheta) + (division2 * sinTheta);
	double b = (-division1 * sinTheta) + (-division2 * cosTheta);
	double a_squared = a * a;
	double b_squared = b * b;

	double division3 = ((1.0) / (1.0 + a_squared + b_squared));

	double u = division3 * (((1.0 + b_squared) * x_m) - (a*b*y_m) + (a*z_m));
	double v = division3 * ((-a*b*x_m) + ((1.0 + a_squared) * y_m) + (b*z_m));
	double w = division3 * ((a * x_m) + (b*y_m) + ((a_squared + b_squared)*z_m));

	FVector LinkStart(x_m, y_m, z_m);
	FVector LinkEnd(u, v, w);
	
	DrawDebugLine(
		m_parentModel->GetWorld(),
		LinkStart,
		LinkEnd,
		FColor(255, 0, 0),
		true, -1, 0,
		12
		);

	DrawDebugSphere(
		m_parentModel->GetWorld(),
		LinkStart,
		10,
		20,
		FColor(0,255,0),
		true,
		-1.f,
		0
		);

	DrawDebugSphere(
		m_parentModel->GetWorld(),
		LinkEnd,
		10,
		20,
		FColor(0, 255, 0),
		true,
		-1.f,
		0
		);

	FVector test1 = residues[0]->GetActorLocation();
	float z = a * test1.X + b * test1.Y;
	test1.Z = z;

	DrawDebugSphere(
		m_parentModel->GetWorld(),
		test1,
		10,
		20,
		FColor(0, 255, 0),
		true,
		-1.f,
		0
		);

	float test2 = (phetaSquared * phetaSquared * phetaSquared) + c2 * phetaSquared * phetaSquared + c1 * phetaSquared + c0;
}