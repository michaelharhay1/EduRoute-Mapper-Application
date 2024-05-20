/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 26/02/2024
 *
 * Description: Contains struct and helper function declarations for managing graphics 
 * data related to intersections, street segments, POIs, and features.
*/

#ifndef GRAPHICS_ENTITIES_H
#define GRAPHICS_ENTITIES_H

#include "m1.h"
#include "m2.h"

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"

#include <unordered_map>
#include <map>
#include <string>
#include <sstream>
#include <thread>
#include <functional>

//------------------------------------ Class Declarations -----------------------------------//
// Features structures
struct FeaturesData
{

    // Feature data from M1
    FeatureIdx feature_ID;
    std::string name;
    FeatureType type;
    TypedOSMID OSMFeatureID;

    // Graphics information for displaying the correct colour
    ezgl::color color;
    ezgl::color gameColor;

    bool isFeatureClosed;
    double area = 0;
    bool highlight = false;

    std::vector<ezgl::point2d> feature_points;
};

struct FeaturesGraphicsData
{

    // Storing features by feature ID, area, and by tag
    std::vector<FeaturesData> features;
    std::multimap<double, FeaturesData> features_by_area;
    std::unordered_map<FeatureType, std::vector<FeaturesData>> features_by_tag;

    // Mapping index for features_by_tages to feature type
    std::unordered_map<size_t, FeatureType> featureTypeMap = {
        {0, FeatureType::UNKNOWN},
        {1, FeatureType::PARK},
        {2, FeatureType::BEACH},
        {3, FeatureType::LAKE},
        {4, FeatureType::RIVER},
        {5, FeatureType::ISLAND},
        {6, FeatureType::BUILDING},
        {7, FeatureType::GREENSPACE},
        {8, FeatureType::GOLFCOURSE},
        {9, FeatureType::STREAM},
        {10, FeatureType::GLACIER}};

    std::pair<int, std::pair<FeatureType, LatLon>> findClosestFeaturePoint(const LatLon &);
    FeaturesGraphicsData();
};

// POI structures
struct POIData
{

    // POI data from m1
    std::string type;
    std::string name;
    std::string amenity;
    ezgl::color colour;
    ezgl::surface *image;

    // Positiong data for lat lon and x y coordinates
    LatLon position;
    ezgl::point2d xy_loc;

    OSMID OSMNodeID;
};

struct POIGraphicsData
{

    // Storing intersection in vector
    std::vector<POIData> POIs;
    std::unordered_map<std::string, std::vector<POIData>> POI_amenities;

    // Storing images for different types of POIs
    ezgl::surface *sustenances_image = ezgl::renderer::load_png("libstreetmap/resources/POI_icons/sustenance.png");
    ezgl::surface *transportation_image = ezgl::renderer::load_png("libstreetmap/resources/POI_icons/transportation.png");
    ezgl::surface *healthcare_image = ezgl::renderer::load_png("libstreetmap/resources/POI_icons/healthcare.png");
    ezgl::surface *others_image = ezgl::renderer::load_png("libstreetmap/resources/POI_icons/others.png");
    ezgl::surface *start_image = ezgl::renderer::load_png("libstreetmap/resources/POI_icons/start.png");
    ezgl::surface *dest_image = ezgl::renderer::load_png("libstreetmap/resources/POI_icons/dest.png");
    // Constructor and Deconstructor
    POIGraphicsData();

    ~POIGraphicsData()
    {
        ezgl::renderer::free_surface(sustenances_image);
        ezgl::renderer::free_surface(transportation_image);
        ezgl::renderer::free_surface(healthcare_image);
        ezgl::renderer::free_surface(others_image);
    }
};

// Intersection structures
struct IntersectionData
{

    // Intersection data from m1
    LatLon position;
    ezgl::point2d xy_loc;
    std::string name;
    bool highlight = false;
    bool clicked_quiz = false;
};

struct IntersectionsGraphicsData
{
    // Storing intersection in vector
    std::vector<IntersectionData> intersections;

    IntersectionsGraphicsData();
};

// Street structures
struct StreetSegsData
{

    // Identification information street data from m1
    StreetIdx streetID;
    std::string name;
    bool oneWay;
    std::string road_type;
    int index_in_street_segs_by_tag;
    bool highlight;
    bool path_from_to_to;
    double angle;

    // from and to in lat lon and xy coordinates
    LatLon from;
    LatLon to;
    ezgl::point2d from_xy_loc;
    ezgl::point2d to_xy_loc;
    ezgl::point2d center_xy_loc;

    // Curve points in lat lon ad xy coordinates
    int num_curve_points;
    std::vector<LatLon> curve_points;
    std::vector<ezgl::point2d> curve_points_xy_loc;

    // OSM ID of the source way
    OSMID wayOSMID;
};

struct StreetData
{

    // Identification information street data from m1
    std::string name;
    std::string road_type;
    double length = 0;

    // Organize street segmenets by streets
    std::vector<StreetSegsData> street_to_street_segs;
};

struct StreetsGraphicsData
{

    // Assigns an integer to each road tag
    std::unordered_map<std::string, int> tag_num = {
        // Highway values - the higher the value the farther away you can see it from
        // Roads
        {"motorway", 0},
        {"trunk", 1},
        {"primary", 2},
        {"secondary", 3},
        {"tertiary", 4},
        {"unclassified", 5},
        {"residential", 6},

        // Link roads
        {"motorway_link", 7},
        {"trunk_link", 8},
        {"primary_link", 9},
        {"secondary_link", 10},
        {"tertiary_link", 11},

        // Special road types
        {"living_street", 12},
        {"service", 13},
        {"pedestrian", 14},
        {"track", 15},
        {"bus_guideway", 16},
        {"escape", 17},
        {"raceway", 18},
        {"road", 19},
        {"busway", 20},

        // Paths
        {"footway", 21},
        {"bridleway", 22},
        {"steps", 23},
        {"corridor", 24},
        {"path", 25},
        {"via_ferrata", 26},
        {"cycleway", 27},

        // Other
        {"other", 28}};

    // Contains all street segments, unsorted
    std::vector<StreetSegsData> all_street_segs;

    // Organize streets segments by tags
    std::vector<std::vector<StreetSegsData>> street_segs_by_tag;

    // Contains all streets, unsorted
    std::vector<StreetData> all_streets;

    // Organize streets by tags
    std::vector<std::vector<StreetData>> streets_by_tag;

    StreetsGraphicsData();
};

//---------------------------------- End Class Declarations ---------------------------------//

#endif
