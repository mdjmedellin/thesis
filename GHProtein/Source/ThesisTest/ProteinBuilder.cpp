#include "ThesisTest.h"
#include "ProteinBuilder.h"
#include "ProteinModel.h"
#include "Residue.h"
#include "ProteinUtilities.h"

bool ProteinBuilder::LoadFile(const FString& inFile)
{
	// Remove any file stuff if it already exists
	Clear();

	// So far no error (Early so it can be overwritten below by errors)
	//ErrorMessage = NSLOCTEXT("PdbParser", "LoadSuccess", "PdbFile was loaded successfully").ToString();

	// Create file reader
	TUniquePtr<FArchive> fileReader(IFileManager::Get().CreateFileReader(*inFile));
	if (!fileReader)
	{
		//ErrorMessage = NSLOCTEXT("PdbParser", "FileLoadFail", "Failed to load the file").ToString();
		//ErrorMessage += TEXT("\"");
		//ErrorMessage += InFile;
		//ErrorMessage += TEXT("\"");
		return false;
	}

	// Create buffer for file input
	uint32 bufferSize = fileReader->TotalSize();
	// MAKE SURE TO DEALLOCATE Buffer
	void* buffer = FMemory::Malloc(bufferSize);
	fileReader->Serialize(buffer, bufferSize);

	//TODO:
	// JM: At some point we should add a check to see what is the size of a character on the platform we are running on
	GHProtein::SplitLines(m_inputLines, static_cast<const uint8*>(buffer), bufferSize);

	// We have stored the file in the an array of strings
	// We no longer need the buffer at this point
	// Release resources
	FMemory::Free(buffer);

	// Pre-process the input
	//PreProcessInput(Input);

	// All done with creation, set up necessary information
	LoadedFile = inFile;
	bFileLoaded = true;

	return true;
}

FString ProteinBuilder::GetLastError() const
{
	return ErrorMessage;
}

void ProteinBuilder::Clear()
{
	//TODO:
	//JM: Actually use the variable bFileLoaded in order to detect when we have contents of a file
	if (bFileLoaded || m_inputLines.Num() > 0)
	{
		bFileLoaded = false;
		LoadedFile = TEXT("");
		m_inputLines.Empty();
		//ErrorMessage = NSLOCTEXT("PdbParser", "ClearSuccess", "PdbFile was cleared successfully").ToString();
	}
}

bool ProteinBuilder::GetLineOfFirstAminoAcid(int& out_LineOfFirstAminoAcid)
{
	//we know that a dssp file will have a line that looks like this before the information of amino acids
	//  #  RESIDUE AA STRUCTURE BP1 BP2  ACC     N - H-->O    O-->H - N    N - H-->O    O-->H - N    TCO  KAPPA ALPHA  PHI   PSI    X - CA   Y - CA   Z - CA
	//Therefore we can check for sepecific characters in this line in order to find the line that comes before
	//the information of amino acids

	bool foundLineOfAminoAcidInformation = false;

	if (bFileLoaded)
	{
		FString* currentLine;
		for (int i = 0; i < m_inputLines.Num(); ++i)
		{
			currentLine = &m_inputLines[i];

			if (m_inputLines[i].Len() > 130
				&& m_inputLines[i][2] == '#'
				&& m_inputLines[i][48] == '>'
				&& m_inputLines[i][92] == 'K')
			{
				//we are on the line before the amino acids information
				if (m_inputLines.Num() > i)
				{
					out_LineOfFirstAminoAcid = i + 1;
					foundLineOfAminoAcidInformation = true;
				}
				break;
			}
		}
	}

	return foundLineOfAminoAcidInformation;
}

void ProteinBuilder::PreProcessInput(TArray<FString>& Input)
{}

