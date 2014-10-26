// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

namespace GHProtein
{
	class ProteinModel;
}

class Residue;

/** JM: The protein builder class is intended to be used to create the models for the proteins
	It contains functions that will load a file in order to create a protein model */
class ProteinBuilder
{
public:

	ProteinBuilder() { bFileLoaded = false; };

	/** Destructor */
	~ProteinBuilder() { Clear(); };

private:

	/** No conversion allowed */
	ProteinBuilder(const ProteinBuilder& rhs) {};
	/** No assignment allowed */
	ProteinBuilder& operator=(const ProteinBuilder& rhs) { return *this; };

public:

	/**
	* Loads the file with the passed path
	* @param	Path				The path to use
	*/
	bool LoadFile(const FString& Path);
	/** Gets the last error message from the class */
	FString GetLastError() const;
	/** Clears the file of all internals. */
	void Clear();

	/** Gets a pointer to a protein created from the file that is currently loader
	*	@return null if no file has been laoded
	*/
	GHProtein::ProteinModel* GetCurrentProteinModel(UWorld* proteinWorld);

private:

	/** Pre-processes the file doing anything necessary (such as removing comments) */
	void PreProcessInput(TArray<FString>& Input);
	bool ExtractResidue(const FString& informationString, Residue*& out_extractedResidue);
	bool GetLineOfFirstAminoAcid(int& out_LineOfFirstAminoAcid);

private:

	/** The passed-in path of the loaded file (might be absolute or relative) */
	FString LoadedFile;
	/** An error message generated on errors to return to the client */
	FString ErrorMessage;
	/** Flag for whether a file is loaded or not */
	bool bFileLoaded;
	/** Array that keeps the file's information separated by lines */
	TArray<FString> m_inputLines;
};