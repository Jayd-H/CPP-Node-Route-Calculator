#pragma once

class Utility final
{
public:
	static void LLtoUTM(const double Lat, const double Long, double& UTMNorthing, double& UTMEasting);
	static double arcLength(const double startNorth, const double startEast, const double endNorth, const double endEast);
};

