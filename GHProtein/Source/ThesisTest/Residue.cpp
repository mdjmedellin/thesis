#include "ThesisTest.h"
#include "Residue.h"
#include "ProteinUtilities.h"
#include <assert.h>

const ResidueInfo kResidueInfo[] = {
	{ EResidueType::kUnknownResidue, 'X', "UNK" },
	{ EResidueType::kAlanine, 'A', "ALA" },
	{ EResidueType::kArginine, 'R', "ARG" },
	{ EResidueType::kAsparagine, 'N', "ASN" },
	{ EResidueType::kAsparticAcid, 'D', "ASP" },
	{ EResidueType::kCysteine, 'C', "CYS" },
	{ EResidueType::kGlutamicAcid, 'E', "GLU" },
	{ EResidueType::kGlutamine, 'Q', "GLN" },
	{ EResidueType::kGlycine, 'G', "GLY" },
	{ EResidueType::kHistidine, 'H', "HIS" },
	{ EResidueType::kIsoleucine, 'I', "ILE" },
	{ EResidueType::kLeucine, 'L', "LEU" },
	{ EResidueType::kLysine, 'K', "LYS" },
	{ EResidueType::kMethionine, 'M', "MET" },
	{ EResidueType::kPhenylalanine, 'F', "PHE" },
	{ EResidueType::kProline, 'P', "PRO" },
	{ EResidueType::kSerine, 'S', "SER" },
	{ EResidueType::kThreonine, 'T', "THR" },
	{ EResidueType::kTryptophan, 'W', "TRP" },
	{ EResidueType::kTyrosine, 'Y', "TYR" },
	{ EResidueType::kValine, 'V', "VAL" }
};

const char SecondaryStructureInfo[] = {
	' '
	, 'H'
	, 'B'
	, 'E'
	, 'G'
	, 'I'
	, 'T'
	, 'S'
};

EResidueType::Type MapResidue( FString inName)
{
	inName.Trim();
	inName.TrimTrailing();

	EResidueType::Type result = EResidueType::kUnknownResidue;

	for (uint32 i = 0; i < EResidueType::kResidueTypeCount; ++i)
	{
		if (inName == kResidueInfo[i].name)
		{
			result = kResidueInfo[i].type;
			break;
		}
	}

	return result;
}

EResidueType::Type MapResidue(char inCode)
{
	EResidueType::Type result = EResidueType::kUnknownResidue;

	for (uint32 i = 0; i < EResidueType::kResidueTypeCount; ++i)
	{
		if (inCode == kResidueInfo[i].code)
		{
			result = kResidueInfo[i].type;
			break;
		}
	}

	return result;
}

ESecondaryStructure::Type MapSecondaryStructure(char inSSCharLabel)
{
	ESecondaryStructure::Type result = ESecondaryStructure::ssLoop;

	for (int i = 0; i < ESecondaryStructure::ssCount; ++i)
	{
		if (inSSCharLabel == SecondaryStructureInfo[i])
		{
			result = ESecondaryStructure::Type( result + i );
			break;
		}
	}

	return result;
}

