#pragma once

// hacky method to easily print
#include <maya/MGlobal.h>

class HackPrint
{
public:
	HackPrint() {};
	~HackPrint() {};

	static void print(const char * text)
	{
		MGlobal::displayInfo(text);
	}

	static void print(MString& text)
	{
		MGlobal::displayInfo(text);
	}

	static void print(std::string& text)
	{
		MGlobal::displayInfo(text.c_str());
	}
};

