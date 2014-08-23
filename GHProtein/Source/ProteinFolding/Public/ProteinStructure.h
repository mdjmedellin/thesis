// Copyright Maarten L. Hekkelman, Radboud University 2008-2011.
//   Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE_1_0.txt or copy at    
//             http://www.boost.org/LICENSE_1_0.txt)      

#pragma once
#ifndef __ProteinStructure_h__
#define __ProteinStructure_h__

struct MAtom;
class MResidue;
class MChain;
class MProtein;

// forward declaration of buffer
template<typename T, uint32 N> class buffer;
typedef buffer<MResidue*, 100>	MResidueQueue;

const uint32 kHistogramSize = 30;

// a limited set of known atoms. This is an obvious candidate for improvement of DSSP.
UENUM()
namespace EAtomType
{
	enum Type
	{
		kUnknownAtom
		, kHydrogen
		, kCarbon
		, kNitrogen
		, kOxygen
		, kFlourine
		, kPhosphorus
		, kSulfur
		, kChlorine
		, kMagnesium
		, kPotassium
		, kCalcium
		, kZinc
		, kSelenium
		, Count						UMETA(Hidden)
	};
}

UENUM()
namespace EResidueType
{
	enum Type
	{
		kUnknownResidue
		, kAlanine					// A	ala
		, kCysteine					// R	arg
		, kAsparticAcid				// N	asn
		, kGlutamicAcid				// D	asp
		, kPhenylalanine			// C	cys
		, kGlycine					// E	glu
		, kHistidine				// Q	gln
		, kIsoleucine				// G	gly
		, kLysine					// H	his
		, kLeucine					// I	ile
		, kMethionine				// L	leu
		, kAsparagine				// K	lys
		, kProline					// M	met
		, kGlutamine				// F	phe
		, kArginine					// P	pro
		, kSerine					// S	ser
		, kThreonine				// T	thr
		, kValine					// W	trp
		, kTryptophan				// Y	tyr
		, kTyrosine					// V	val
		, Count						UMETA(Hidden)
	};
}

UENUM()
namespace EBridgeType
{
	enum Type
	{
		btNoBridge
		, btParallel
		, btAntiParallel
	};
}

UENUM()
namespace EHelixFlag
{
	enum Type
	{
		helixNone
		, helixStart
		, helixEnd
		, helixStartAndEnd
		, helixMiddle
	};
}

UENUM()
namespace ESecondaryStructure
{
	enum Type
	{
		loop			 //' '
		, alphahelix	 // H
		, betabridge	 // B
		, strand		 // E
		, helix_3		 // G
		, helix_5		 // I
		, turn			 // T
		, bend			 // S
	};
}

EAtomType::Type MapElement(FString inElement);

// for now, MAtom contains exactly what the ATOM line contains in a PDB file
struct MAtom
{
	uint32		mSerial;
	char		mName[5];
	char		mAltLoc;
	char		mResName[5];
	char		mChainID;
	int16		mResSeq;
	char		mICode;
	EAtomType	mType;
	FVector		mLoc;
	double		mOccupancy;
	double		mTempFactor;
	char		mElement[3];
	int			mCharge;

	void		SetChainID(char inID)					{ mChainID = inID; }
	FString		GetName() const							{ return mName; }
	void		Translate(const FVector& inTranslation)	{ mLoc += inTranslation; }
	void		Rotate(const MQuaternion& inRotation)	{ mLoc.Rotate(inRotation); }
	//void		WritePDB(std::ostream& os) const;

	operator const FVector&() const			{ return mLoc; }
	operator FVector&()						{ return mLoc; }
};

struct MResidueInfo
{
	EResidueType::Type		type;
	char				code;
	char				name[4];
};

// a residue number to info mapping
extern const MResidueInfo kResidueInfo[];

EResidueType MapResidue(FString inName);

struct HBond
{
	MResidue*		residue;
	double			energy;
};

struct MBridgeParner
{
	MResidue*		residue;
	uint32			ladder;
	bool			parallel;
};

class MResidue
{
public:
	MResidue(const MResidue& residue);
	MResidue(uint32 inNumber, char inTypeCode, MResidue* inPrevious);
	MResidue(uint32 inNumber, MResidue* inPrevious, const TArray<MAtom>& inAtoms);

	void				SetChainID(char inID);
	char				GetChainID() const				{ return mChainID; }

	EResidueType		GetType() const					{ return mType; }

	const MAtom&		GetCAlpha() const				{ return mCA; }
	const MAtom&		GetC() const					{ return mC; }
	const MAtom&		GetN() const					{ return mN; }
	const MAtom&		GetO() const					{ return mO; }
	const MAtom&		GetH() const					{ return mH; }

