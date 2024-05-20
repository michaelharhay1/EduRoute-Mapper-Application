/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 08/02/2024
 *
 * Description: Contains StreetDatabaseAPIMembers and OSMDatabaseAPIMembers constructors and destructors
*/

#include "m1_helper.h"
#include "m1.h"
#include <iostream>
#include <unordered_map>
#include <math.h>
#include <utility>
#include <cmath>
#include <algorithm>


/**************************StreetDatabaseAPI Helper Function Definitions***************************/

// StreetDatabaseAPIMembers destructor
StreetDatabaseAPIMembers::StreetDatabaseAPIMembers(){
    loadLengthOfStreetSegments();
    loadLengthOfStreet();
    loadIntersectionsOfStreets();
    loadStreetSegmentsOfIntersections();
    loadSpeedOfStreetSegments();
    loadMapOfStreetNames();
    loadDataofStreetSegs();
    loadIntersectionLatlon();
}

// StreetDatabaseAPIMembers destructor
StreetDatabaseAPIMembers::~StreetDatabaseAPIMembers(){
    street_segment_speed.clear();
    intersection_street_segments.clear();
    street_intersections.clear();
    street_name_map.clear();
    street_lengths.clear();
    street_seg_lengths.clear();
    street_seg_data.clear();
    intersection_lat_lon.clear();
}


void StreetDatabaseAPIMembers::loadDataofStreetSegs() {
    street_seg_data.resize(getNumStreetSegments());

    // Iterate through street segments
    for(int streetSegmentIdx = 0; streetSegmentIdx < getNumStreetSegments(); streetSegmentIdx++) {
        StreetSegmentInfo street_seg = getStreetSegmentInfo(streetSegmentIdx);

        // Calculate street segment speed by dividing length by speed limit, store to street_segment_speed
        street_seg_data[streetSegmentIdx].from =  street_seg.from;
        street_seg_data[streetSegmentIdx].to =  street_seg.to;
        street_seg_data[streetSegmentIdx].numCurvePoints =  street_seg.numCurvePoints;
        street_seg_data[streetSegmentIdx].oneWay =  street_seg.oneWay;
        street_seg_data[streetSegmentIdx].streetID =  street_seg.streetID;
        street_seg_data[streetSegmentIdx].speedLimit =  street_seg.speedLimit;
    }
}

void StreetDatabaseAPIMembers::loadIntersectionLatlon() {
    // Resize intersection_street_segments to correct size
    intersection_lat_lon.resize(getNumIntersections());

    // Iterate through intersections
    for(int intersectionIdx = 0; intersectionIdx < getNumIntersections(); intersectionIdx++) {
        intersection_lat_lon[intersectionIdx] = getIntersectionPosition(intersectionIdx);
    }
}

// Fills up global variable street_name_map by breaking down every street name into substrings
// and for each substring having a vector of streetIDs that possess that substring.
void StreetDatabaseAPIMembers::loadMapOfStreetNames() {

    // Iterate through all streets
    for (StreetIdx curStreetId = 0; curStreetId < getNumStreets(); curStreetId++) {
        
        // Access the street name, change it to lowercase, and remove the spaces
        std::string streetName = getStreetName(curStreetId); 
        std::transform(streetName.begin(), streetName.end(), streetName.begin(), ::tolower); 
        streetName.erase(std::remove_if(streetName.begin(), streetName.end(), ::isspace), streetName.end()); 
        
        // For each substring of the name as the key, add the ID to the map as the value of the map
        for (int substringLength = 1; substringLength <= streetName.length(); substringLength++) {
            std::string subStreetName = streetName.substr(0, substringLength);
            street_name_map[subStreetName].push_back(curStreetId); 
        }    
    }
}


// Load speed of each street, store in street_segment_speed    
void StreetDatabaseAPIMembers::loadSpeedOfStreetSegments() {
    
    // Resize street_segment_speed to correct size
    street_segment_speed.resize(getNumStreetSegments());

    // Iterate through street segments
    for(int streetSegmentIdx = 0; streetSegmentIdx < getNumStreetSegments(); streetSegmentIdx++) {
        
        // Calculate street segment speed by dividing length by speed limit, store to street_segment_speed
        street_segment_speed[streetSegmentIdx] = street_seg_lengths[streetSegmentIdx] / getStreetSegmentInfo(streetSegmentIdx).speedLimit;
    }
}



// Load all street segments of each intersection, store in intersection_street_segments
void StreetDatabaseAPIMembers::loadStreetSegmentsOfIntersections(){
    
    // Resize intersection_street_segments to correct size
    intersection_street_segments.resize(getNumIntersections());

    // Iterate through intersections
    for(int intersectionIdx = 0; intersectionIdx < getNumIntersections(); intersectionIdx++) {
        
        // Iterate through street segments for current intersection
        for(int streetSegmentIdx = 0; streetSegmentIdx < getNumIntersectionStreetSegment(intersectionIdx); streetSegmentIdx++) {
            
            // Store street segments to corresponding place in intersection_street_segments
            intersection_street_segments[intersectionIdx].push_back(getIntersectionStreetSegment(streetSegmentIdx, intersectionIdx));
        }
    }
}


