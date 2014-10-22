#include "ProteinUtilities.hpp"

#include <iostream>

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
			firstNonWhiteSpace = in_string.find_first_not_of(whitespaces);
			lastNonWhiteSpace = in_string.find_last_not_of(whitespaces);
			break;
		}

		if (lastNonWhiteSpace != std::string::npos)
		{
			in_string = in_string.substr(0, lastNonWhiteSpace+1);
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

	void ShowVectorVals(const std::string& label, const std::vector<double>& values)
	{
		std::cout << label.c_str() << " ";

		for (unsigned i = 0; i < values.size(); ++i)
		{
			std::cout << values[i] << " ";
		}

		std::cout << std::endl;
	}

	void PrintVectorVals(const std::string& label, const std::vector<double>& values, std::ostream& outFile)
	{
		outFile << label.c_str() << " ";

		for (unsigned i = 0; i < values.size(); ++i)
		{
			outFile << values[i] << " ";
		}

		outFile << "\n";
	}
}