GHProtein::ProteinModel* ProteinBuilder::GetCurrentProteinModel()
{
	GHProtein::ProteinModel* localProteinModel = nullptr;

	if (bFileLoaded)
	{
		localProteinModel = new GHProtein::ProteinModel();

		/** search through the lines of the dssp file in order to find where
		*	the information of the amino acids is located
		*/
		int aminoAcidInformationStartingLine = 0;

		if (!GetLineOfFirstAminoAcid(aminoAcidInformationStartingLine))
		{
			//if we do not have the amino acid informatio, then we cannot
			//build a protein model
			return nullptr;
		}

		/** once we have the starting line of the first amino acid, then it
		*	is time to parse the lines in order to build the model
		*/
		FString currentLine = "";
		Residue* extractedResidue = nullptr;
		for(int currentLineNumber = aminoAcidInformationStartingLine;
			currentLineNumber < m_inputLines.Num();
			++currentLineNumber)
		{
			//instead of creating a new pointer, we just recycle the one used
			//in the previous iteration
			extractedResidue = nullptr;
			currentLine = m_inputLines[currentLineNumber];

			if (ExtractResidue(currentLine, extractedResidue))
			{
				localProteinModel->AddResidue(extractedResidue);
				extractedResidue = nullptr;
			}
		}

		//build the protein model at this point
		localProteinModel->BuildProteinModel();
		return localProteinModel;
	}

	return nullptr;
}