// Load all the intersection of each street, store in street_intersections
void StreetDatabaseAPIMembers::loadIntersectionsOfStreets() {
    
    // Resize street_intersections to correct size
    street_intersections.resize(getNumStreets());

    // Create a vector of maps to track duplicates
    std::vector <std::unordered_map<int,bool>> inters_str_map;
    inters_str_map.resize(getNumStreets());

    // Iterate through street segements, add them to their respective street
    for (int str_seg_idx = 0; str_seg_idx < getNumStreetSegments(); str_seg_idx++) {
        StreetSegmentInfo street_seg = getStreetSegmentInfo(str_seg_idx);

        // Intersection not associated with street yet
        if(inters_str_map[street_seg.streetID].find(street_seg.from) == inters_str_map[street_seg.streetID].end()) {
            street_intersections[street_seg.streetID].push_back(street_seg.from);
        }

        // Intersection not associated with street yet
        if(inters_str_map[street_seg.streetID].find(street_seg.to) == inters_str_map[street_seg.streetID].end()) {
            street_intersections[street_seg.streetID].push_back(street_seg.to);
        }

        // Put intersection in map
        inters_str_map[street_seg.streetID][street_seg.from] = true; 
        inters_str_map[street_seg.streetID][street_seg.to] = true; 
   }
}

// Iterate through each street segment and calculate its length 
void StreetDatabaseAPIMembers::loadLengthOfStreetSegments() {
    street_seg_lengths.resize(getNumStreetSegments());

    // Load each street segement length into each street 
    for(int streetSegmentIdx = 0; streetSegmentIdx < getNumStreetSegments(); streetSegmentIdx++){

        // Initialize variables
        StreetSegmentInfo currentSeg = getStreetSegmentInfo(streetSegmentIdx);
        LatLon from = getIntersectionPosition(currentSeg.from);
        LatLon to = getIntersectionPosition(currentSeg.to);
        double length = 0;
        
        // If the street segment has no curve points, find distance between "from" and "to"
        if (currentSeg.numCurvePoints == 0) {
            length = findDistanceBetweenTwoPoints(from, to);
        }

        // Else, iterate through curve points and sum distances
        for (int curvePointIdx = 0; curvePointIdx < currentSeg.numCurvePoints; curvePointIdx++) {
            
            // On first iteration, find distance between "from" and ith point
            if (curvePointIdx == 0) {
                length += findDistanceBetweenTwoPoints(from, getStreetSegmentCurvePoint(curvePointIdx, streetSegmentIdx));
            } 

            // On subsequent iterations, find distances between (i-1)th point and ith point
            else if (currentSeg.numCurvePoints  > 1){
                length += findDistanceBetweenTwoPoints(getStreetSegmentCurvePoint((curvePointIdx - 1), streetSegmentIdx), getStreetSegmentCurvePoint(curvePointIdx, streetSegmentIdx));
            }

            // On last iteration, find distance between ith point and "to"
            if (curvePointIdx == (currentSeg.numCurvePoints - 1)) {
                length += findDistanceBetweenTwoPoints(getStreetSegmentCurvePoint(curvePointIdx, streetSegmentIdx), to);
            }    
        }

        street_seg_lengths[streetSegmentIdx] = length;
    }
}

// Iterate through each street segment and add its length to the total street length
void StreetDatabaseAPIMembers::loadLengthOfStreet() {
    street_lengths.resize(getNumStreets());

    // Load each street segement length into each street 
    for(int streetSegmentIdx = 0; streetSegmentIdx < getNumStreetSegments(); streetSegmentIdx++) {
        StreetSegmentInfo street_seg = getStreetSegmentInfo(streetSegmentIdx);

        street_lengths[street_seg.streetID] += street_seg_lengths[streetSegmentIdx];
    }
}
/*****************End StreetDatabaseAPI Helper Function Definitions**********************/



/**********************OSMDatabaseAPI Helper Function Definitions***********************/

// OSMDatabaseAPIMembers constructor
OSMDatabaseAPIMembers::OSMDatabaseAPIMembers(){
    loadWayLengths();
    loadOSMIDsTagKeyPairs();
}

// OSMDatabaseAPIMembers destructor
OSMDatabaseAPIMembers::~OSMDatabaseAPIMembers(){
    way_lengths.clear();
    wayIDs_to_wayLength_map.clear();
    OSMIDs_to_tags_map.clear();
}

