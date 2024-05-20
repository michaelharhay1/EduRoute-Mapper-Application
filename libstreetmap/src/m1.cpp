/* 
 * Copyright 2024 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

#include "m1.h"
#include "m1_helper/m1_helper.h"
#include "m1_helper/m1_globals.h"

#include <iostream>
#include <unordered_map>
#include <math.h>
#include <utility>
#include <algorithm>
#include <cmath>
#include <thread>
#include <functional>


/*********************************Global Variables**********************************/
StreetDatabaseAPIMembers *all_street_database_API_members;
OSMDatabaseAPIMembers *all_OSM_database_API_members;
float max_speed;

void load_max_speed();
/*********************************Function Definitions**********************************/
// loadMap will be called with the name of the file that stores the "layer-2"
// map data accessed through StreetsDatabaseAPI: the street and intersection 
// data that is higher-level than the raw OSM data). 
// This file name will always end in ".streets.bin" and you 
// can call loadStreetsDatabaseBIN with this filename to initialize the
// layer 2 (StreetsDatabase) API.
// If you need data from the lower level, layer 1, API that provides raw OSM
// data (nodes, ways, etc.) you will also need to initialize the layer 1 
// OSMDatabaseAPI by calling loadOSMDatabaseBIN. That function needs the 
// name of the ".osm.bin" file that matches your map -- just change 
// ".streets" to ".osm" in the map_streets_database_filename to get the proper
// name.
bool loadMap(std::string map_streets_database_filename) {
    
    // Loading in map data accessed through StreetsDatabaseAPI
    bool load_successful = loadStreetsDatabaseBIN(map_streets_database_filename); // Indicates whether the StreetsDatabase has loaded successfully

    if(!load_successful) {
        return false;
    }
    std::cout << "loadMap: " << map_streets_database_filename << std::endl;


    // Loading in map data accessed through OSMDatabaseAPI 
    std::string osm_database_filename = map_streets_database_filename; 
    osm_database_filename.replace(osm_database_filename.find(".streets"), 8, ".osm"); // Replaces the 8 characters of .streets with .osm
    load_successful = loadOSMDatabaseBIN(osm_database_filename); // Indicates whether the OSMDatabase has loaded successfully

    if(!load_successful) {
        return false;
    }
    std::cout << "loadMap: " << map_streets_database_filename << std::endl;

    // Load helper function calls that are related to the StreetsDatabase API
    all_street_database_API_members = new StreetDatabaseAPIMembers();

    // Load helper function calls that are related to the OSMDatabase API
    all_OSM_database_API_members = new OSMDatabaseAPIMembers();

    // load speed limits for M3
    load_max_speed();
    
    // Returns true when everything has been successfully loade
    return true;
}


// Close the map (if loaded).
// Speed Requirement --> moderate
void closeMap() { 
    
    // Clean-up map related data structures
    delete all_street_database_API_members;
    delete all_OSM_database_API_members;
    max_speed = 1;

    // Close OSM and Streets Databases
    closeStreetDatabase(); 
    closeOSMDatabase();
}


void load_max_speed(){
    max_speed = 1;

    // Iterate through street segments
    for(int streetSegmentIdx = 0; streetSegmentIdx < getNumStreetSegments(); streetSegmentIdx++) {
        float speed = getStreetSegmentInfo(streetSegmentIdx).speedLimit;

        if(speed > max_speed){
            max_speed = speed;
        }
    }
}

float get_max_speed(){
    return max_speed;
}

int get_street_seg_street_id(int street_seg_id){
    return all_street_database_API_members->street_seg_data[street_seg_id].streetID;
}

int get_street_seg_from(int street_seg_id){
    return all_street_database_API_members->street_seg_data[street_seg_id].from;

}

int get_street_seg_to(int street_seg_id){
    return all_street_database_API_members->street_seg_data[street_seg_id].to;
}

bool get_street_seg_one_way(int street_seg_id){
    return all_street_database_API_members->street_seg_data[street_seg_id].oneWay;
}

LatLon get_intersection_position(int intersection_id){
    return all_street_database_API_members->intersection_lat_lon[intersection_id];
}

