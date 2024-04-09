#include "utility.h"
#include <math.h>

// ACW specific functions

void Utility::LLtoUTM(const double Lat, const double Long, double& UTMNorthing, double& UTMEasting)
{
	//converts lat/long to UTM coords.  Equations from USGS Bulletin 1532 
	//East Longitudes are positive, West longitudes are negative. 
	//North latitudes are positive, South latitudes are negative
	//Lat and Long are in decimal degrees
	//Original code by Chuck Gantz- chuck.gantz@globalstar.com (http://www.gpsy.com/gpsinfo/geotoutm/)
	//Adapted by Darren McKie / Warren Viant

	constexpr double PI = 3.14159265;
	constexpr double DEG2RAD = PI / 180.0;

	constexpr double a = 6378137;
	constexpr double eccSquared = 0.00669438f;
	constexpr double k0 = 0.9996f;

	//Make sure the longitude is between -180.00 .. 179.9
	const double LongTemp = (Long + 180) - static_cast<int>((Long + 180) / 360) * 360 - 180; // -180.00 .. 179.9;

	const double LatRad = Lat * DEG2RAD;
	const double LongRad = LongTemp * DEG2RAD;
	constexpr int ZoneNumber = 30;

	constexpr double LongOrigin = (ZoneNumber - 1) * 6.0f - 180 + 3;  //+3 puts origin in middle of zone
	constexpr double LongOriginRad = LongOrigin * DEG2RAD;

	constexpr double eccPrimeSquared = (eccSquared) / (1 - eccSquared);

	const double N = a / sqrt(1 - eccSquared * sin(LatRad) * sin(LatRad));
	const double T = tan(LatRad) * tan(LatRad);
	const double C = eccPrimeSquared * cos(LatRad) * cos(LatRad);
	const double A = cos(LatRad) * (LongRad - LongOriginRad);

	const double M = a * ((1 - eccSquared / 4 - 3 * eccSquared * eccSquared / 64 - 5 * eccSquared * eccSquared * eccSquared / 256) * LatRad
		- (3 * eccSquared / 8 + 3 * eccSquared * eccSquared / 32 + 45 * eccSquared * eccSquared * eccSquared / 1024) * sin(2 * LatRad)
		+ (15 * eccSquared * eccSquared / 256 + 45 * eccSquared * eccSquared * eccSquared / 1024) * sin(4 * LatRad)
		- (35 * eccSquared * eccSquared * eccSquared / 3072) * sin(6 * LatRad));

	UTMEasting = (k0 * N * (A + (1 - T + C) * A * A * A / 6
		+ (5 - 18 * T + T * T + 72 * C - 58 * eccPrimeSquared) * A * A * A * A * A / 120)
		+ 500000.0f);

	UTMNorthing = (k0 * (M + N * tan(LatRad) * (A * A / 2 + (5 - T + 9 * C + 4 * C * C) * A * A * A * A / 24
		+ (61 - 58 * T + T * T + 600 * C - 330 * eccPrimeSquared) * A * A * A * A * A * A / 720)));

	if (Lat < 0)
		UTMNorthing += 10000000.0f; //10000000 meter offset for southern hemisphere
}

double Utility::arcLength(const double startNorth, const double startEast, const double endNorth, const double endEast) {
	double UTMNorthingStart;
	double UTMEastingStart;
	double UTMNorthingEnd;
	double UTMEastingEnd;

	LLtoUTM(startNorth, startEast, UTMNorthingStart, UTMEastingStart);
	LLtoUTM(endNorth, endEast, UTMNorthingEnd, UTMEastingEnd);

	const double dNorth = UTMNorthingEnd - UTMNorthingStart;
	const double dEast = UTMEastingEnd - UTMEastingStart;

	return sqrt(dNorth * dNorth + dEast * dEast) * 0.001;
}