bool ProteinBuilder::ExtractResidue(const FString& informationString, Residue*& out_extractedResidue)
{
	//The reason we take a pointer by reference is that we have a need to modify the pointer rather
	//than the object the pointer is pointing to

	//A valid line has 136 characters
	//"%5.5d%5.5d%c%c %c  %c %c%c%c%c%c%c%c%4.4d%4.4d%c%4.4d %11s%11s%11s%11s  %6.3f%6.1f%6.1f%6.1f%6.1f %6.1f %6.1f %6.1f"
	//  #  RESIDUE AA STRUCTURE BP1 BP2  ACC     N - H-->O    O-->H - N    N - H-->O    O-->H - N    TCO  KAPPA ALPHA  PHI   PSI    X - CA   Y - CA   Z - CA
	//	  1    1 A K              0   0   92      0, 0.0    39, -2.6     0, 0.0     2, -0.6   0.000 360.0 360.0 360.0 146.1    2.4   10.4    9.2

	if (informationString.Len() != 136)
	{
		return false;
	}
	else
	{
		//check if we need to construct a residue
		if (out_extractedResidue == nullptr)
		{
			out_extractedResidue = new Residue();
		}

		int currentStringIndex = 0;

		//the following variables are used in order to store information we are extracting from a line
		FString tempString = "";
		int tempNumber = 0;
		float tempFloat = 0.f;

		//the residue number line
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 5);
		GHProtein::GetTypeFromString(tempNumber, tempString);
		out_extractedResidue->SetNumber(tempNumber);

		//the residue sequence number
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 5);
		GHProtein::GetTypeFromString(tempNumber, tempString);
		out_extractedResidue->SetSeqNumber(tempNumber);

		//Not sure what the insertion code is for
		//it is likely to be empty in most cases
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 1);
		out_extractedResidue->SetInsertionCode(tempString[0]);

		//ChainID
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 1);
		out_extractedResidue->SetChainID(tempString[0]);

		//there is a space in between
		currentStringIndex += 1;

		//the shorthand form of the residue we are currently creating
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 1);
		out_extractedResidue->SetType(tempString[0]);

		//there are two spaces in between
		currentStringIndex += 2;

		//shorthand for the secondary structure the Define Secondary Structure of Proteins Algorithms gives us
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 1);
		out_extractedResidue->SetSecondaryStructure(tempString[0]);

		//there is a space in between
		currentStringIndex += 1;

		//read the helix information
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 3);
		char helix[3] = { tempString[0], tempString[1], tempString[2] };
		out_extractedResidue->SetHelixFlags(helix);

		//Set the bend information
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 1);
		out_extractedResidue->SetBend(tempString[0] != ' ');

		//we probably do not need chirality so we just skip that character
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 1);
		///*chirality = ReadFromLine(informationString, currentStringIndex, 1);
		//out_extractedResidue->SetChirality(chirality);*/

		//Set the bridge labels
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 2);
		char bridgeLabel[2] = { tempString[0], tempString[1] };
		out_extractedResidue->SetBridgeLabels(bridgeLabel);

		//Read the bridge partner IDs
		//They seem to range from 0 - 9,999
		//This means that if we have more than 9,999 residues, we might not pair up the appropiate ones
		//A solution with this would be to check the residue it matches to see if it matches with the one
		//that asked for its match, this would only incur a cost in the pairing of the residues since we
		//would have to wait until all residues are read and then traverses them to see if they match with
		//each other.
		//At this point then I would rather go with a map. I would prefer to map the residue number to a
		//list of residues. Then I could quickly access the residue instead of traversing through all of them
		int bp[2] = { 0, 0 };
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 4);
		GHProtein::GetTypeFromString(bp[0], tempString);
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 4);
		GHProtein::GetTypeFromString(bp[1], tempString);
		out_extractedResidue->SetBetaPartners(bp);

		//Sheet does not seem to be used for anything at this moment
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 1);
		out_extractedResidue->SetSheetLabel(tempString[0]);

		//
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 4);
		GHProtein::GetTypeFromString(tempNumber, tempString);
		out_extractedResidue->SetAccessibility(tempNumber);

		//there is a space in between
		currentStringIndex += 1;

		FString NHO[2] = { "", "" };
		FString ONH[2] = { "", "" };
		GHProtein::ReadFromLine(informationString, NHO[0], currentStringIndex, 11);
		GHProtein::ReadFromLine(informationString, ONH[0], currentStringIndex, 11);
		GHProtein::ReadFromLine(informationString, NHO[1], currentStringIndex, 11);
		GHProtein::ReadFromLine(informationString, ONH[1], currentStringIndex, 11);
		out_extractedResidue->SetNHO(NHO);
		out_extractedResidue->SetONH(ONH);

		//there are two spaces in between
		currentStringIndex += 2;

		//TCO is not used for structure definition
		//The cosine of the angle between C=O of the current residue and C=O of previous residue.
		//For alpha-helices, TCO is near +1, for beta-sheets TCO is near -1. Not used for structure definition
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 6);
		GHProtein::GetTypeFromString(tempFloat, tempString);
		out_extractedResidue->SetTCO(tempFloat);

		//Kappa could be very useful for us when determining the angle between the bend
		//The virtual bond angle(bend angle) defined by the three C - alpha atoms of the residues current - 2, current and current + 2.
		//Used to define bend(structure code 'S').
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 6);
		GHProtein::GetTypeFromString(tempFloat, tempString);
		out_extractedResidue->SetKappa(tempFloat);

		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 6);
		GHProtein::GetTypeFromString(tempFloat, tempString);
		out_extractedResidue->SetAlpha(tempFloat);

		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 6);
		GHProtein::GetTypeFromString(tempFloat, tempString);
		out_extractedResidue->SetPhi(tempFloat);

		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 6);
		GHProtein::GetTypeFromString(tempFloat, tempString);
		out_extractedResidue->SetPsi(tempFloat);

		currentStringIndex += 1;

		FVector caLocation;
		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 6);
		GHProtein::GetTypeFromString(caLocation.X, tempString);

		currentStringIndex += 1;

		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 6);
		GHProtein::GetTypeFromString(caLocation.Y, tempString);

		currentStringIndex += 1;

		GHProtein::ReadFromLine(informationString, tempString, currentStringIndex, 6);
		GHProtein::GetTypeFromString(caLocation.Z, tempString);

		out_extractedResidue->SetCALocation(caLocation);

		return true;
	}
}