	double				Phi() const;
	double				Psi() const;
	/*std::tr1::tuple<double, char>
	Alpha() const;*/
	double				Kappa() const;
	double				TCO() const;

	double				Accessibility() const			{ return mAccessibility; }

	void				SetSecondaryStructure(ESecondaryStructure inSS)
	{
		mSecondaryStructure = inSS;
	}
	ESecondaryStructure	GetSecondaryStructure() const	{ return mSecondaryStructure; }

	const MResidue*		Next() const					{ return mNext; }
	const MResidue*		Prev() const					{ return mPrev; }

	void				SetPrev(MResidue* inResidue);

	void				SetBetaPartner(uint32 n, MResidue* inResidue, uint32 inLadder, bool inParallel);
	MBridgeParner		GetBetaPartner(uint32 n) const;

	void				SetSheet(uint32 inSheet)	{ mSheet = inSheet; }
	uint32				GetSheet() const			{ return mSheet; }

	bool				IsBend() const				{ return mBend; }
	void				SetBend(bool inBend)		{ mBend = inBend; }

	EHelixFlag			GetHelixFlag(uint32 inHelixStride) const;
	bool				IsHelixStart(uint32 inHelixStride) const;
	void				SetHelixFlag(uint32 inHelixStride, EHelixFlag inHelixFlag);

	void				SetSSBridgeNr(uint8 inBridgeNr);
	uint8				GetSSBridgeNr() const;

	void				AddAtom(MAtom& inAtom);

	HBond*				Donor()						{ return mHBondDonor; }
	HBond*				Acceptor()					{ return mHBondAcceptor; }

	const HBond*		Donor() const				{ return mHBondDonor; }
	const HBond*		Acceptor() const			{ return mHBondAcceptor; }

	bool				ValidDistance(const MResidue& inNext) const;

	static bool			TestBond(const MResidue* a, const MResidue* b)
	{
		return a->TestBond(b);
	}

	// bridge functions
	EBridgeType			TestBridge(MResidue* inResidue) const;

	uint16				GetSeqNumber() const		{ return mSeqNumber; }
	char				GetInsertionCode() const	{ return mInsertionCode; }

	void				SetNumber(uint16 inNumber)	{ mNumber = inNumber; }
	uint16				GetNumber() const			{ return mNumber; }

	void				Translate(const FVector& inTranslation);
	void				Rotate(const MQuaternion& inRotation);

	//void				WritePDB(std::ostream& os);

	static double		CalculateHBondEnergy(MResidue& inDonor, MResidue& inAcceptor);

	/*std::vector<MAtom>&	GetSideChain()				{ return mSideChain; }
	const std::vector<MAtom>&
	GetSideChain() const		{ return mSideChain; }*/

	void				GetPoints(TArray<FVector>& outPoints) const;

	void				CalculateSurface(const TArray<MResidue*>& inResidues);

	void				GetCenterAndRadius(FVector& outCenter, double& outRadius) const
	{
		outCenter = mCenter; outRadius = mRadius;
	}

	static bool			NoChainBreak(const MResidue* from, const MResidue* to);

protected:

	double				CalculateSurface(const MAtom& inAtom,
		double inRadius,
		const TArray<MResidue*>& inResidues);

	bool				TestBond(const MResidue* other) const;

	void				ExtendBox(const MAtom& atom, double inRadius);
	bool				AtomIntersectsBox(const MAtom& atom, double inRadius) const;

	char				mChainID;
	MResidue*			mPrev;
	MResidue*			mNext;
	int32				mSeqNumber, mNumber;
	char				mInsertionCode;
	EResidueType		mType;
	uint8				mSSBridgeNr;
	double				mAccessibility;
	ESecondaryStructure	mSecondaryStructure;
	MAtom				mC, mN, mCA, mO, mH;
	HBond				mHBondDonor[2], mHBondAcceptor[2];
	TArray<MAtom>		mSideChain;
	MBridgeParner		mBetaPartner[2];
	uint32				mSheet;
	MHelixFlag			mHelixFlags[3];	//
	bool				mBend;
	FVector				mBox[2];		// The 3D box containing all atoms
	FVector				mCenter;		// and the 3d Sphere containing all atoms
	double				mRadius;

private:
	MResidue&			operator=(const MResidue& residue);
};

class MChain
{
public:

	MChain(const MChain& chain);
	MChain(char inChainID = 0) : mChainID(inChainID) {}
	~MChain();

	MChain&				operator=(const MChain& chain);

	char				GetChainID() const					{ return mChainID; }
	void				SetChainID(char inID);

	MResidue*			GetResidueBySeqNumber(uint16 inSeqNumber, char inInsertionCode);