// Returns the distance between two (lattitude,longitude) coordinates in meters.
// Speed Requirement --> moderate 
double findDistanceBetweenTwoPoints(LatLon point_1, LatLon point_2) { 
    
    // Value to be returned
    double distance;
     
    // Access each point's lat and lon values
    double p1_lat = kDegreeToRadian * point_1.latitude();
    double p1_lon = kDegreeToRadian * point_1.longitude();
    
    double p2_lat = kDegreeToRadian * point_2.latitude();
    double p2_lon = kDegreeToRadian * point_2.longitude();

    // lat_avg defined as (lat_1+lat_2)/2
    double lat_avg = (p1_lat + p2_lat)/2;

    // Express coords in meters
    double x_1 = kEarthRadiusInMeters*p1_lon*cos(lat_avg); // x = R * lon * cos(lat_avg)
    double y_1 = kEarthRadiusInMeters*p1_lat; // y = R * lat

    double x_2 = kEarthRadiusInMeters*p2_lon*cos(lat_avg); // x = R * lon * cos(lat_avg)
    double y_2 = kEarthRadiusInMeters*p2_lat; // y = R * lat

    // Apply Pythagorean theorem to find distance between the (x,y) pairs
    distance = sqrt(pow((y_2 - y_1),2) + pow((x_2 - x_1),2));

    // Return final distance in meters
    return distance;
}
 

// Returns the length of the given street segment in meters.
// Speed Requirement --> moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id) {  
    return all_street_database_API_members->street_seg_lengths[street_segment_id];
}


// Returns the travel time to drive from one end of a street segment
// to the other, in seconds, when driving at the speed limit.
// Note: (time = distance/speed_limit)
// Speed Requirement --> high
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id) {
    
    // Return speed of street segment
    return all_street_database_API_members->street_segment_speed[street_segment_id];
}


// Returns the angle (in radians) that would result as you exit
// src_street_segment_id and enter dst_street_segment_id, if they share an
// intersection.
// If a street segment is not completely straight, use the last piece of the
// segment closest to the shared intersection.
// If the two street segments do not share an intersection, return a constant
// NO_ANGLE, which is defined above.
// Speed Requirement --> none
double findAngleBetweenStreetSegments(StreetSegmentIdx src_street_segment_id, StreetSegmentIdx dst_street_segment_id) {

    StreetSegmentInfo srcStreetSegment = getStreetSegmentInfo(src_street_segment_id);
    StreetSegmentInfo dstStreetSegment = getStreetSegmentInfo(dst_street_segment_id);

    LatLon position1Latlon;
    LatLon position2Latlon;
    LatLon position3Latlon;

    // Point 1: Where the source and destination street segments intersect
    if(srcStreetSegment.from == dstStreetSegment.from || srcStreetSegment.from == dstStreetSegment.to) {
        position1Latlon = getIntersectionPosition(srcStreetSegment.from);
    }
    else if(srcStreetSegment.to == dstStreetSegment.from || srcStreetSegment.to == dstStreetSegment.to) {
        position1Latlon = getIntersectionPosition(srcStreetSegment.to);
    }
    else{
        return NO_ANGLE;
    }

       
    // Point 2: Closest position on the source street segement
    if(getIntersectionPosition(srcStreetSegment.from) == position1Latlon) {

        // No curvature: Closest point is the end point of street segement
        if(srcStreetSegment.numCurvePoints == 0){
            position2Latlon = getIntersectionPosition(srcStreetSegment.to);     
        }

        // Curvature: Closest point is the first curve point
        else{
            position2Latlon = getStreetSegmentCurvePoint(0, src_street_segment_id);
        }
    }
    else{

        // No curvature: Closest point is the end point of street segement
        if(srcStreetSegment.numCurvePoints == 0) {
            position2Latlon = getIntersectionPosition(srcStreetSegment.from);    
        }

        // Curvature: Closest point is the last curve point
        else{
            position2Latlon = getStreetSegmentCurvePoint(srcStreetSegment.numCurvePoints - 1, src_street_segment_id);
        }
    }

    // Point 3: Closest position on the destination street segement
    if(getIntersectionPosition(dstStreetSegment.from) == position1Latlon) {
        if(dstStreetSegment.numCurvePoints == 0) {
            position3Latlon = getIntersectionPosition(dstStreetSegment.to);  
        }
        else{
            position3Latlon = getStreetSegmentCurvePoint(0, dst_street_segment_id);
        }
    }
    else{
        if(dstStreetSegment.numCurvePoints == 0) {
            position3Latlon = getIntersectionPosition(dstStreetSegment.from);    
        }
        else{
            position3Latlon = getStreetSegmentCurvePoint(dstStreetSegment.numCurvePoints - 1, dst_street_segment_id);
        }
    }

    // Calculate distance between 3 points
    double distance1to2 = findDistanceBetweenTwoPoints(position1Latlon, position2Latlon);
    double distance2to3 = findDistanceBetweenTwoPoints(position2Latlon, position3Latlon);
    double distance1to3 = findDistanceBetweenTwoPoints(position3Latlon, position1Latlon);

    // Use cosine law to calculate angle between 2 intersections
    double numerator = pow(distance1to2, 2) + pow(distance1to3, 2) - pow(distance2to3, 2);
    double denominator = (2 * distance1to2 * distance1to3);
    
    double ratio = numerator/denominator;

    // Make sure value is between -1 and 1
    if(fabs(ratio) > 1){
        return 0;
    }

    // Calculate angle and return
    double angle = acos(ratio);
    return M_PI - angle;
}

