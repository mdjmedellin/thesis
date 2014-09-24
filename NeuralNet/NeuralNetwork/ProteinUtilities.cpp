#include "ProteinUtilities.hpp"

namespace GHProtein
{
	void Trim(std::string& in_string, ETrim trimDetails)
	{
		int firstNonWhiteSpace = std::string::npos;
		int lastNonWhiteSpace = std::string::npos;

		switch (trimDetails)
		{
		case ETrim::TrimFront:
			
			firstNonWhiteSpace = in_string.find_first_not_of(whitespaces);
			break;

		case ETrim::TrimBack:

			lastNonWhiteSpace = in_string.find_last_not_of(whitespaces);
			break;

		default:
			//do both
			break;
		}

		if (lastNonWhiteSpace != std::string::npos)
		{
			in_string = in_string.substr(0, lastNonWhiteSpace);
		}
		if (firstNonWhiteSpace != std::string::npos)
		{
			in_string = in_string.substr(firstNonWhiteSpace);
		}
	}

	void TokenizeString(std::vector<std::string>& out_list, const std::string& source,
		const char* delimiter, bool keepEmpty)
	{
		size_t prev = 0;
		size_t next = 0;
		out_list.clear();

		while ((next = source.find_first_of(delimiter, prev)) != std::string::npos)
		{
			if (keepEmpty || (next - prev != 0))
			{
				out_list.push_back(source.substr(prev, next - prev));
			}
			prev = next + 1;
		}

		if (prev < source.size())
		{
			out_list.push_back(source.substr(prev));
		}
	}

	void GetIntFromString(int& out_int, const std::string& in_stringLine)
	{
		out_int = std::atoi(in_stringLine.c_str());
	}

	void GetFloatFromString(float& out_float, const std::string& in_stringLine)
	{
		out_float = std::atof(in_stringLine.c_str());
	}

	void ReadFromLine(const std::string& lineToReadFrom, std::string& out_substringDestination, int& startingIndex, int charactersToRead)
	{
		out_substringDestination = lineToReadFrom.substr(startingIndex, charactersToRead);
		startingIndex += charactersToRead;
	}

	double RandZeroToN(double maxValue)
	{
		return (rand() * INVERSE_RAND_MAX * maxValue);
	}

	double RandNToN(double maxValue, double minValue)
	{
		return (rand() * INVERSE_RAND_MAX * (maxValue - minValue)) + minValue;
	}
}