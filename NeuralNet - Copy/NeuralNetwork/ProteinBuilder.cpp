#include "ProteinBuilder.hpp"
#include "ProteinUtilities.hpp"
#include "ProteinModel.hpp"
#include <sstream>

namespace GHProtein
{
	ProteinModel* ProteinBuilder::LoadFile(const std::string& inFile)
	{
		// Remove any file stuff if it already exists
		Clear();

		// Create file reader
		m_dataFile.open(inFile);
		if (!m_dataFile.good())
		{
			return nullptr;
		}
		else
		{
			m_loadedFileName = inFile;
			bFileLoaded = true;
		}

		ProteinModel* loadedProtein = LoadProteinModel();

		m_dataFile.close();
		return loadedProtein;
	}

	ProteinModel* ProteinBuilder::LoadProteinModel()
	{
		//depending on the type of file that is loaded, we choose how to load the protein model
		int indexOfExtensionStart = m_loadedFileName.find_last_of('.') + 1;
		std::string substring = m_loadedFileName.substr(indexOfExtensionStart);
		GHProtein::Trim(substring, GHProtein::ETrim::TrimBoth);

		//check what ype of file it is
		if (substring == "jm")
		{
			return LoadProteinFromJM();
		}
		else if (substring == "dssp")
		{
			return LoadProteinFromDSSP();
		}

		return nullptr;
	}

	ProteinModel* ProteinBuilder::LoadProteinFromJM()
	{
		return CreateProteinModelFromJM();
	}

	ProteinModel* ProteinBuilder::LoadProteinFromDSSP()
	{
		if (MoveToFirstLineOfAminoAcid())
		{
			return CreateProteinModel();
		}

		return nullptr;
	}

	void ProteinBuilder::Clear()
	{
		bFileLoaded = false;
	}

	bool ProteinBuilder::MoveToFirstLineOfAminoAcid()
	{
		//we know that a dssp file will have a line that looks like this before the information of amino acids
		//  #  RESIDUE AA STRUCTURE BP1 BP2  ACC     N - H-->O    O-->H - N    N - H-->O    O-->H - N    TCO  KAPPA ALPHA  PHI   PSI    X - CA   Y - CA   Z - CA
		//Therefore we can check for sepecific characters in this line in order to find the line that comes before
		//the information of amino acids
		bool foundLineOfAminoAcidInformation = false;

		if (bFileLoaded)
		{
			std::string currentLine;
			while (!m_dataFile.eof() && !foundLineOfAminoAcidInformation)
			{
				getline(m_dataFile, currentLine);

				if (currentLine.length() > 130
					&& currentLine.at(2) == '#'
					&& currentLine.at(48) == '>'
					&& currentLine.at(92) == 'K')
				{
					//we are on the line before the amino acids information
					foundLineOfAminoAcidInformation = true;
				}
			}
		}

		return foundLineOfAminoAcidInformation;
	}

	ProteinModel* ProteinBuilder::CreateProteinModel()
	{
		ProteinModel* localProteinModel = nullptr;

		if (bFileLoaded)
		{
			localProteinModel = new ProteinModel();

			/** once we have the starting line of the first amino acid, then it
			*	is time to parse the lines in order to build the model
			*/
			std::string currentLine = "";
			Residue* extractedResidue = nullptr;
			while (!m_dataFile.eof())
			{
				//instead of creating a new pointer, we just recycle the one used
				//in the previous iteration
				extractedResidue = nullptr;
				
				getline(m_dataFile, currentLine);
				if (currentLine.find_first_not_of(' ') != std::string::npos)
				{
					if (ExtractResidue(currentLine, extractedResidue))
					{
						localProteinModel->AddResidue(extractedResidue);
						extractedResidue = nullptr;
					}
				}
				else
				{
					break;
				}
			}

			//build the protein model at this point
			localProteinModel->BuildProteinModel();
			return localProteinModel;
		}

		return nullptr;
	}

