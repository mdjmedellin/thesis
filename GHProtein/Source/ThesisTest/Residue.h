#pragma once
#include "UObjectBaseUtility.h"
#include "ThesisStaticLibrary.h"

//JM: Globals that seem to be used thorughout
// --------------------------------------------------------------------

const double
kSSBridgeDistance = 3.0,
kMinimalDistance = 0.5,
kMinimalCADistance = 9.0,
kMinHBondEnergy = -9.9,
kMaxHBondEnergy = -0.5,
kCouplingConstant = -27.888,	//	= -332 * 0.42 * 0.2
kMaxPeptideBondLength = 2.5;

const double
kRadiusN = 1.65,
kRadiusCA = 1.87,
kRadiusC = 1.76,
kRadiusO = 1.4,
kRadiusSideAtom = 1.8,
kRadiusWater = 1.4;

// --------------------------------------------------------------------

class Residue;
EAtomType::Type MapElement(FString inElement);

// for now, Atom contains exactly what the ATOM line contains in a PDB file
struct Atom
{
	uint32		mSerial;
	char		mName[5];
	char		mAltLoc;
	char		mResName[5];
	char		mChainID;
	int16		mResSeq;
	char		mICode;
	EAtomType::Type	mType;
	FVector		mLoc;
	double		mOccupancy;
	double		mTempFactor;
	char		mElement[3];
	int			mCharge;

	void		SetChainID(char inID)					{ mChainID = inID; }
	FString		GetName() const							{ return mName; }
	void		Translate(const FVector& inTranslation)	{ mLoc += inTranslation; }
	//void		Rotate(const MQuaternion& inRotation)	{ mLoc.Rotate(inRotation); }
	
	operator const FVector&() const			{ return mLoc; }
	operator FVector&()						{ return mLoc; }
};

struct ResidueInfo
{
	EResidueType::Type	type;
	char				code;
	char				name[4];
	FString				fullName;
};

// a residue number to info mapping
extern const ResidueInfo kResidueInfo[];

EResidueType::Type MapResidue(FString inName);
EResidueType::Type MapResidue(char inChar);
ResidueInfo MapResidueInfo(EResidueType::Type ResidueType);

struct HBond
{
	Residue*		residue;
	double			energy;
};

struct BridgePartner
{
	BridgePartner()
	: residue(nullptr)
	, ladder(' ')
	, parallel(false)
	, number( -1 )
	{}

	Residue*		residue;
	uint32			ladder;
	bool			parallel;
	int				number;
};

extern const char SecondaryStructureInfo[];
ESecondaryStructure::Type MapSecondaryStructure(char inSSCharLabel);

class Residue
{

public:
	Residue();
	//Residue(const Residue& residue);
	//Residue(uint32 inNumber, char inTypeCode, Residue* inPrevious);
	//Residue(uint32 inNumber, Residue* inPrevious, const TArray<Atoms>& inAtoms);

public:
	void				SetChainID(char inID);
	char				GetChainID() const							{ return mChainID; }
	EResidueType::Type	GetType() const								{ return mType; }
	void				SetType(char inResidueType);
	void				SetType(EResidueType::Type inResidueType);
	const Atom&			GetCAlpha() const							{ return mCA; }
	const Atom&			GetC() const								{ return mC; }
	const Atom&			GetN() const								{ return mN; }
	const Atom&			GetO() const								{ return mO; }
	const Atom&			GetH() const								{ return mH; }
	double				Phi() const;
	double				Psi() const;
	
	//std::tr1::tuple<double, char> Alpha() const;

	double				Accessibility() const						{ return mAccessibility; }
	void				SetAccessibility(int inAccessibility)		{ mAccessibility = inAccessibility; }

	void				SetSecondaryStructure(ESecondaryStructure::Type inSS)
						{
							mSecondaryStructure = inSS;
						}
	void				SetSecondaryStructure(char inSSCharLabel);

	void				SetNHO(FString* NHO);
	void				SetONH(FString* ONH);

	ESecondaryStructure::Type	GetSecondaryStructure() const		{ return mSecondaryStructure; }

	const Residue*		Next() const								{ return mNext; }
	const Residue*		Prev() const								{ return mPrev; }
	void				SetNext(Residue* nextResidue)				{ mNext = nextResidue; }
	void				SetPrevious(Residue* previousResidue)		{ mPrev = previousResidue; }

	void				SetPrev(Residue* inResidue);

	BridgePartner		GetBetaPartner(uint32 n) const;
	void				SetBetaPartners(int* betaPartners);
	void				SetBetaPartnerResidue(int betaPartnerID, Residue* betaPartnerResidue);

