/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 08/02/2024
 *
 * Description: Contains structs containing helper functions and class variables
*/

#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

/*****************************Helper Function Declarations******************************/
std::pair<std::pair<double,double>, std::pair<double,double>> coordsToMeters(LatLon coords1, LatLon coords2);
/***************************End Helper Function Declarations****************************/

/**********************************Class Definitions************************************/
struct SegData{
    IntersectionIdx from, to;  // intersection ID this segment runs from/to
    bool oneWay;        // if true, then can only travel in from->to direction

    int numCurvePoints;      // number of curve points between the ends
    float speedLimit;        // in m/s

    StreetIdx streetID;     // index of street this segment belongs to
};




struct StreetDatabaseAPIMembers {
    // Stores speed of each street segment
    std::vector<double> street_segment_speed;

    // Stores the street segments of each intersection
    std::vector<std::vector<StreetSegmentIdx>> intersection_street_segments;

    // Stores the intersections of each street
    std::vector<std::vector<IntersectionIdx>> street_intersections;

    // Maps prefix to vector of street
    std::unordered_map<std::string, std::vector<StreetIdx>> street_name_map;

    // Stores lengths of each street
    std::vector<double> street_seg_lengths;

    // Stores lengths of each street
    std::vector<double> street_lengths;

    // Stores all data for street segmenets
    std::vector<SegData> street_seg_data;

    // Stores all data for intersections
    std::vector<LatLon> intersection_lat_lon;
 
    // Clear StreetDatabaseAPIMembers
    StreetDatabaseAPIMembers();

    // Clear StreetDatabaseAPIMembers
    ~StreetDatabaseAPIMembers();

    // Loads street names into street_name_map
    void loadMapOfStreetNames();

    // Load speed of each street into street_segment_speed
    void loadSpeedOfStreetSegments();

    // Load street segment of each intersection into intersection_street_segments
    void loadStreetSegmentsOfIntersections();

    // Load intersections of each street into street_intersections
    void loadIntersectionsOfStreets();

    // Load the length of each street into street_seg_lengths
    void loadLengthOfStreetSegments();

    // Load the length of each street into street_lengths
    void loadLengthOfStreet();

    void loadDataofStreetSegs();
    
    void loadIntersectionLatlon();

};

struct OSMDatabaseAPIMembers {
    // Stores length of each way
    std::vector<int> way_lengths;

    // Create an unordered map of OSMIDs and their corresponding OSMNodes
    std::unordered_map<OSMID, const OSMNode*> OSMIDs_to_node_map;

    // Stores the wayLength of each wayID 
    std::unordered_map<OSMID, double> wayIDs_to_wayLength_map;

    // Stores the tags of each OSMID
    std::unordered_map<OSMID, std::unordered_map<std::string, std::string>> OSMIDs_to_tags_map;

    // Clear OSMDatabaseAPIMembers
    OSMDatabaseAPIMembers();

    // Clear OSMDatabaseAPIMembers
    ~OSMDatabaseAPIMembers();

    // Load wayLengths into way_lengths, load wayIDs_to_wayLength_map
    void loadWayLengths();

    // Load OSMID / tag pairs into OSMIDs_to_tags_map
    void loadOSMIDsTagKeyPairs();
};
/********************************End Class Definitions**********************************/

   