	void				GetSequence(FString& outSequence) const;

	void				Translate(const FVector& inTranslation);
	void				Rotate(const FVector& inRotation);

	//void				WritePDB(std::ostream& os);

	TArray<MResidue*>&
		GetResidues()						{ return mResidues; }
	const TArray<MResidue*>&
		GetResidues() const					{ return mResidues; }

	bool				Empty() const						{ return mResidues.empty(); }

private:
	char				mChainID;
	TArray<MResidue*>	mResidues;
};

class MProtein
{
public:
	MProtein() {}
	MProtein(const FString& inID, MChain* inChain);
	~MProtein();

	const FString&	GetID() const					{ return mID; }

	//MProtein(std::istream& is, bool inCAlphaOnly = false);

	const FString&	GetHeader() const				{ return mHeader; }
	FString			GetCompound() const;
	FString			GetSource() const;
	FString			GetAuthor() const;

	void				CalculateSecondaryStructure();

	void				GetStatistics(uint32& outNrOfResidues, uint32& outNrOfChains,
		uint32& outNrOfSSBridges, uint32& outNrOfIntraChainSSBridges,
		uint32& outNrOfHBonds, uint32 outNrOfHBondsPerDistance[11]) const;

	void				GetCAlphaLocations(char inChain, TArray<FVector>& outPoints) const;
	FVector				GetCAlphaPosition(char inChain, int16 inPDBResSeq) const;

	/*void				GetSequence(char inChain, entry& outEntry) const;
	void				GetSequence(char inChain, sequence& outSequence) const;*/

	void				Center();
	void				Translate(const FVector& inTranslation);
	//void				Rotate(const MQuaternion& inRotation);

	//void				WritePDB(std::ostream& os);

	void				GetPoints(TArray<FVector>& outPoints) const;

	char				GetFirstChainID() const								{ return mChains.front()->GetChainID(); }

	void				SetChain(char inChainID, const MChain& inChain);

	MChain&				GetChain(char inChainID);
	const MChain&		GetChain(char inChainID) const;

	const TArray<MChain*>& GetChains() const									{ return mChains; }

	/*template<class OutputIterator>
	void				GetSequences(OutputIterator outSequences) const;*/

	MResidue*			GetResidue(char inChainID, uint16 inSeqNumber, char inInsertionCode);

	// statistics
	uint32				GetNrOfHBondsInParallelBridges() const				{ return mNrOfHBondsInParallelBridges; }
	uint32				GetNrOfHBondsInAntiparallelBridges() const			{ return mNrOfHBondsInAntiparallelBridges; }

	void				GetResiduesPerAlphaHelixHistogram(uint32 outHistogram[30]) const;
	void				GetParallelBridgesPerLadderHistogram(uint32 outHistogram[30]) const;
	void				GetAntiparallelBridgesPerLadderHistogram(uint32 outHistogram[30]) const;
	void				GetLaddersPerSheetHistogram(uint32 outHistogram[30]) const;

private:

	void				AddResidue(const TArray<MAtom>& inAtoms);

	void				CalculateHBondEnergies(const TArray<MResidue*>& inResidues);
	void				CalculateAlphaHelices(const TArray<MResidue*>& inResidues);
	void				CalculateBetaSheets(const TArray<MResidue*>& inResidues);
	void				CalculateAccessibilities(const TArray<MResidue*>& inResidues);

	// a thread entry point
	void				CalculateAccessibility(MResidueQueue& inQueue, TArray<MResidue*>& inResidues);

	FString			mID, mHeader;

	TArray<FString> mCompound, mSource, mAuthor;
	TArray<MChain*>mChains;
	uint32				mResidueCount, mChainBreaks;

	TArray< TPair<MResidue*, MResidue*> >
		mSSBonds;
	uint32				mIgnoredWaterMolecules;

	// statistics
	uint32				mNrOfHBondsInParallelBridges, mNrOfHBondsInAntiparallelBridges;
	uint32				mParallelBridgesPerLadderHistogram[kHistogramSize];
	uint32				mAntiparallelBridgesPerLadderHistogram[kHistogramSize];
	uint32				mLaddersPerSheetHistogram[kHistogramSize];
};

// inlines

// GetSequences can be used to quickly get all sequences in a vector<string> e.g.
//template<class OutputIterator>
//void MProtein::GetSequences(OutputIterator outSequences) const
//{
//	for (std::vector<MChain*>::const_iterator chain = mChains.begin(); chain != mChains.end(); ++chain)
//	{
//		std::string seq;
//		(*chain)->GetSequence(seq);
//		*outSequences++ = seq;
//	}
//}