//Residue::Residue(uint32 inNumber, Residue* inPrevious, const TArray<Atom>& inAtoms)
//	: mChainID(0)
//	, mPrev(inPrevious)
//	, mNext(nullptr)
//	, mSeqNumber(inAtoms.front().mResSeq)
//	, mNumber(inNumber)
//	, mInsertionCode(inAtoms.front().mICode)
//	, mType(MapResidue(inAtoms.front().mResName))
//	, mSSBridgeNr(0)
//	, mAccessibility(0)
//	, mSecondaryStructure(loop)
//	, mSheet(0)
//{
//	if (mPrev != nullptr)
//		mPrev->mNext = this;
//
//	fill(mHelixFlags, mHelixFlags + 3, helixNone);
//
//	mBetaPartner[0].residue = mBetaPartner[1].residue = nullptr;
//
//	mHBondDonor[0].energy = mHBondDonor[1].energy = mHBondAcceptor[0].energy = mHBondAcceptor[1].energy = 0;
//	mHBondDonor[0].residue = mHBondDonor[1].residue = mHBondAcceptor[0].residue = mHBondAcceptor[1].residue = nullptr;
//
//	static const MAtom kNullAtom = {};
//	mN = mCA = mC = mO = kNullAtom;
//
//	foreach(const MAtom& atom, inAtoms)
//	{
//		if (mChainID == 0)
//			mChainID = atom.mChainID;
//
//		if (MapResidue(atom.mResName) != mType)
//			throw mas_exception(
//			boost::format("inconsistent residue types in atom records for residue %1% (%2% != %3%)")
//			% inNumber % atom.mResName % inAtoms.front().mResName);
//
//		if (atom.mResSeq != mSeqNumber)
//			throw mas_exception(boost::format("inconsistent residue sequence numbers (%1% != %2%)") % atom.mResSeq % mSeqNumber);
//
//		if (atom.GetName() == " N  ")
//			mN = atom;
//		else if (atom.GetName() == " CA ")
//			mCA = atom;
//		else if (atom.GetName() == " C  ")
//			mC = atom;
//		else if (atom.GetName() == " O  ")
//			mO = atom;
//		else
//			mSideChain.push_back(atom);
//	}
//
//	// assign the Hydrogen
//	mH = GetN();
//
//	if (mType != kProline and mPrev != nullptr)
//	{
//		const MAtom& pc = mPrev->GetC();
//		const MAtom& po = mPrev->GetO();
//
//		double CODistance = Distance(pc, po);
//
//		mH.mLoc.mX += (pc.mLoc.mX - po.mLoc.mX) / CODistance;
//		mH.mLoc.mY += (pc.mLoc.mY - po.mLoc.mY) / CODistance;
//		mH.mLoc.mZ += (pc.mLoc.mZ - po.mLoc.mZ) / CODistance;
//	}
//
//	// update the box containing all atoms
//	mBox[0].mX = mBox[0].mY = mBox[0].mZ = numeric_limits<double>::max();
//	mBox[1].mX = mBox[1].mY = mBox[1].mZ = -numeric_limits<double>::max();
//
//	ExtendBox(mN, kRadiusN + 2 * kRadiusWater);
//	ExtendBox(mCA, kRadiusCA + 2 * kRadiusWater);
//	ExtendBox(mC, kRadiusC + 2 * kRadiusWater);
//	ExtendBox(mO, kRadiusO + 2 * kRadiusWater);
//	foreach(const MAtom& atom, mSideChain)
//		ExtendBox(atom, kRadiusSideAtom + 2 * kRadiusWater);
//
//	mRadius = mBox[1].mX - mBox[0].mX;
//	if (mRadius < mBox[1].mY - mBox[0].mY)
//		mRadius = mBox[1].mY - mBox[0].mY;
//	if (mRadius < mBox[1].mZ - mBox[0].mZ)
//		mRadius = mBox[1].mZ - mBox[0].mZ;
//
//	mCenter.mX = (mBox[0].mX + mBox[1].mX) / 2;
//	mCenter.mY = (mBox[0].mY + mBox[1].mY) / 2;
//	mCenter.mZ = (mBox[0].mZ + mBox[1].mZ) / 2;
//
//	if (VERBOSE > 3)
//		cerr << "Created residue " << mN.mResName << endl;
//}

//Residue::Residue(uint32 inNumber, char inTypeCode, Residue* inPrevious)
//	: mChainID(0)
//	, mPrev(nullptr)
//	, mNext(nullptr)
//	, mSeqNumber(inNumber)
//	, mNumber(inNumber)
//	, mInsertionCode(' ')
//	, mType(MapResidue(inTypeCode))
//	, mSSBridgeNr(0)
//	, mAccessibility(0)
//	, mSecondaryStructure(ESecondaryStructure::ssLoop)
//	, mSheet(0)
//	, mBend(false)
//{
//	fill(mHelixFlags, mHelixFlags + 3, EHelixFlag::helixNone);
//
//	mBetaPartner[0].residue = mBetaPartner[1].residue = nullptr;
//
//	mHBondDonor[0].energy = mHBondDonor[1].energy = mHBondAcceptor[0].energy = mHBondAcceptor[1].energy = 0;
//	mHBondDonor[0].residue = mHBondDonor[1].residue = mHBondAcceptor[0].residue = mHBondAcceptor[1].residue = nullptr;
//
//	static const Atom kNullAtom = {};
//	mN = mCA = mC = mO = kNullAtom;
//
//	mCA.mICode = ' ';
//	mCA.mResSeq = inTypeCode;
//	mCA.mChainID = 'A';
//}

