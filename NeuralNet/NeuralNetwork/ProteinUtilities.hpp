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
	
	static std::string whitespaces(" \t\f\v\n\r");

	void TokenizeString(std::vector<std::string>& out_list, const std::string& source,
							const char* delimiter = " ", bool keepEmpty = false);

	void Trim(std::string& in_string, ETrim trimDetails = ETrim::TrimBoth);

	void GetIntFromString(int& out_int, const std::string& in_stringLine);
	void GetFloatFromString(float& out_float, const std::string& in_stringLine);

	void ReadFromLine(const std::string& lineToReadFrom, std::string& out_substringDestination, int& startingIndex, int charactersToRead);
}

#endif