#include "Residue.hpp"
#include "ProteinUtilities.hpp"
#include <assert.h>

namespace GHProtein
{
	const ResidueInfo kResidueInfo[] = 
	{
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

	const char SecondaryStructureInfo[] = 
	{
		' '
		, 'H'
		, 'B'
		, 'E'
		, 'G'
		, 'I'
		, 'T'
		, 'S'
	};

	EResidueType MapResidue(std::string inName)
	{
		Trim(inName, ETrim::TrimBoth);

		EResidueType result = EResidueType::kUnknownResidue;

		for (int i = 0; i < EResidueType::kResidueTypeCount; ++i)
		{
			if (inName == kResidueInfo[i].name)
			{
				result = kResidueInfo[i].type;
				break;
			}
		}

		return result;
	}

	EResidueType MapResidue(char inCode)
	{
		EResidueType result = EResidueType::kUnknownResidue;

		for (unsigned i = 0; i < EResidueType::kResidueTypeCount; ++i)
		{
			if (inCode == kResidueInfo[i].code)
			{
				result = kResidueInfo[i].type;
				break;
			}
		}

		return result;
	}

	ESecondaryStructure MapSecondaryStructure(char inSSCharLabel)
	{
		ESecondaryStructure result = ESecondaryStructure::ssLoop;

		for (int i = 0; i < ESecondaryStructure::ssCount; ++i)
		{
			if (inSSCharLabel == SecondaryStructureInfo[i])
			{
				result = ESecondaryStructure(result + i);
				break;
			}
		}

		return result;
	}

	Residue::Residue()
		: mSheetLabel(' ')
		, mPrev(nullptr)
		, mNext(nullptr)
		, mSeqNumber(-1)
		, mNumber(-1)
		, mInsertionCode(' ')
		, mType(EResidueType::kUnknownResidue)
		, mSecondaryStructure(ESecondaryStructure::ssLoop)
	{}

	void Residue::SetSeqNumber(unsigned inSeqNumber)
	{
		mSeqNumber = inSeqNumber;
	}

	void Residue::SetInsertionCode(char inICode)
	{
		mInsertionCode = inICode;
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

	void Residue::SetSecondaryStructure(char inSSCharLabel)
	{
		mSecondaryStructure = MapSecondaryStructure(inSSCharLabel);
	}
}