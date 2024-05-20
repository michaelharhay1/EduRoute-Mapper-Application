/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 15/03/2024
 *
 * Description: Contains functions for calculating city information, such as 
 * average latitude and conversion between lat/lon and x/y. Defines functions 
 * conversions and average lat.
 */

#include "city_information.h"
#include "StreetsDatabaseAPI.h"

#include <limits.h>

// Global variables for storing city information
double lat_avg, lat_avg_constant; 
double lat_avg_const;
double max_lat, min_lat, max_lon, min_lon;

// Conversion functions between lat/lon and x/y
double x_from_lon(double lon) {
    return lon * kDegreeToRadian * kEarthRadiusInMeters * lat_avg_constant;
}

double y_from_lat(double lat) {
    return kEarthRadiusInMeters * kDegreeToRadian * lat;
}

double lon_from_x(double x) {
    return x / (kDegreeToRadian * kEarthRadiusInMeters * lat_avg_constant);
}

double lat_from_y(double y) {
    return y / (kEarthRadiusInMeters * kDegreeToRadian);
}


// Calculate average latitude
void calculateLatAvg(){
    
    // Find map bounds
    max_lat = getIntersectionPosition(0).latitude(); 
    min_lat = max_lat;
    max_lon = getIntersectionPosition(0).longitude();
    min_lon = max_lon;


    // Loop through every intersection and find the max and min longitude and latitude
    for (int id = 0; id < getNumIntersections(); id++) {  
        LatLon position = getIntersectionPosition(id); 
        
        double lon = position.longitude();
        double lat = position.latitude();

        max_lat = std::max(max_lat, lat);
        min_lat = std::min(min_lat, lat);
        max_lon = std::max(max_lon, lon);
        min_lon = std::min(min_lon, lon);
    }

    // Returen the lat_avg constant
    lat_avg = (max_lat + min_lat) / 2;
    lat_avg_constant = std::cos(lat_avg * kDegreeToRadian);
}