	void				SetBridgeLabels(char* inBridgeLabels);
	void				SetBridgePartnersNumber(int* inBridgePartnersNumber);

	void				SetSheetLabel(char inSheetLabel)			{ mSheetLabel = inSheetLabel; }

	bool				IsBend() const								{ return mBend; }
	void				SetBend(bool inBend)						{ mBend = inBend; }

	EHelixFlag::Type	GetHelixFlag(uint32 inHelixStride) const;
	bool				IsHelixStart(uint32 inHelixStride) const;
	void				SetHelixFlag(uint32 inHelixStride, EHelixFlag::Type inHelixFlag);
	void				SetHelixFlags(const char* inHelixFlags);

	void				SetSSBridgeNr(uint8 inBridgeNr);
	uint8				GetSSBridgeNr() const;

	void				AddAtom(Atom& inAtom);

	/*HBond*				Donor()										{ return mHBondDonor; }
	HBond*				Acceptor()									{ return mHBondAcceptor; }

	const HBond*		Donor() const								{ return mHBondDonor; }
	const HBond*		Acceptor() const							{ return mHBondAcceptor; }*/

	bool				ValidDistance(const Residue& inNext) const;

	static bool			TestBond(const Residue* a, const Residue* b)
						{
								return a->TestBond(b);
						}

	// bridge functions
	EBridgeType::Type	TestBridge(Residue* inResidue) const;

	uint16				GetSeqNumber() const						{ return mSeqNumber; }
	void				SetSeqNumber(uint16 inSeqNumber);
	char				GetInsertionCode() const					{ return mInsertionCode; }
	void				SetInsertionCode(char inICode);

	void				SetNumber(uint16 inNumber)					{ mNumber = inNumber; }
	uint16				GetNumber() const							{ return mNumber; }

	void				Translate(const FVector& inTranslation);
	//void				Rotate(const MQuaternion& inRotation);

	static double		CalculateHBondEnergy(Residue& inDonor, Residue& inAcceptor);

	/*TArray<Atom>&		GetSideChain()								{ return mSideChain; }
	const TArray<Atom>& GetSideChain() const						{ return mSideChain; }*/

	void				GetPoints(TArray<FVector>& outPoints) const;

	void				CalculateSurface(const TArray<Residue*>& inResidues);

	/*void				GetCenterAndRadius(FVector& outCenter, double& outRadius) const
						{
							outCenter = mCenter; outRadius = mRadius;
						}*/

	void				SetTCO(float inTCO)							{ mTCO = inTCO; }
	void				SetKappa(float inKappa)						{ mKappa = inKappa; }
	void				SetAlpha(float inAlpha)						{ mAlpha = inAlpha; }
	void				SetPhi(float inPhi)							{ mPhi = inPhi; }
	void				SetPsi(float inPsi)							{ mPsi = inPsi; }
	void				SetCALocation(FVector inLocation)			{ mCA.mLoc = inLocation; }
	void				GetCALocation(FVector& out_location)		{ out_location.Set(mCA.mLoc.X, mCA.mLoc.Y, mCA.mLoc.Z); }

	bool				IsBetaPartnerOf(Residue* partnerResidue);
	static bool			NoChainBreak(const Residue* from, const Residue* to);

protected:

	/*double				CalculateSurface(
						const Atom& inAtom, double inRadius,
						const TArray<Residue*>& inResidues);*/

	bool				TestBond(const Residue* other) const;

	//void				ExtendBox(const Atom& atom, double inRadius);
	/*bool				AtomIntersectsBox(const Atom& atom, double inRadius) const;*/

	char				mSheetLabel;
	char				mChainID;
	Residue*			mPrev;
	Residue*			mNext;
	int32				mSeqNumber, mNumber;
	char				mInsertionCode;
	EResidueType::Type	mType;
	uint8				mSSBridgeNr;
	double				mAccessibility;
	ESecondaryStructure::Type	mSecondaryStructure;
	Atom				mC, mN, mCA, mO, mH;
	HBond				mHBondDonor[2], mHBondAcceptor[2];
	//TArray<Atom>		mSideChain;
	BridgePartner		mBetaPartner[2];
	uint32				mSheet;
	EHelixFlag::Type	mHelixFlags[3];	//
	bool				mBend;
	float				mTCO;
	float				mKappa;
	float				mAlpha;
	float				mPhi;
	float				mPsi;
	//FVector				mBox[2];		// The 3D box containing all atoms
	//FVector				mCenter;		// and the 3d Sphere containing all atoms
	//double				mRadius;
};