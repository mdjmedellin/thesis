#pragma once

#ifndef PROTEIN_BUILDER_H
#define PROTEIN_BUILDER_H

#include <fstream>
#include <vector>

namespace GHProtein
{
	class ProteinModel;
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
		ProteinModel* LoadFile(const std::string& Path);
		/** Clears the file of all internals. */
		void Clear();

		/** 
		*	Gets a pointer to a protein created from the file that is currently loader
		*	@return null if no file has been laoded
		*/
		

	private:
		/** Pre-processes the file doing anything necessary (such as removing comments) */
		bool ExtractResidue(const std::string& informationString, Residue*& out_extractedResidue);
		bool MoveToFirstLineOfAminoAcid();
		ProteinModel* LoadProteinModel();
		ProteinModel* LoadProteinFromJM();
		ProteinModel* LoadProteinFromDSSP();
		ProteinModel* CreateProteinModel();
		void CheckIfStringContainsAminoAcidInfo(const std::string& currentLine);
		ProteinModel* CreateProteinModelFromJM();

	private:
		/** The passed-in path of the loaded file (might be absolute or relative) */
		std::string LoadedFile;
		/** Flag for whether a file is loaded or not */
		bool bFileLoaded;
		/** Array that keeps the file's information separated by lines */
		std::vector<std::string> m_inputLines;

		std::ifstream m_dataFile;

		std::string m_loadedFileName;
	};
}

#endif