// Returns true if the two intersections are directly connected, meaning you can legally
// drive from the first intersection to the second using only one street segment.
// Speed Requirement --> moderate
bool intersectionsAreDirectlyConnected(std::pair<IntersectionIdx, IntersectionIdx> intersection_ids) {
 
    // Get a list of street segments related to the first intersection
    std::vector<StreetSegmentIdx> firstIntersectionStreetSegments = all_street_database_API_members->intersection_street_segments[intersection_ids.first];

    // Loop through segments and check intersections
    for(int streetSegmentIndex = 0; streetSegmentIndex< firstIntersectionStreetSegments.size(); streetSegmentIndex ++){
        StreetSegmentInfo streetSegment = getStreetSegmentInfo(firstIntersectionStreetSegments[streetSegmentIndex]);
        
        // If street segment has an equivalent intersection ID to the second intersection return true
        if(streetSegment.from == intersection_ids.second || streetSegment.to == intersection_ids.second){
            return true;
        }
    }

    return false;
}


// Returns the geographically nearest intersection (i.e. as the crow flies) to
// the given position.
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position) {
    IntersectionIdx closest = 0;
    
    // Iterate through list of intersections
    for (int intersectionIndex = 1; intersectionIndex < getNumIntersections(); intersectionIndex++) {

        // If my_position is closer to ith intersection than "closest", set closest to ith intersection
        if (findDistanceBetweenTwoPoints(my_position, getIntersectionPosition(intersectionIndex)) < findDistanceBetweenTwoPoints(my_position, getIntersectionPosition(closest))) {
            closest = intersectionIndex;
        }
    }
    return closest;
}


// Returns the street segments that connect to the given intersection.
// Speed Requirement --> high
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id) {
    return all_street_database_API_members->intersection_street_segments[intersection_id];
}


// Returns all intersections along the given street.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id) { 
    
    // Load all intersections of each street into street_intersections in loadMap(), return vectors here
    return all_street_database_API_members->street_intersections[street_id];
}


// Return all intersection ids at which the two given streets intersect. 
// This function will typically return one intersection id for streets that
// intersect and a length 0 vector for streets that do not. For unusual curved
// streets it is possible to have more than one intersection at which two
// streets cross.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(std::pair<StreetIdx, StreetIdx> street_ids) {

    // Return vector for intersections in between 2 streets
    std::vector<IntersectionIdx> intersectionsOfTwoStreets;

    // Load in a vector of intersections related to street
    std::vector<IntersectionIdx> intersectionStreet1 = all_street_database_API_members->street_intersections[street_ids.first];
    std::vector<IntersectionIdx> intersectionStreet2 = all_street_database_API_members->street_intersections[street_ids.second];

    // Loop through each intersection in street 1 & 2 
    for (int indexStreet1 = 0; indexStreet1 < intersectionStreet1.size(); indexStreet1++) {
        for (int indexStreet2 = 0; indexStreet2 < intersectionStreet2.size(); indexStreet2++) {

            // If intersections are equal, add them to return value
            if (intersectionStreet1[indexStreet1] == intersectionStreet2[indexStreet2]) { 
                intersectionsOfTwoStreets.push_back(intersectionStreet1[indexStreet1]);
            }
        }
    }   

    return intersectionsOfTwoStreets;
}


// Returns all street ids corresponding to street names that start with the
// given prefix.
// The function should be case-insensitive to the street prefix.
// The function should ignore spaces.
//  For example, both "bloor " and "BloOrst" are prefixes to 
//  "Bloor Street East".
// If no street names match the given prefix, this routine returns an empty
// (length 0) vector.
// You can choose what to return if the street prefix passed in is an empty
// (length 0) string, but your program must not crash if street_prefix is a
// length 0 string.
// Speed Requirement --> high
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix) {

    // Make the street_prefix lower case and remove all the spaces
    std::transform(street_prefix.begin(), street_prefix.end(), street_prefix.begin(), ::tolower);
    street_prefix.erase(std::remove_if(street_prefix.begin(), street_prefix.end(), ::isspace), street_prefix.end());

    // Access the global map with the street_prefix to get the vector of street ids
    return all_street_database_API_members->street_name_map[street_prefix];
}


// Returns the length of a given street in meters.
// Speed Requirement --> high
double findStreetLength(StreetIdx street_id) {
   
   // Load all street lengths in loadMap(), return vector here
   return all_street_database_API_members->street_lengths[street_id];
}


