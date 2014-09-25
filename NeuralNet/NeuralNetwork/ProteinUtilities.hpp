#pragma once
#ifndef PROTEIN_UTILITIES_H
#define PROTEIN_UTILITIES_H

#include <iostream>
#include <vector>

namespace GHProtein
{
	enum ETrim
	{
		TrimFront,
		TrimBack,
		TrimBoth
	};

	static const double INVERSE_RAND_MAX = 1.0 / double(RAND_MAX);
	
	double RandZeroToN(double maxValue = 1.0);
	double RandNToN(double maxValue = 1.0, double minValue = 0.0);

	//String parsing utilities
	static std::string whitespaces(" \t\f\v\n\r");

	void TokenizeString(std::vector<std::string>& out_list, const std::string& source,
							const char* delimiter = " ", bool keepEmpty = false);

	void Trim(std::string& in_string, ETrim trimDetails = ETrim::TrimBoth);

	void GetIntFromString(int& out_int, const std::string& in_stringLine);
	void GetFloatFromString(float& out_float, const std::string& in_stringLine);

	void ReadFromLine(const std::string& lineToReadFrom, std::string& out_substringDestination, int& startingIndex, int charactersToRead);

	void ShowVectorVals(const std::string& label, const std::vector<double>& values);
}

#endif