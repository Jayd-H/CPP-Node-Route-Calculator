#include <iostream>
#include <iomanip>
#include <sstream>

#include "Navigation.h"
#include "Utility.h"


Navigation::Navigation() : _outFile("Output.txt") {
	// Add your code here
}

Navigation::~Navigation() {
	// Add your code here
}

bool Navigation::ProcessCommand(const std::string& commandString) {
	std::istringstream inString(commandString);
	std::string command;
	inString >> command;

	// Add your code here

	return false;
}

bool Navigation::BuildNetwork(const std::string &fileNamePlaces, const std::string &fileNameLinks) {
	std::ifstream finPlaces(fileNamePlaces);
	std::ifstream finLinks(fileNameLinks);
	if (finPlaces.fail() || finLinks.fail()) 
		return false;

	// ****** Add your code here *******

	// example code of using the LLtoUTM function from ACW_Wrapper to convert 
	// latitude and longitude values into x and y global coordinates:
	const double latitude = 53.770, longitude = -0.368;
	double x, y;
	Utility::LLtoUTM(latitude, longitude, x, y);
	
	return true;
}

// Add your code here
