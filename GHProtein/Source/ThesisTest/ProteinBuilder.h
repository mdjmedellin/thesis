// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

namespace GHProtein
{
	class ProteinModel;
}

class Residue;
class AAminoAcid;

/** JM: The protein builder class is intended to be used to create the models for the proteins
	It contains functions that will load a file in order to create a protein model */
class ProteinBuilder
{
private:
	/** This class is intended to be a singleton */
	ProteinBuilder() { bFileLoaded = false; };
	/** No conversion allowed */
	ProteinBuilder(const ProteinBuilder& rhs);
	/** No assignment allowed */
	ProteinBuilder& operator=(const ProteinBuilder& rhs);

	/** Destructor */
	~ProteinBuilder() { Clear(); };


	bool ExtractResidue(const FString& informationString, Residue*& out_extractedResidue);
	bool GetLineOfFirstAminoAcid(int& out_LineOfFirstAminoAcid);
	/** Clears the file of all internals. */
	void Clear();

public:
	static ProteinBuilder& GetInstance();

	/**
	* Loads the file with the passed path
	* @param	Path				The path to use
	*/
	bool LoadFile(const FString& Path);

	/** Gets a pointer to a protein created from the file that is currently loader
	*	@return null if no file has been laoded
	*/
	GHProtein::ProteinModel* GetCurrentProteinModel(UWorld* proteinWorld);
	GHProtein::ProteinModel* CreateCustomChain(TArray<AAminoAcid*>& residues, float aminoAcidSize,
		float distanceScale, float linkWidth, float linkHeight, float betaStrandLinkWidth, float helixLinkWidth,
		float hydrogenBondLinkWidth, FColor& normalColor, FColor& helixColor, FColor& betaStrandColor,
		FColor& hydrogenBondColor, UClass* defaultHydrogenBondClass, UWorld* proteinWorld);

private:
	/** The passed-in path of the loaded file (might be absolute or relative) */
	FString LoadedFile;
	/** Flag for whether a file is loaded or not */
	bool bFileLoaded;
	/** Array that keeps the file's information separated by lines */
	TArray<FString> m_inputLines;

public:
};