//Residue::Residue(const Residue& residue)
//	: mChainID(residue.mChainID)
//	, mPrev(nullptr)
//	, mNext(nullptr)
//	, mSeqNumber(residue.mSeqNumber)
//	, mNumber(residue.mNumber)
//	, mType(residue.mType)
//	, mSSBridgeNr(residue.mSSBridgeNr)
//	, mAccessibility(residue.mAccessibility)
//	, mSecondaryStructure(residue.mSecondaryStructure)
//	, mC(residue.mC)
//	, mN(residue.mN)
//	, mCA(residue.mCA)
//	, mO(residue.mO)
//	, mH(residue.mH)
//	, mSideChain(residue.mSideChain)
//	, mSheet(residue.mSheet)
//	, mBend(residue.mBend)
//	, mCenter(residue.mCenter)
//	, mRadius(residue.mRadius)
//{
//	copy(residue.mHBondDonor, residue.mHBondDonor + 2, mHBondDonor);
//	copy(residue.mHBondAcceptor, residue.mHBondAcceptor + 2, mHBondAcceptor);
//	copy(residue.mBetaPartner, residue.mBetaPartner + 2, mBetaPartner);
//	copy(residue.mHelixFlags, residue.mHelixFlags + 3, mHelixFlags);
//	copy(residue.mBox, residue.mBox + 2, mBox);
//}

Residue::Residue()
: mSheetLabel(' ')
, mChainID(' ')
, mPrev(nullptr)
, mNext(nullptr)
, mSeqNumber(-1)
, mNumber(-1)
, mInsertionCode(' ')
, mType(EResidueType::kUnknownResidue)
, mSSBridgeNr(0)
, mAccessibility(0.0)
, mSecondaryStructure(ESecondaryStructure::ssLoop)
, mSheet(0)
, mBend(false)
{
	//fill the array of helix flags
	for (int i = 0; i < 3; ++i)
	{
		mHelixFlags[i] = EHelixFlag::helixNone;
	}
	
	static const Atom kNullAtom = {};
	mN = mCA = mC = mO = kNullAtom;

	mCA.mICode = ' ';
	mCA.mResSeq = -1;
	mCA.mChainID = 'A';

	mBetaPartner[0].residue = mBetaPartner[1].residue = nullptr;
}

void Residue::SetSeqNumber(uint16 inSeqNumber)
{
	mSeqNumber = inSeqNumber;
	mCA.mResSeq = inSeqNumber;
}

void Residue::SetInsertionCode(char inICode)
{
	mInsertionCode = inICode;
	mCA.mICode = inICode;
}

void Residue::SetChainID(char inID)
{
	mChainID = inID;

	mC.SetChainID(inID);
	mCA.SetChainID(inID);
	mO.SetChainID(inID);
	mN.SetChainID(inID);
	mH.SetChainID(inID);

	/*for (int index = 0; index < mSideChain.Num(); ++index)
	{
		mSideChain[index].SetChainID(inID);
	}*/
}

void Residue::SetType(char inResidueType)
{
	if (inResidueType > 'Z' || inResidueType < 'A')
	{
		mType = EResidueType::kCysteine;
		//SetSSBridgeNr(inResidueType);
	}
	else
	{
		mType = MapResidue(inResidueType);
	}
}

void Residue::SetHelixFlags(const char* inHelixFlags)
{
	for (int index = 0; index < 3; ++index)
	{
		switch (inHelixFlags[index])
		{
		case ' ':
			SetHelixFlag(index + 3, EHelixFlag::helixNone);
			break;
		case '>':
			SetHelixFlag(index + 3, EHelixFlag::helixStart);
			break;
		case '<':
			SetHelixFlag(index + 3, EHelixFlag::helixEnd);
			break;
		case 'X':
			SetHelixFlag(index + 3, EHelixFlag::helixStartAndEnd);
			break;
		default:
			if (inHelixFlags[index] <= '9' && inHelixFlags[index] >= '0')
			{
				SetHelixFlag(index + 3, EHelixFlag::helixMiddle);
			}
			break;
		}
	}
}

void Residue::SetSecondaryStructure(char inSSCharLabel)
{
	mSecondaryStructure = MapSecondaryStructure(inSSCharLabel);
}