// Fills up global variable wayIDs_to_wayLength_map by calculating the length of the way and storing 
// it at the key value of the way's OSMID. This is done by iterating through all the way members and 
// summing up the distance between each of them.
void OSMDatabaseAPIMembers::loadWayLengths() {

    // Iterate through all the nodes and place them into the OSMIDs_to_node_map based on their OSMID
    for (int idx = 0; idx < getNumberOfNodes(); idx++) { 
        const OSMNode* curNode = getNodeByIndex(idx);  
        OSMID curOSMID = curNode->id(); 
        OSMIDs_to_node_map[curOSMID] = curNode; 
    }

    // Iterate through all the ways 
    for (int idx = 0; idx < getNumberOfWays(); idx++) {
        const OSMWay *curWay = getWayByIndex(idx);

        // Create vector of the OSMIDs of all the way members within the current way
        const std::vector<OSMID> curWayMembers = getWayMembers(curWay); 
        
        // Initialize the calculated distance to 0
        double totalWayDistance = 0;
        
        // Set the first node as the previous node coordinates
        OSMID curOSMID = curWayMembers[0];
        const OSMNode *curNode = OSMIDs_to_node_map[curOSMID];
        LatLon prevNodeCoords = getNodeCoords(curNode);

        // Interate through way members starting from second node and add up distances between the nodes within the way
        for (int wayMemberIdx = 1; wayMemberIdx < curWayMembers.size(); wayMemberIdx++) { 
            curOSMID = curWayMembers[wayMemberIdx];

            // Access OSMIDs_to_node_map variable to find the node with that OSMID
            curNode = OSMIDs_to_node_map[curOSMID]; 

            // Set current node coordinates then add the distance between this node and previous node to the total distance
            LatLon curNodeCoords = getNodeCoords(curNode); 
            totalWayDistance += findDistanceBetweenTwoPoints(prevNodeCoords, curNodeCoords);

            // Set the current node to previous node for the next iteration
            prevNodeCoords = curNodeCoords; 
        }

        // After going through all the wayMembers add distance to global variable where key is the way's OSMID
        OSMID curWayID = curWay->id();
        wayIDs_to_wayLength_map[curWayID] = totalWayDistance;
    }
}

// Fills up global unordered map OSMIDs_to_tags_map with each node's OSMID as the key and a corresponding map
// with all the node's tag information (keys to values) as its value.
void OSMDatabaseAPIMembers::loadOSMIDsTagKeyPairs(){
    
    // Iterate through all the nodes
    for (int idx = 0; idx < getNumberOfNodes(); idx++) { 

        // Get the OSMID of the current node
        const OSMNode* curNode = getNodeByIndex(idx); 
        OSMID curOSMID = curNode->id(); 
        
        // Create a new key_value_map for each node that includes all the node's tags' corresponding keys and values
        std::unordered_map<std::string, std::string> key_value_map; 
        
        // Iterate through all the tags in the node and store its keys and values in the key_value_map
        for(int curTag=0;curTag<getTagCount(curNode); ++curTag)
        {
            std::string key,value;
            std::tie(key,value) = getTagPair(curNode,curTag);
            key_value_map[key] = value; 
        }

        // Store this key value map within the OSMIDs_to_tags_map with its corresponding key as the node's OSMID
        OSMIDs_to_tags_map[curOSMID] = key_value_map;
    }

    // Iterate through all the ways
    for (int idx = 0; idx < getNumberOfWays(); idx++) { 

        // Get the OSMID of the current node
        const OSMWay* curWay = getWayByIndex(idx); 
        OSMID curOSMID = curWay->id(); 
        
        // Create a new key_value_map for each node that includes all the node's tags' corresponding keys and values
        std::unordered_map<std::string, std::string> key_value_map; 
        
        // Iterate through all the tags in the node and store its keys and values in the key_value_map
        for(int curTag=0;curTag<getTagCount(curWay); ++curTag)
        {
            std::string key,value;
            std::tie(key,value) = getTagPair(curWay,curTag);
            key_value_map[key] = value; 
        }

        // Store this key value map within the OSMIDs_to_tags_map with its corresponding key as the node's OSMID
        OSMIDs_to_tags_map[curOSMID] = key_value_map;
    }
}
/*******************End OSMDatabaseAPI Helper Function Definitions**********************/


/************************General Helper Function Definitions****************************/
// Convert pair of coordinates to meters
std::pair<std::pair<double,double>, std::pair<double,double>> coordsToMeters(LatLon coords1, LatLon coords2) {
    
    // Initialize vectors
    std::pair<std::pair<double,double>, std::pair<double,double>> results;

    // Access each point's lat and lon values
    double p1_lat = kDegreeToRadian * coords1.latitude();
    double p1_lon = kDegreeToRadian * coords1.longitude();
    
    double p2_lat = kDegreeToRadian * coords2.latitude();
    double p2_lon = kDegreeToRadian * coords2.longitude();

    // lat_avg defined as (lat_1+lat_2)/2
    double lat_avg = (p1_lat + p2_lat)/2;

    // Convert coords to meters
    results.first.first = kEarthRadiusInMeters*p1_lon*cos(lat_avg); // x = R * lon * cos(lat_avg)
    results.first.second = kEarthRadiusInMeters*p1_lat; // y = R * lat
    results.second.first = kEarthRadiusInMeters*p2_lon*cos(lat_avg); // x = R * lon * cos(lat_avg)
    results.second.second = kEarthRadiusInMeters*p2_lat; // y = R * lat

    return results;
}
/**********************End General Helper Function Definitions**************************/
