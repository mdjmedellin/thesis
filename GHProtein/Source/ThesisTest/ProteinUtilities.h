#pragma once

namespace GHProtein
{
	template< typename OutType >
	void GetTypeFromString(OutType& out_variable, FString in_stringLine)
	{
		TTypeFromString<OutType>::FromString(out_variable, *(in_stringLine.Trim().TrimTrailing()));
	}

	template< typename OutType >
	OutType GetMax(const OutType& val1, const OutType& val2)
	{
		return (val1 < val2 ? val2 : val1);
	}

	template< typename OutType >
	OutType GetMin(const OutType& val1, const OutType& val2)
	{
		return (val1 < val2 ? val1 : val2);
	}

	//void GetIntFromString(int& out_int, FString in_stringLine);
	//void GetFloatFromString(float& out_float, FString in_stringLine);

	/**
	* Split a buffer of characters into lines
	*/
	template <typename CharType>
	void SplitLines(TArray<FString>& OutArray, const CharType* InBuffer, uint32 BufferSize, ANSICHAR Delim = '\n')
	{
		FString WorkingLine;
		for (uint32 i = 0; i < BufferSize; ++i)
		{
			if (InBuffer[i] == Delim)
			{
				if (WorkingLine.Len())
				{
					OutArray.Add(WorkingLine);
					WorkingLine = TEXT("");
				}
			}
			else
			{
				WorkingLine += static_cast<TCHAR>(InBuffer[i]);
			}
		}

		if (WorkingLine.Len())
		{
			OutArray.Add(WorkingLine);
		}
	}

	bool IsWhiteSpace(TCHAR Char);

	void ReadFromLine(const FString& lineToReadFrom, FString& out_substringDestination, int& startingIndex, int charactersToRead);
}