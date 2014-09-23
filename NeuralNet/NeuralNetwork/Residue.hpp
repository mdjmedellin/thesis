#pragma once
#ifndef RESIDUE_H
#define RESIDUE_H

#include <iostream>

namespace GHProtein
{
	enum EResidueType
	{
		kUnknownResidue,

		//
		kAlanine,				// A	ala
		kArginine,				// R	arg
		kAsparagine,			// N	asn
		kAsparticAcid,			// D	asp
		kCysteine,				// C	cys
		kGlutamicAcid,			// E	glu
		kGlutamine,				// Q	gln
		kGlycine,				// G	gly
		kHistidine,				// H	his
		kIsoleucine,			// I	ile
		kLeucine,				// L	leu
		kLysine,				// K	lys
		kMethionine,			// M	met
		kPhenylalanine,			// F	phe
		kProline,				// P	pro
		kSerine,				// S	ser
		kThreonine,				// T	thr
		kTryptophan,			// W	trp
		kTyrosine,				// Y	tyr
		kValine,				// V	val

		kResidueTypeCount
	};

	struct ResidueInfo
	{
		EResidueType		type;
		char				code;
		char				name[4];
	};

	// a residue number to info mapping
	extern const ResidueInfo kResidueInfo[];
	EResidueType MapResidue(std::string inName);

	enum EHelixFlag
	{
		helixNone
		, helixStart
		, helixEnd
		, helixStartAndEnd
		, helixMiddle
		, helixCount
	};

	enum ESecondaryStructure
	{
		ssLoop			//' '
		, ssAlphaHelix	// H
		, ssBetaBridge	// B
		, ssStrand		// E
		, ssHelix_3		// G
		, ssHelix_5		// I
		, ssTurn		// T
		, ssBend		// S
		, ssCount
	};

	extern const char SecondaryStructureInfo[];
	ESecondaryStructure MapSecondaryStructure(char inSSCharLabel);

	class Residue
	{
	public:
		Residue();

	public:
		EResidueType		GetType() const								{ return mType; }
		void				SetType(char inResidueType);
		
		void				SetSecondaryStructure(ESecondaryStructure inSS)
							{
								mSecondaryStructure = inSS;
							}
		void				SetSecondaryStructure(char inSSCharLabel);
		ESecondaryStructure	GetSecondaryStructure() const				{ return mSecondaryStructure; }

		const Residue*		Next() const								{ return mNext; }
		const Residue*		Prev() const								{ return mPrev; }
		void				SetNext(Residue* nextResidue)				{ mNext = nextResidue; }
		void				SetPrevious(Residue* previousResidue)		{ mPrev = previousResidue; }

		void				SetSheetLabel(char inSheetLabel)			{ mSheetLabel = inSheetLabel; }

		unsigned			GetSeqNumber() const						{ return mSeqNumber; }
		void				SetSeqNumber(unsigned inSeqNumber);
		char				GetInsertionCode() const					{ return mInsertionCode; }
		void				SetInsertionCode(char inICode);

		void				SetNumber(unsigned inNumber)				{ mNumber = inNumber; }
		unsigned			GetNumber() const							{ return mNumber; }

	protected:
		char				mSheetLabel;
		Residue*			mPrev;
		Residue*			mNext;
		int					mSeqNumber, mNumber;
		char				mInsertionCode;
		EResidueType		mType;
		ESecondaryStructure	mSecondaryStructure;

	private:
		Residue&			operator=(const Residue& residue);
	};
}

#endif