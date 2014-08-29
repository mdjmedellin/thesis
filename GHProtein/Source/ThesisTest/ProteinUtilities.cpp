#include "ThesisTest.h"
#include "ProteinUtilities.h"

namespace GHProtein
{
	/*void GetIntFromString(int& out_int, FString in_stringLine)
	{
		TTypeFromString<int>::FromString(out_int, *(in_stringLine.Trim().TrimTrailing()));
	}

	void GetFloatFromString(float& out_float, FString in_stringLine)
	{
		TTypeFromString<float>::FromString(out_float, *(in_stringLine.Trim().TrimTrailing()));
	}*/

	/** Checks if the passed character is a whitespace character */
	bool IsWhiteSpace(TCHAR Char)
	{
		// Whitespace will be any character that is not a common printed ASCII character (and not space/tab)
		if (Char == TCHAR(' ') ||
			Char == TCHAR('\t') ||
			Char < 32) // ' '
		{
			return true;
		}
		return false;
	}

	void ReadFromLine(const FString& lineToReadFrom, FString& out_substringDestination, int& startingIndex, int charactersToRead)
	{
		out_substringDestination = lineToReadFrom.Mid(startingIndex, charactersToRead);
		startingIndex += charactersToRead;
	}
}