// Returns the nearest point of interest of the given name (e.g. "Starbucks")
// to the given position.
// Speed Requirement --> none 
POIIdx findClosestPOI(LatLon my_position, std::string poi_name) { 

    // Initialize the closest POI Index to 0
    POIIdx closestPOIIdx = 0;

    // Initialize shortestDistance with the first value found
    for (POIIdx curPOIIdx = 0; curPOIIdx < getNumPointsOfInterest(); curPOIIdx++) {
        
        // Check if names match
        if (getPOIName(curPOIIdx) == poi_name) {
            
            // Set this POI as the POI we continue to compare to
            closestPOIIdx = curPOIIdx;

            // Exit once the first POI is found
            break;
        }
    }

    // Set shortestDistance to the distance between given position and current closestPOIIdxs
    double shortestDistance = findDistanceBetweenTwoPoints(my_position, getPOIPosition(closestPOIIdx));

    // Loop through the rest of the POIs (starting from first POI found) to find the shortest possible distance
    for (POIIdx curPOIIdx = closestPOIIdx + 1; curPOIIdx < getNumPointsOfInterest(); curPOIIdx++) {
        
        // Check if names match
        if (getPOIName(curPOIIdx) == poi_name) {
            
            // Calculate distance between my position and current POI
            double distance = findDistanceBetweenTwoPoints(my_position, getPOIPosition(curPOIIdx));

            // Update closet POI if this one is closer
            if (distance < shortestDistance) {
                closestPOIIdx = curPOIIdx;
                shortestDistance = distance;
            }
        }
    }

    // After looping through all the POIs, return the POIIdx of the shortest distance found
    return closestPOIIdx; 
}

// Returns the area of the given closed feature in square meters.
// Assume a non self-intersecting polygon (i.e. no holes).
// Return 0 if this feature is not a closed polygon.
// Speed Requirement --> moderate
double findFeatureArea(FeatureIdx feature_id) {
    
    // Initialize variables
    double area = 0;
    double currentX, currentY, nextX, nextY;

    // If number of feature points < 3 OR if first and last points don't match, return 0
    if (getNumFeaturePoints(feature_id) < 3 || 
        (getFeaturePoint(0, feature_id).latitude() != getFeaturePoint(getNumFeaturePoints(feature_id) - 1, feature_id).latitude() &&
        getFeaturePoint(0, feature_id).longitude() != getFeaturePoint(getNumFeaturePoints(feature_id) - 1, feature_id).longitude())) {
        
        area = 0;
        return area;
    }

    // Set reference point to first point
    LatLon referencePoint = getFeaturePoint(0, feature_id); 

    // Use numerical integration, iterate through points
    for (int featurePointIdx = 0; featurePointIdx < getNumFeaturePoints(feature_id); featurePointIdx++) {
        
        // Set point 1 (current) and point 2 (next)
        LatLon current = getFeaturePoint(featurePointIdx, feature_id);
        LatLon next;
        
        // If on last iteration, set next to 0th point, else set to point after current
        if (featurePointIdx == (getNumFeaturePoints(feature_id) - 1)) {
            next = getFeaturePoint(0, feature_id);
        }
        else {
            next = getFeaturePoint(featurePointIdx + 1, feature_id);
        }

        // Convert to meters, with respect to the reference point
        std::pair<std::pair<double,double>, std::pair<double,double>> metersCurrent;
        std::pair<std::pair<double,double>, std::pair<double,double>> metersNext;

        metersCurrent = coordsToMeters(referencePoint, current);
        metersNext = coordsToMeters(referencePoint, next);
        

        currentX =  metersCurrent.second.first;
        currentY =  metersCurrent.second.second;
        nextX =  metersNext.second.first;
        nextY =  metersNext.second.second;

        // Find delta Y and avg X
        double deltaY = nextY - currentY;
        double avgX = (nextX + currentX) / 2;

        // Use formula (y2 - y1) * ((x2 + x1) / 2) = deltaY - avgX
        // This takes the average x (longitude) value of the two points,
        // multiplies by the difference in y (latitude) values, then takes absolute value
        area += deltaY * avgX;
    }
    return fabs(area);
}

// Returns the length of the OSMWay that has the given OSMID, in meters.
// To implement this function you will have to  access the OSMDatabaseAPI.h
// functions.
// Speed Requirement --> high
double findWayLength(OSMID way_id) { 

    // Access global map with the way_id to get the length of the way
    return all_OSM_database_API_members->wayIDs_to_wayLength_map[way_id] ;
}


// Return the value associated with this key on the specified OSMNode.
// If this OSMNode does not exist in the current map, or the specified key is
// not set on the specified OSMNode, return an empty string.
// Speed Requirement --> high
std::string getOSMNodeTagValue(OSMID osm_id, std::string key) { 
    // Access global map with the osm_id and the key to get the associated tag value
    return (all_OSM_database_API_members->OSMIDs_to_tags_map[osm_id])[key];;
}