void Residue::SetBridgeLabels(char* inBridgeLabels)
{
	//JM
	//TODO:
	//This needs to be revisited because we cannot extract all teh information about the
	//beta partners at this moment
	BridgePartner* currentBridgePartner;

	for (int i = 0; i < 2; ++i)
	{
		currentBridgePartner = &mBetaPartner[i];

		if (inBridgeLabels[i] == tolower(inBridgeLabels[i]))
		{
			currentBridgePartner->parallel = true;
		}
		else
		{
			currentBridgePartner->parallel = false;
		}

		currentBridgePartner->ladder = inBridgeLabels[i];
	}

	//NOTE
	//THIS IS HOW THE BRIDGE PARTNER LABELS ARE ASSIGNED
	//if (VERBOSE)
	//	cerr << "Calculate beta sheets" << endl;

	//// Calculate Bridges
	//vector<MBridge> bridges;
	//if (inResidues.size() > 4)
	//{
	//	for (uint32 i = 1; i + 4 < inResidues.size(); ++i)
	//	{
	//		MResidue* ri = inResidues[i];

	//		for (uint32 j = i + 3; j + 1 < inResidues.size(); ++j)
	//		{
	//			MResidue* rj = inResidues[j];

	//			MBridgeType type = ri->TestBridge(rj);
	//			if (type == btNoBridge)
	//				continue;

	//			bool found = false;
	//			foreach(MBridge& bridge, bridges)
	//			{
	//				if (type != bridge.type or i != bridge.i.back() + 1)
	//					continue;

	//				if (type == btParallel and bridge.j.back() + 1 == j)
	//				{
	//					bridge.i.push_back(i);
	//					bridge.j.push_back(j);
	//					found = true;
	//					break;
	//				}

	//				if (type == btAntiParallel and bridge.j.front() - 1 == j)
	//				{
	//					bridge.i.push_back(i);
	//					bridge.j.push_front(j);
	//					found = true;
	//					break;
	//				}
	//			}

	//			if (not found)
	//			{
	//				MBridge bridge = {};

	//				bridge.type = type;
	//				bridge.i.push_back(i);
	//				bridge.chainI = ri->GetChainID();
	//				bridge.j.push_back(j);
	//				bridge.chainJ = rj->GetChainID();

	//				bridges.push_back(bridge);
	//			}
	//		}
	//	}
	//}

	//// extend ladders
	//sort(bridges.begin(), bridges.end());

	//for (uint32 i = 0; i < bridges.size(); ++i)
	//{
	//	for (uint32 j = i + 1; j < bridges.size(); ++j)
	//	{
	//		uint32 ibi = bridges[i].i.front();
	//		uint32 iei = bridges[i].i.back();
	//		uint32 jbi = bridges[i].j.front();
	//		uint32 jei = bridges[i].j.back();
	//		uint32 ibj = bridges[j].i.front();
	//		uint32 iej = bridges[j].i.back();
	//		uint32 jbj = bridges[j].j.front();
	//		uint32 jej = bridges[j].j.back();

	//		if (bridges[i].type != bridges[j].type or
	//			MResidue::NoChainBreak(inResidues[min(ibi, ibj)], inResidues[max(iei, iej)]) == false or
	//			MResidue::NoChainBreak(inResidues[min(jbi, jbj)], inResidues[max(jei, jej)]) == false or
	//			ibj - iei >= 6 or
	//			(iei >= ibj and ibi <= iej))
	//		{
	//			continue;
	//		}

	//		bool bulge;
	//		if (bridges[i].type == btParallel)
	//			bulge = ((jbj - jei < 6 and ibj - iei < 3) or(jbj - jei < 3));
	//		else
	//			bulge = ((jbi - jej < 6 and ibj - iei < 3) or(jbi - jej < 3));

	//		if (bulge)
	//		{
	//			bridges[i].i.insert(bridges[i].i.end(), bridges[j].i.begin(), bridges[j].i.end());
	//			if (bridges[i].type == btParallel)
	//				bridges[i].j.insert(bridges[i].j.end(), bridges[j].j.begin(), bridges[j].j.end());
	//			else
	//				bridges[i].j.insert(bridges[i].j.begin(), bridges[j].j.begin(), bridges[j].j.end());
	//			bridges.erase(bridges.begin() + j);
	//			--j;
	//		}
	//	}
	//}

	//// Sheet
	//set<MBridge*> ladderset;
	//foreach(MBridge& bridge, bridges)
	//{
	//	ladderset.insert(&bridge);

	//	uint32 n = bridge.i.size();
	//	if (n > kHistogramSize)
	//		n = kHistogramSize;

	//	if (bridge.type == btParallel)
	//		mParallelBridgesPerLadderHistogram[n - 1] += 1;
	//	else
	//		mAntiparallelBridgesPerLadderHistogram[n - 1] += 1;
	//}

	//uint32 sheet = 1, ladder = 0;
	//while (not ladderset.empty())
	//{
	//	set<MBridge*> sheetset;
	//	sheetset.insert(*ladderset.begin());
	//	ladderset.erase(ladderset.begin());

	//	bool done = false;
	//	while (not done)
	//	{
	//		done = true;
	//		foreach(MBridge* a, sheetset)
	//		{
	//			foreach(MBridge* b, ladderset)
	//			{
	//				if (Linked(*a, *b))
	//				{
	//					sheetset.insert(b);
	//					ladderset.erase(b);
	//					done = false;
	//					break;
	//				}
	//			}
	//			if (not done)
	//				break;
	//		}
	//	}

	//	foreach(MBridge* bridge, sheetset)
	//	{
	//		bridge->ladder = ladder;
	//		bridge->sheet = sheet;
	//		bridge->link = sheetset;

	//		++ladder;
	//	}

	//	uint32 nrOfLaddersPerSheet = sheetset.size();
	//	if (nrOfLaddersPerSheet > kHistogramSize)
	//		nrOfLaddersPerSheet = kHistogramSize;
	//	if (nrOfLaddersPerSheet == 1 and(*sheetset.begin())->i.size() > 1)
	//		mLaddersPerSheetHistogram[0] += 1;
	//	else if (nrOfLaddersPerSheet > 1)
	//		mLaddersPerSheetHistogram[nrOfLaddersPerSheet - 1] += 1;

	//	++sheet;
	//}

	//foreach(MBridge& bridge, bridges)
	//{
	//	// find out if any of the i and j set members already have
	//	// a bridge assigned, if so, we're assigning bridge 2

	//	uint32 betai = 0, betaj = 0;

	//	foreach(uint32 l, bridge.i)
	//	{
	//		if (inResidues[l]->GetBetaPartner(0).residue != nullptr)
	//		{
	//			betai = 1;
	//			break;
	//		}
	//	}

	//	foreach(uint32 l, bridge.j)
	//	{
	//		if (inResidues[l]->GetBetaPartner(0).residue != nullptr)
	//		{
	//			betaj = 1;
	//			break;
	//		}
	//	}

	//	MSecondaryStructure ss = betabridge;
	//	if (bridge.i.size() > 1)
	//		ss = strand;

	//	if (bridge.type == btParallel)
	//	{
	//		mNrOfHBondsInParallelBridges += bridge.i.back() - bridge.i.front() + 2;

	//		deque<uint32>::iterator j = bridge.j.begin();
	//		foreach(uint32 i, bridge.i)
	//			inResidues[i]->SetBetaPartner(betai, inResidues[*j++], bridge.ladder, true);

	//		j = bridge.i.begin();
	//		foreach(uint32 i, bridge.j)
	//			inResidues[i]->SetBetaPartner(betaj, inResidues[*j++], bridge.ladder, true);
	//	}
	//	else
	//	{
	//		mNrOfHBondsInAntiparallelBridges += bridge.i.back() - bridge.i.front() + 2;

	//		deque<uint32>::reverse_iterator j = bridge.j.rbegin();
	//		foreach(uint32 i, bridge.i)
	//			inResidues[i]->SetBetaPartner(betai, inResidues[*j++], bridge.ladder, false);

	//		j = bridge.i.rbegin();
	//		foreach(uint32 i, bridge.j)
	//			inResidues[i]->SetBetaPartner(betaj, inResidues[*j++], bridge.ladder, false);
	//	}

	//	for (uint32 i = bridge.i.front(); i <= bridge.i.back(); ++i)
	//	{
	//		if (inResidues[i]->GetSecondaryStructure() != strand)
	//			inResidues[i]->SetSecondaryStructure(ss);
	//		inResidues[i]->SetSheet(bridge.sheet);
	//	}

	//	for (uint32 i = bridge.j.front(); i <= bridge.j.back(); ++i)
	//	{
	//		if (inResidues[i]->GetSecondaryStructure() != strand)
	//			inResidues[i]->SetSecondaryStructure(ss);
	//		inResidues[i]->SetSheet(bridge.sheet);
	//	}
	//}
}

