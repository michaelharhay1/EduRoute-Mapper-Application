/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 15/03/2024
 *
 * Description: Contains functions declarations for calculating city information, 
 * such as average latitude and conversion between lat/lon and x/y and 
 * global variable delcarations for city information and pointers to 
 * graphics data structures.
*/


#include "graphics_entities.h"

#ifndef CITY_INFORMATION_H
#define CITY_INFORMATION_H

// City Information global variables
extern double lat_avg; 
extern double lat_avg_const;
extern double max_lat, min_lat, max_lon, min_lon;

// Graphics data structures
extern IntersectionsGraphicsData *Intersections_graphics_data;
extern StreetsGraphicsData *Streets_graphics_data;
extern POIGraphicsData *POIs_graphics_data;
extern FeaturesGraphicsData *Features_graphics_data;

// Calculations functions declarations
double x_from_lon(double lon);
double y_from_lat(double lat);
double lon_from_x(double x);
double lat_from_y(double y);
void calculateLatAvg();

#endif