	ProteinModel* ProteinBuilder::CreateProteinModelFromJM()
	{
		ProteinModel* localProteinModel = nullptr;

		if (bFileLoaded)
		{
			localProteinModel = new ProteinModel();

			std::string residueLine = "";
			std::string secondaryStructureLine = "";
			Residue* residue = nullptr;

			//we read data from the file until we reach the end of the file
			while (!m_dataFile.eof())
			{
				//first we load the line of residue names
				bool foundResidues = false;
				while (!m_dataFile.eof() && !foundResidues)
				{
					getline(m_dataFile, residueLine);

					if (residueLine.find_first_not_of(" ") != std::string::npos)
					{
						foundResidues = true;
					}
				}

				//if we reached the end of the file before reading the secondary structure prediction
				//we break because we need both the secondary structure prediction and the residue name
				if (m_dataFile.eof())
				{
					break;
				}

				//current line has residues
				//next line contains the secondary structure that correspond to each residue
				std::string secondaryStructureLine;
				getline(m_dataFile, secondaryStructureLine);

				for (int i = 0; i < residueLine.length(); ++i)
				{
					if (residueLine[i] == ' ')
					{
						continue;
					}

					residue = new Residue();
					residue->SetType(residueLine[i]);
					residue->SetSecondaryStructure(secondaryStructureLine[i]);
					localProteinModel->AddResidue(residue);
				}
			}

			//build the protein model at this point
			localProteinModel->BuildProteinModel();
			return localProteinModel;
		}

		return nullptr;
	}

	bool ProteinBuilder::ExtractResidue(const std::string& informationString, Residue*& out_extractedResidue)
	{
		//The reason we take a pointer by reference is that we have a need to modify the pointer rather
		//than the object the pointer is pointing to

		//A valid line has 136 characters
		//"%5.5d%5.5d%c%c %c  %c %c%c%c%c%c%c%c%4.4d%4.4d%c%4.4d %11s%11s%11s%11s  %6.3f%6.1f%6.1f%6.1f%6.1f %6.1f %6.1f %6.1f"
		//  #  RESIDUE AA STRUCTURE BP1 BP2  ACC     N - H-->O    O-->H - N    N - H-->O    O-->H - N    TCO  KAPPA ALPHA  PHI   PSI    X - CA   Y - CA   Z - CA
		//	  1    1 A K              0   0   92      0, 0.0    39, -2.6     0, 0.0     2, -0.6   0.000 360.0 360.0 360.0 146.1    2.4   10.4    9.2
		if (informationString.length() != 136)
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
			std::string tempString = "";
			int tempNumber = 0;
			float tempFloat = 0.f;

			//the residue number line
			ReadFromLine(informationString, tempString, currentStringIndex, 5);
			GetIntFromString(tempNumber, tempString);
			out_extractedResidue->SetNumber(tempNumber);

			//the residue sequence number
			ReadFromLine(informationString, tempString, currentStringIndex, 5);
			GetIntFromString(tempNumber, tempString);
			out_extractedResidue->SetSeqNumber(tempNumber);

			//Not sure what the insertion code is for
			//it is likely to be empty in most cases
			ReadFromLine(informationString, tempString, currentStringIndex, 1);
			out_extractedResidue->SetInsertionCode(tempString[0]);

			//ChainID
			currentStringIndex += 1;

			//there is a space in between
			currentStringIndex += 1;

			//the shorthand form of the residue we are currently creating
			ReadFromLine(informationString, tempString, currentStringIndex, 1);
			out_extractedResidue->SetType(tempString[0]);

			//there are two spaces in between
			currentStringIndex += 2;

			//shorthand for the secondary structure the Define Secondary Structure of Proteins Algorithms gives us
			ReadFromLine(informationString, tempString, currentStringIndex, 1);
			out_extractedResidue->SetSecondaryStructure(tempString[0]);

			return true;
		}
	}
}