void Residue::SetBridgePartnersNumber(int* inBridgePartnersNumber)
{
	/*BridgePartner* currentBridgePartner;

	for (int i = 0; i < 2; ++i)
	{
		currentBridgePartner = &mBetaPartner[i];

		currenBridgePartner.number = inBridgePartnersNumber[i];
	}*/
}

void Residue::SetNHO(FString* inNHO)
{
	//JM
	//Setting NHO is a little more convoluted than the other properties of the residue
	//in order to set them right we need to wait until all the residues have been inserted
	//into the protein. This is because the number that is given by the NHO is the distance
	//between this residue and the one it is bonded with

	//Here is how it is calculated
	/*string NHO[2], ONH[2];
	const HBond* acceptors = residue.Acceptor();
	const HBond* donors = residue.Donor();
	for (uint32 i = 0; i < 2; ++i)
	{
		NHO[i] = ONH[i] = "0, 0.0";

		if (acceptors[i].residue != nullptr)
		{
			int32 d = acceptors[i].residue->GetNumber() - residue.GetNumber();
			NHO[i] = (boost::format("%d,%3.1f") % d % acceptors[i].energy).str();
		}

		if (donors[i].residue != nullptr)
		{
			int32 d = donors[i].residue->GetNumber() - residue.GetNumber();
			ONH[i] = (boost::format("%d,%3.1f") % d % donors[i].energy).str();
		}
	}*/

	//containers for the separate strings constructed from NHO
	FString RightString;
	FString LeftString;

	int tempInt = 0;
	float tempFloat = 0.f;

	for(int i = 0; i < 2; ++i)
	{
		inNHO[i].Split(",", &LeftString, &RightString);
		mHBondAcceptor[i].residue = new Residue();

		GHProtein::GetTypeFromString(tempInt, LeftString);
		mHBondAcceptor[i].residue->SetNumber(tempInt);

		GHProtein::GetTypeFromString(tempFloat, RightString);
		mHBondAcceptor[i].energy = tempFloat;
	}
}

