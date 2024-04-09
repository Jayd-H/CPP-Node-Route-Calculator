// #######################################################################################
// ##                                                                                   ##
// ##  This main.cpp file WILL BE REPLACED during the marking procedure.                ##
// ##  Therefore all of your implementation code MUST be in the Navigation class,       ##
// ##  or any other new classes that you create and use inside of the Navigation class. ##
// ##                                                                                   ##
// #######################################################################################

#include <iostream>
#include <iomanip>
#include <chrono>

#include "Navigation.h"

using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
using std::chrono::duration_cast;

int main(int, char**) {

	// Build Navigation
	Navigation nav;
	auto start = high_resolution_clock::now();
	if (nav.BuildNetwork("Places.csv", "Links.csv")) {
		std::cout << std::fixed << std::setprecision(1) << "BuildNetwork - " 
			<< duration_cast<microseconds>(high_resolution_clock::now() - start).count()
			<< " microseconds" << std::endl;
	}
	else {
		std::cout << "\n*** Error *** BuildNetwork" << std::endl;
	}

	// Process commands
	std::ifstream fin("commands.txt");
	while (!fin.eof())	{
		std::string command;
		std::getline(fin, command);

		start = high_resolution_clock::now();
		if (nav.ProcessCommand(command)) {
			const auto start = high_resolution_clock::now();
			std::cout << command << std::fixed << std::setprecision(1) << " - " 
				<< duration_cast<microseconds>(high_resolution_clock::now() - start).count()
				<< " microseconds" << std::endl;
		}
		else {
			std::cout << "\n*** Error *** " << command << std::endl;
		}
	}
}