void Residue::SetONH(FString* inONH)
{
	//containers for the separate strings constructed from NHO
	/*FString RightString;
	FString LeftString;

	for (int i = 0; i < 2; ++i)
	{
		inONH[i].Split(",", &LeftString, &RightString);
		mDonors[i].mNumber = TTypeFromString<int>(LeftString);
		mDonors[i].mEnergy = TTypeFromString<float>(RightString);
	}*/

	//JM
	//As in the SetNHO function, in order to set this property we need to wait until all
	//the residues have been read. The reason for teh wait is that the id of the donator
	//is given as a stride/distance of the current residue
	//containers for the separate strings constructed from NHO
	FString RightString;
	FString LeftString;

	int tempInt = 0;
	float tempFloat = 0.f;

	for(int i = 0; i < 2; ++i)
	{
		inONH[i].Split(",", &LeftString, &RightString);
		mHBondDonor[i].residue = new Residue();

		GHProtein::GetTypeFromString(tempInt, LeftString);
		mHBondDonor[i].residue->SetNumber(tempInt);

		GHProtein::GetTypeFromString(tempFloat, RightString);
		mHBondDonor[i].energy = tempFloat;
	}
}

void Residue::SetPrev(Residue* inResidue)
{
	mPrev = inResidue;
	mPrev->mNext = this;
}

bool Residue::NoChainBreak(const Residue* from, const Residue* to)
{
	bool result = true;
	for (const Residue* r = from; result && r != to; r = r->mNext)
	{
		Residue* next = r->mNext;
		if (next == nullptr)
			result = false;
		else
			result = next->mNumber == r->mNumber + 1;
	}
	return result;
}

void Residue::SetBetaPartners(int* betaPartners)
{
	for (int i = 0; i < 2; ++i)
	{
		mBetaPartner[i].number = betaPartners[i];
	}
}

void Residue::SetBetaPartnerResidue(int betaPartnerID, Residue* betaPartnerResidue)
{
	if (mBetaPartner[betaPartnerID].residue == nullptr)
	{
		mBetaPartner[betaPartnerID].residue = betaPartnerResidue;
	}
	else
	{
		//this presents a strange case, what if we are trying to replace the beta partner residue
		//if not taken care of, this could lead to a very problematic dangling pointer
	}
}

//this function checks to see if this residue is a beta partner to the passed in residue
bool Residue::IsBetaPartnerOf(Residue* partnerResidue)
{
	for (int i = 0; i < 2; ++i)
	{
		if (mBetaPartner[i].number == partnerResidue->GetSeqNumber())
		{
			return true;
		}
	}

	return false;
}

bool Residue::ValidDistance(const Residue& inNext) const
{
	return FVector::DistSquared(GetC(),inNext.GetN()) <= kMaxPeptideBondLength * kMaxPeptideBondLength;
}

//bool MResidue::TestBond(const MResidue* other) const
//{
//	return
//		(mHBondAcceptor[0].residue == other and mHBondAcceptor[0].energy < kMaxHBondEnergy) or
//		(mHBondAcceptor[1].residue == other and mHBondAcceptor[1].energy < kMaxHBondEnergy);
//}
//
//double MResidue::Phi() const
//{
//	double result = 360;
//	if (mPrev != nullptr and NoChainBreak(mPrev, this))
//		result = DihedralAngle(mPrev->GetC(), GetN(), GetCAlpha(), GetC());
//	return result;
//}
//
//double MResidue::Psi() const
//{
//	double result = 360;
//	if (mNext != nullptr and NoChainBreak(this, mNext))
//		result = DihedralAngle(GetN(), GetCAlpha(), GetC(), mNext->GetN());
//	return result;
//}
//
//tr1::tuple<double, char> MResidue::Alpha() const
//{
//	double alhpa = 360;
//	char chirality = ' ';
//
//	const MResidue* nextNext = mNext ? mNext->Next() : nullptr;
//	if (mPrev != nullptr and nextNext != nullptr and NoChainBreak(mPrev, nextNext))
//	{
//		alhpa = DihedralAngle(mPrev->GetCAlpha(), GetCAlpha(), mNext->GetCAlpha(), nextNext->GetCAlpha());
//		if (alhpa < 0)
//			chirality = '-';
//		else
//			chirality = '+';
//	}
//	return tr1::make_tuple(alhpa, chirality);
//}
//
//double MResidue::Kappa() const
//{
//	double result = 360;
//	const MResidue* prevPrev = mPrev ? mPrev->Prev() : nullptr;
//	const MResidue* nextNext = mNext ? mNext->Next() : nullptr;
//	if (prevPrev != nullptr and nextNext != nullptr and NoChainBreak(prevPrev, nextNext))
//	{
//		double ckap = CosinusAngle(GetCAlpha(), prevPrev->GetCAlpha(), nextNext->GetCAlpha(), GetCAlpha());
//		double skap = sqrt(1 - ckap * ckap);
//		result = atan2(skap, ckap) * 180 / kPI;
//	}
//	return result;
//}
//
//double MResidue::TCO() const
//{
//	double result = 0;
//	if (mPrev != nullptr and NoChainBreak(mPrev, this))
//		result = CosinusAngle(GetC(), GetO(), mPrev->GetC(), mPrev->GetO());
//	return result;
//}
//
//void MResidue::SetBetaPartner(uint32 n,
//	MResidue* inResidue, uint32 inLadder, bool inParallel)
//{
//	assert(n == 0 or n == 1);
//
//	mBetaPartner[n].residue = inResidue;
//	mBetaPartner[n].ladder = inLadder;
//	mBetaPartner[n].parallel = inParallel;
//}
//
BridgePartner Residue::GetBetaPartner(uint32 n) const
{
	assert(n == 0 or n == 1);
	return mBetaPartner[n];
}
//
//MHelixFlag MResidue::GetHelixFlag(uint32 inHelixStride) const
//{
//	assert(inHelixStride == 3 or inHelixStride == 4 or inHelixStride == 5);
//	return mHelixFlags[inHelixStride - 3];
//}
//
//bool MResidue::IsHelixStart(uint32 inHelixStride) const
//{
//	assert(inHelixStride == 3 or inHelixStride == 4 or inHelixStride == 5);
//	return mHelixFlags[inHelixStride - 3] == helixStart or mHelixFlags[inHelixStride - 3] == helixStartAndEnd;
//}
//
void Residue::SetHelixFlag(uint32 inHelixStride, EHelixFlag::Type inHelixFlag)
{
	assert(inHelixStride == 3 || inHelixStride == 4 || inHelixStride == 5);
	mHelixFlags[inHelixStride - 3] = inHelixFlag;
}
//
//void MResidue::SetSSBridgeNr(uint8 inBridgeNr)
//{
//	if (mType != kCysteine)
//		throw mas_exception("Only cysteine residues can form sulphur bridges");
//	mSSBridgeNr = inBridgeNr;
//}
//
//uint8 MResidue::GetSSBridgeNr() const
//{
//	if (mType != kCysteine)
//		throw mas_exception("Only cysteine residues can form sulphur bridges");
//	return mSSBridgeNr;
//}
//
//// TODO: use the angle to improve bond energy calculation.
//double MResidue::CalculateHBondEnergy(MResidue& inDonor, MResidue& inAcceptor)
//{
//	double result = 0;
//
//	if (inDonor.mType != kProline)
//	{
//		double distanceHO = Distance(inDonor.GetH(), inAcceptor.GetO());
//		double distanceHC = Distance(inDonor.GetH(), inAcceptor.GetC());
//		double distanceNC = Distance(inDonor.GetN(), inAcceptor.GetC());
//		double distanceNO = Distance(inDonor.GetN(), inAcceptor.GetO());
//
//		if (distanceHO < kMinimalDistance or distanceHC < kMinimalDistance or distanceNC < kMinimalDistance or distanceNO < kMinimalDistance)
//			result = kMinHBondEnergy;
//		else
//			result = kCouplingConstant / distanceHO - kCouplingConstant / distanceHC + kCouplingConstant / distanceNC - kCouplingConstant / distanceNO;
//
//		// DSSP compatibility mode:
//		result = bm::round(result * 1000) / 1000;
//
//		if (result < kMinHBondEnergy)
//			result = kMinHBondEnergy;
//	}
//
//	// update donor
//	if (result < inDonor.mHBondAcceptor[0].energy)
//	{
//		inDonor.mHBondAcceptor[1] = inDonor.mHBondAcceptor[0];
//		inDonor.mHBondAcceptor[0].residue = &inAcceptor;
//		inDonor.mHBondAcceptor[0].energy = result;
//	}
//	else if (result < inDonor.mHBondAcceptor[1].energy)
//	{
//		inDonor.mHBondAcceptor[1].residue = &inAcceptor;
//		inDonor.mHBondAcceptor[1].energy = result;
//	}
//
//	// and acceptor
//	if (result < inAcceptor.mHBondDonor[0].energy)
//	{
//		inAcceptor.mHBondDonor[1] = inAcceptor.mHBondDonor[0];
//		inAcceptor.mHBondDonor[0].residue = &inDonor;
//		inAcceptor.mHBondDonor[0].energy = result;
//	}
//	else if (result < inAcceptor.mHBondDonor[1].energy)
//	{
//		inAcceptor.mHBondDonor[1].residue = &inDonor;
//		inAcceptor.mHBondDonor[1].energy = result;
//	}
//
//	return result;
//}
//
//MBridgeType MResidue::TestBridge(MResidue* test) const
//{										// I.	a	d	II.	a	d		parallel    
//	const MResidue* a = mPrev;			//		  \			  /
//	const MResidue* b = this;			//		b	e		b	e
//	const MResidue* c = mNext;			// 		  /			  \                      ..
//	const MResidue* d = test->mPrev;	//		c	f		c	f
//	const MResidue* e = test;			//
//	const MResidue* f = test->mNext;	// III.	a <- f	IV. a	  f		antiparallel
//	//		                                   
//	MBridgeType result = btNoBridge;	//		b	 e      b <-> e                  
//	//                                          
//	//		c -> d		c     d
//
//	if (a and c and NoChainBreak(a, c) and d and f and NoChainBreak(d, f))
//	{
//		if ((TestBond(c, e) and TestBond(e, a)) or(TestBond(f, b) and TestBond(b, d)))
//			result = btParallel;
//		else if ((TestBond(c, d) and TestBond(f, a)) or(TestBond(e, b) and TestBond(b, e)))
//			result = btAntiParallel;
//	}
//
//	return result;
//}
//
//void MResidue::ExtendBox(const MAtom& atom, double inRadius)
//{
//	if (mBox[0].mX > atom.mLoc.mX - inRadius)
//		mBox[0].mX = atom.mLoc.mX - inRadius;
//	if (mBox[0].mY > atom.mLoc.mY - inRadius)
//		mBox[0].mY = atom.mLoc.mY - inRadius;
//	if (mBox[0].mZ > atom.mLoc.mZ - inRadius)
//		mBox[0].mZ = atom.mLoc.mZ - inRadius;
//	if (mBox[1].mX < atom.mLoc.mX + inRadius)
//		mBox[1].mX = atom.mLoc.mX + inRadius;
//	if (mBox[1].mY < atom.mLoc.mY + inRadius)
//		mBox[1].mY = atom.mLoc.mY + inRadius;
//	if (mBox[1].mZ < atom.mLoc.mZ + inRadius)
//		mBox[1].mZ = atom.mLoc.mZ + inRadius;
//}
//
//inline
//bool MResidue::AtomIntersectsBox(const MAtom& atom, double inRadius) const
//{
//	return
//		atom.mLoc.mX + inRadius >= mBox[0].mX and atom.mLoc.mX - inRadius <= mBox[1].mX and
//		atom.mLoc.mY + inRadius >= mBox[0].mY and atom.mLoc.mY - inRadius <= mBox[1].mY and
//		atom.mLoc.mZ + inRadius >= mBox[0].mZ and atom.mLoc.mZ - inRadius <= mBox[1].mZ;
//}
//
//void MResidue::CalculateSurface(const vector<MResidue*>& inResidues)
//{
//	vector<MResidue*> neighbours;
//
//	foreach(MResidue* r, inResidues)
//	{
//		MPoint center;
//		double radius;
//		r->GetCenterAndRadius(center, radius);
//
//		if (Distance(mCenter, center) < mRadius + radius)
//			neighbours.push_back(r);
//	}
//
//	mAccessibility = CalculateSurface(mN, kRadiusN, neighbours) +
//		CalculateSurface(mCA, kRadiusCA, neighbours) +
//		CalculateSurface(mC, kRadiusC, neighbours) +
//		CalculateSurface(mO, kRadiusO, neighbours);
//
//	foreach(const MAtom& atom, mSideChain)
//		mAccessibility += CalculateSurface(atom, kRadiusSideAtom, neighbours);
//}