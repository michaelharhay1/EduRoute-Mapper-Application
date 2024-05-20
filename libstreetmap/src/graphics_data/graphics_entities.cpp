/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 26/02/2024
 *
 * Description: Contains struct and helper function definitions for managing graphics 
 * data related to intersections, street segments, POIs, and features.
*/

#include "graphics_entities.h"
#include "OSMDatabaseAPI.h"
#include "city_information.h"
#include <limits.h>

//-------------------- IntersectionsGraphicsData Helper Functions Definitions -------------------//

IntersectionsGraphicsData::IntersectionsGraphicsData()
{

    // Resize the intersections vector
    intersections.resize(getNumIntersections());

    // Pre-compute intersections and fill with respective data
    for (int id = 0; id < getNumIntersections(); id++)
    {

        intersections[id].position = getIntersectionPosition(id);

        double lon = intersections[id].position.longitude();
        double lat = intersections[id].position.latitude();

        intersections[id].name = getIntersectionName(id);
        intersections[id].xy_loc = ezgl::point2d(x_from_lon(lon), y_from_lat(lat));
    }
}

//------------------- End IntersectionsGraphicsData Helper Functions Definitions -----------------//

//---------------------- StreetSegsGraphicsData Helper Functions Definitions ---------------------//

// StreetSegsGraphicsData Constructor
StreetsGraphicsData::StreetsGraphicsData()
{

    // Pre-compute street data
    all_street_segs.resize(getNumStreetSegments());
    all_streets.resize(getNumStreets());
    street_segs_by_tag.resize(28);
    streets_by_tag.resize(28);

    for (int ss_id = 0; ss_id < getNumStreetSegments(); ss_id++)
    {

        // Get LatLon data of street segment's "from" and "to" intersections
        StreetSegmentInfo segInfo = getStreetSegmentInfo(ss_id);

        all_street_segs[ss_id].from = getIntersectionPosition(segInfo.from);
        all_street_segs[ss_id].to = getIntersectionPosition(segInfo.to);
        all_street_segs[ss_id].num_curve_points = segInfo.numCurvePoints;
        all_street_segs[ss_id].wayOSMID = segInfo.wayOSMID;
        all_street_segs[ss_id].road_type = getOSMNodeTagValue(all_street_segs[ss_id].wayOSMID, "highway");
        all_street_segs[ss_id].streetID = segInfo.streetID;
        all_street_segs[ss_id].name = getStreetName(all_street_segs[ss_id].streetID);
        all_street_segs[ss_id].oneWay = segInfo.oneWay;

        // If num_curve_points is non-zero, pre-compute curve points
        if (all_street_segs[ss_id].num_curve_points != 0)
        {

            all_street_segs[ss_id].curve_points.resize(all_street_segs[ss_id].num_curve_points);
            all_street_segs[ss_id].curve_points_xy_loc.resize(all_street_segs[ss_id].num_curve_points);

            for (int curve_id = 0; curve_id < all_street_segs[ss_id].num_curve_points; ++curve_id)
            {
                all_street_segs[ss_id].curve_points[curve_id] = getStreetSegmentCurvePoint(curve_id, ss_id);

                double curve_lon = all_street_segs[ss_id].curve_points[curve_id].longitude();
                double curve_lat = all_street_segs[ss_id].curve_points[curve_id].latitude();

                all_street_segs[ss_id].curve_points_xy_loc[curve_id] = ezgl::point2d(x_from_lon(curve_lon), y_from_lat(curve_lat));
            }
        }

        double from_lon = all_street_segs[ss_id].from.longitude();
        double from_lat = all_street_segs[ss_id].from.latitude();
        double to_lon = all_street_segs[ss_id].to.longitude();
        double to_lat = all_street_segs[ss_id].to.latitude();

        // Find xy coordinates of "from" and "to"
        all_street_segs[ss_id].from_xy_loc = ezgl::point2d(x_from_lon(from_lon), y_from_lat(from_lat));
        all_street_segs[ss_id].to_xy_loc = ezgl::point2d(x_from_lon(to_lon), y_from_lat(to_lat));

        // Set street segment angle
        all_street_segs[ss_id].angle = atan2(all_street_segs[ss_id].to_xy_loc.y - all_street_segs[ss_id].from_xy_loc.y,
                                             all_street_segs[ss_id].to_xy_loc.x - all_street_segs[ss_id].from_xy_loc.x);

        // Set center point of segment (for displaying text)
        // If no curve points, set center to average of "from" and "to"
        if (all_street_segs[ss_id].num_curve_points == 0)
        {
            all_street_segs[ss_id].center_xy_loc = ezgl::point2d((all_street_segs[ss_id].to_xy_loc.x + all_street_segs[ss_id].from_xy_loc.x) / 2,
                                                                 (all_street_segs[ss_id].to_xy_loc.y + all_street_segs[ss_id].from_xy_loc.y) / 2);
        }

        // Else if even number of curve points, set center to average of middle two curve points
        else if (all_street_segs[ss_id].num_curve_points % 2 == 0)
        {
            all_street_segs[ss_id].center_xy_loc = ezgl::point2d((all_street_segs[ss_id].curve_points_xy_loc[(all_street_segs[ss_id].num_curve_points) / 2].x +
                                                                  all_street_segs[ss_id].curve_points_xy_loc[((all_street_segs[ss_id].num_curve_points) / 2) - 1].x) /
                                                                     2,
                                                                 (all_street_segs[ss_id].curve_points_xy_loc[(all_street_segs[ss_id].num_curve_points) / 2].y +
                                                                  all_street_segs[ss_id].curve_points_xy_loc[((all_street_segs[ss_id].num_curve_points) / 2) - 1].y) /
                                                                     2);
        }

        // Else set center to middle curve point
        else
        {
            all_street_segs[ss_id].center_xy_loc = all_street_segs[ss_id].curve_points_xy_loc[(all_street_segs[ss_id].num_curve_points - 1) / 2];
        }

        // Sort street segments into street_segs_by_tag
        if (tag_num.find(all_street_segs[ss_id].road_type) == tag_num.end())
        {
            all_street_segs[ss_id].index_in_street_segs_by_tag = street_segs_by_tag[27].size();
            street_segs_by_tag[27].push_back(all_street_segs[ss_id]);
        }

        else
        {
            all_street_segs[ss_id].index_in_street_segs_by_tag = street_segs_by_tag[tag_num[all_street_segs[ss_id].road_type]].size();
            street_segs_by_tag[tag_num[all_street_segs[ss_id].road_type]].push_back(all_street_segs[ss_id]);
        }

        // Sort street segments into their respective streets
        int street_id = all_street_segs[ss_id].streetID;
        all_streets[street_id].street_to_street_segs.push_back(all_street_segs[ss_id]);
        all_streets[street_id].road_type = all_street_segs[ss_id].road_type;
        all_streets[street_id].name = all_street_segs[ss_id].name;
    }

    // Loop through each street
    for (int street_id = 0; street_id < all_streets.size(); street_id++)
    {

        // Sort street segments into streets_by_tag
        if (tag_num.find(all_streets[street_id].road_type) == tag_num.end())
        {
            streets_by_tag[27].push_back(all_streets[street_id]);
        }
        else
        {
            streets_by_tag[tag_num[all_streets[street_id].road_type]].push_back(all_streets[street_id]);
        }
    }
}

//--------------------- End StreetSegsGraphicsData Helper Functions Definitions -------------------//

//-------------------------------- POI Helper Functions Definitions -------------------------------//

POIGraphicsData::POIGraphicsData()
{

    POIs.resize(getNumPointsOfInterest());

    // Pre-load each POI into a vector for easy access of information
    for (int poi_idx = 0; poi_idx < getNumPointsOfInterest(); poi_idx++)
    {

        // Fill POI Data with respective data
        POIs[poi_idx].type = getPOIType(poi_idx);
        POIs[poi_idx].name = getPOIName(poi_idx);

        POIs[poi_idx].position = getPOIPosition(poi_idx);
        double lon = POIs[poi_idx].position.longitude();
        double lat = POIs[poi_idx].position.latitude();

        POIs[poi_idx].xy_loc = ezgl::point2d(x_from_lon(lon), y_from_lat(lat));
        POIs[poi_idx].OSMNodeID = getPOIOSMNodeID(poi_idx);

        // Group POIs of the Sustenance Type Together
        if (POIs[poi_idx].type == "bar" || POIs[poi_idx].type == "biergarten" ||
            POIs[poi_idx].type == "cafe" || POIs[poi_idx].type == "fast_food" ||
            POIs[poi_idx].type == "food_court" || POIs[poi_idx].type == "ice_cream" ||
            POIs[poi_idx].type == "pub" || POIs[poi_idx].type == "restaurant")
        {
            POIs[poi_idx].amenity = "sustenance";
            POIs[poi_idx].colour = ezgl::YELLOW;
            POIs[poi_idx].image = sustenances_image;
            POI_amenities[POIs[poi_idx].amenity].push_back(POIs[poi_idx]);
        }

        // Group POIs of the Sustenance Type Together
        if (POIs[poi_idx].type == "college" || POIs[poi_idx].type == "dancing_school" ||
            POIs[poi_idx].type == "driving_school" || POIs[poi_idx].type == "first_aid_school" ||
            POIs[poi_idx].type == "kindergarten" || POIs[poi_idx].type == "language_school" ||
            POIs[poi_idx].type == "library" || POIs[poi_idx].type == "surf_school" ||
            POIs[poi_idx].type == "toy_library" || POIs[poi_idx].type == "research_institute" ||
            POIs[poi_idx].type == "training" || POIs[poi_idx].type == "music_school" ||
            POIs[poi_idx].type == "school" || POIs[poi_idx].type == "traffic_park" ||
            POIs[poi_idx].type == "university")
        {
            POIs[poi_idx].amenity = "others";
            POIs[poi_idx].colour = ezgl::BLACK;
            POIs[poi_idx].image = others_image;
            POI_amenities[POIs[poi_idx].amenity].push_back(POIs[poi_idx]);
        }

        // Transportation
        if (POIs[poi_idx].type == "bicycle_parking" || POIs[poi_idx].type == "bicycle_repair_station" ||
            POIs[poi_idx].type == "bicycle_rental" || POIs[poi_idx].type == "bicycle_wash" ||
            POIs[poi_idx].type == "boat_rental" || POIs[poi_idx].type == "boat_sharing" ||
            POIs[poi_idx].type == "bus_station" || POIs[poi_idx].type == "car_rental" ||
            POIs[poi_idx].type == "car_sharing" || POIs[poi_idx].type == "car_wash" ||
            POIs[poi_idx].type == "compressed_air" || POIs[poi_idx].type == "vehicle_inspection" ||
            POIs[poi_idx].type == "charging_station" || POIs[poi_idx].type == "driver_training" ||
            POIs[poi_idx].type == "ferry_terminal" || POIs[poi_idx].type == "fuel" ||
            POIs[poi_idx].type == "grit_bin" || POIs[poi_idx].type == "motorcycle_parking" ||
            POIs[poi_idx].type == "parking" || POIs[poi_idx].type == "parking_entrance" ||
            POIs[poi_idx].type == "parking_space" || POIs[poi_idx].type == "taxi" ||
            POIs[poi_idx].type == "weighbridge")
        {
            POIs[poi_idx].amenity = "transportation";
            POIs[poi_idx].colour = ezgl::BLUE;
            POIs[poi_idx].image = transportation_image;
            POI_amenities[POIs[poi_idx].amenity].push_back(POIs[poi_idx]);
        }

        // Financial
        if (POIs[poi_idx].type == "atm" || POIs[poi_idx].type == "payment_terminal" ||
            POIs[poi_idx].type == "bank" || POIs[poi_idx].type == "bureau_de_change" ||
            POIs[poi_idx].type == "money_transfer" || POIs[poi_idx].type == "payment_centre")
        {
            POIs[poi_idx].amenity = "others";
            POIs[poi_idx].colour = ezgl::BLACK;
            POIs[poi_idx].image = others_image;
            POI_amenities[POIs[poi_idx].amenity].push_back(POIs[poi_idx]);
        }

        // Healthcare
        if (POIs[poi_idx].type == "baby_hatch" || POIs[poi_idx].type == "clinic" ||
            POIs[poi_idx].type == "dentist" || POIs[poi_idx].type == "doctors" ||
            POIs[poi_idx].type == "hospital" || POIs[poi_idx].type == "nursing_home" ||
            POIs[poi_idx].type == "pharmacy" || POIs[poi_idx].type == "social_facility" ||
            POIs[poi_idx].type == "veterinary")
        {
            POIs[poi_idx].amenity = "healthcare";
            POIs[poi_idx].colour = ezgl::RED;
            POIs[poi_idx].image = healthcare_image;
            POI_amenities[POIs[poi_idx].amenity].push_back(POIs[poi_idx]);
        }

        // Entertainment, Arts & Culture
        if (POIs[poi_idx].type == "arts_centre" || POIs[poi_idx].type == "brothel" ||
            POIs[poi_idx].type == "casino" || POIs[poi_idx].type == "cinema" ||
            POIs[poi_idx].type == "community_centre" || POIs[poi_idx].type == "conference_centre" ||
            POIs[poi_idx].type == "events_venue" || POIs[poi_idx].type == "exhibition_centre" ||
            POIs[poi_idx].type == "fountain" || POIs[poi_idx].type == "gambling" ||
            POIs[poi_idx].type == "love_hotel" || POIs[poi_idx].type == "music_venue" ||
            POIs[poi_idx].type == "nightclub" || POIs[poi_idx].type == "planetarium" ||
            POIs[poi_idx].type == "public_bookcase" || POIs[poi_idx].type == "social_centre" ||
            POIs[poi_idx].type == "stripclub" || POIs[poi_idx].type == "studio" ||
            POIs[poi_idx].type == "swingerclub" || POIs[poi_idx].type == "theatre")
        {
            POIs[poi_idx].amenity = "others";
            POIs[poi_idx].colour = ezgl::BLACK;
            POIs[poi_idx].image = others_image;
            POI_amenities[POIs[poi_idx].amenity].push_back(POIs[poi_idx]);
        }

        // Public service
        if (POIs[poi_idx].type == "courthouse" || POIs[poi_idx].type == "fire_station" ||
            POIs[poi_idx].type == "police" || POIs[poi_idx].type == "post_box" ||
            POIs[poi_idx].type == "post_depot" || POIs[poi_idx].type == "post_office" ||
            POIs[poi_idx].type == "prison" || POIs[poi_idx].type == "ranger_station" ||
            POIs[poi_idx].type == "townhall")
        {
            POIs[poi_idx].amenity = "others";
            POIs[poi_idx].colour = ezgl::BLACK;
            POIs[poi_idx].image = others_image;
            POI_amenities[POIs[poi_idx].amenity].push_back(POIs[poi_idx]);
        }

        // Facilities
        if (POIs[poi_idx].type == "bbq" || POIs[poi_idx].type == "bench" ||
            POIs[poi_idx].type == "dog_toilet" || POIs[poi_idx].type == "dressing_room" ||
            POIs[poi_idx].type == "drinking_water" || POIs[poi_idx].type == "give_box" ||
            POIs[poi_idx].type == "mailroom" || POIs[poi_idx].type == "parcel_locker" ||
            POIs[poi_idx].type == "shelter" || POIs[poi_idx].type == "shower" ||
            POIs[poi_idx].type == "telephone" || POIs[poi_idx].type == "toilets" ||
            POIs[poi_idx].type == "water_point" || POIs[poi_idx].type == "watering_place")
        {
            POIs[poi_idx].amenity = "others";
            POIs[poi_idx].colour = ezgl::BLACK;
            POIs[poi_idx].image = others_image;
            POI_amenities[POIs[poi_idx].amenity].push_back(POIs[poi_idx]);
        }

        // Waste Management
        if (POIs[poi_idx].type == "sanitary_dump_station" || POIs[poi_idx].type == "recycling" ||
            POIs[poi_idx].type == "waste_basket" || POIs[poi_idx].type == "waste_disposal" ||
            POIs[poi_idx].type == "waste_transfer_station")
        {
            POIs[poi_idx].amenity = "others";
            POIs[poi_idx].colour = ezgl::BLACK;
            POIs[poi_idx].image = others_image;
            POI_amenities[POIs[poi_idx].amenity].push_back(POIs[poi_idx]);
        }

        // Others
        if (POIs[poi_idx].type == "animal_boarding" || POIs[poi_idx].type == "animal_breeding" ||
            POIs[poi_idx].type == "animal_shelter" || POIs[poi_idx].type == "animal_training" ||
            POIs[poi_idx].type == "baking_oven" || POIs[poi_idx].type == "clock" ||
            POIs[poi_idx].type == "crematorium" || POIs[poi_idx].type == "dive_centre" ||
            POIs[poi_idx].type == "funeral_hall" || POIs[poi_idx].type == "grave_yard" ||
            POIs[poi_idx].type == "hunting_stand" || POIs[poi_idx].type == "internet_cafe" ||
            POIs[poi_idx].type == "kitchen" || POIs[poi_idx].type == "kneipp_water_cure" ||
            POIs[poi_idx].type == "lounger" || POIs[poi_idx].type == "marketplace" ||
            POIs[poi_idx].type == "monastery" || POIs[poi_idx].type == "mortuary" ||
            POIs[poi_idx].type == "photo_booth" || POIs[poi_idx].type == "place_of_mourning" ||
            POIs[poi_idx].type == "place_of_worship" || POIs[poi_idx].type == "public_bath" ||
            POIs[poi_idx].type == "public_building" || POIs[poi_idx].type == "refugee_site" ||
            POIs[poi_idx].type == "vending_machine" || POIs[poi_idx].type == "user defined")
        {
            POIs[poi_idx].amenity = "others";
            POIs[poi_idx].image = others_image;
            POIs[poi_idx].colour = ezgl::BLACK;
            POI_amenities[POIs[poi_idx].amenity].push_back(POIs[poi_idx]);
        }
    }
}

//------------------------- End POI Helper Functions Definitions ---------------------------//

//---------------------------- Feature Helper Functions Definitions ------------------------//
FeaturesGraphicsData::FeaturesGraphicsData()
{

    features.resize(getNumFeatures());

    ezgl::color features_colors[] = {
        ezgl::color(204, 204, 204), // UNKNOWN (Light Gray)
        ezgl::color(6, 187, 138),   // PARK (Green)
        ezgl::color(255, 255, 153), // BEACH (Pale beachy)
        ezgl::color(84, 186, 185),  // LAKE (Blue)
        ezgl::color(82, 169, 212),  // RIVER (Blue)
        ezgl::color(6, 187, 138),   // ISLAND (Green)
        ezgl::color(255, 188, 206), // BUILDING (Silver)
        ezgl::color(199, 233, 176), // GREENSPACE (Green)
        ezgl::color(225, 235, 200), // GOLFCOURSE (Green)
        ezgl::color(158, 210, 198)  // STREAM (Blue)
    };

    ezgl::color features_game_colors[] = {
        ezgl::color(204, 204, 204), // UNKNOWN (Light Gray)
        ezgl::color(6, 187, 138),   // PARK (Green)
        ezgl::color(255, 255, 153), // BEACH (Light Orange)
        ezgl::color(82, 169, 212),  // LAKE (Blue)
        ezgl::color(82, 169, 212),  // RIVER (Blue)
        ezgl::color(6, 187, 138),   // ISLAND (Green)
        ezgl::color(87, 125, 144),  // BUILDING (Silver)
        ezgl::color(6, 187, 138),   // GREENSPACE (Green)
        ezgl::color(6, 166, 124),   // GOLFCOURSE (Green)
        ezgl::color(82, 169, 212)   // STREAM (Blue)
    };

    for (FeatureIdx feature_id = 0; feature_id < getNumFeatures(); feature_id++)
    {

        FeaturesData feature_data;
        int num_points = getNumFeaturePoints(feature_id);

        // Populate feature data
        feature_data.name = getFeatureName(feature_id);
        feature_data.type = getFeatureType(feature_id);
        feature_data.OSMFeatureID = getFeatureOSMID(feature_id);
        feature_data.color = features_colors[static_cast<int>(feature_data.type)];
        feature_data.gameColor = features_game_colors[static_cast<int>(feature_data.type)];
        feature_data.isFeatureClosed = num_points > 0 && (getFeaturePoint(0, feature_id) == getFeaturePoint(num_points - 1, feature_id));
        feature_data.feature_ID = feature_id;

        // Populate feature points
        feature_data.feature_points.resize(num_points);

        for (int i = 0; i < num_points; i++)
        {
            LatLon point = getFeaturePoint(i, feature_id);
            double x = x_from_lon(point.longitude());
            double y = y_from_lat(point.latitude());
            feature_data.feature_points[i] = ezgl::point2d(x, y);
        }

        if (feature_data.type == FeatureType::BUILDING /*|| feature_data.type == FeatureType::ISLAND */ || !feature_data.isFeatureClosed)
        {
            feature_data.area = 0;
        }
        else
        {
            feature_data.area = findFeatureArea(feature_data.feature_ID);
        }

        // Populate features and features_by_tag vectors
        features[feature_id] = feature_data;
        features_by_tag[features[feature_id].type].push_back(features[feature_id]);
        features_by_area.insert(std::pair<double, FeaturesData>(feature_data.area, feature_data));
    }
}

// Finds closest point of a feature to position parameter
std::pair<int, std::pair<FeatureType, LatLon>> FeaturesGraphicsData::findClosestFeaturePoint(const LatLon &position)
{

    double min_dist = std::numeric_limits<double>::max();
    int closest_feature_id = -1;
    LatLon closest_point;
    FeatureType closest_type = GLACIER;

    for (size_t type_num = 0; type_num < 11; type_num++)
    {

        FeatureType feature_type = featureTypeMap[type_num];

        for (FeatureIdx feature_id = 0; feature_id < features_by_tag[feature_type].size(); feature_id++)
        {

            for (const auto &point : features_by_tag[feature_type][feature_id].feature_points)
            {

                LatLon feature_point = LatLon(lat_from_y(point.y), lon_from_x(point.x));
                double dist = findDistanceBetweenTwoPoints(position, feature_point);

                if (dist < min_dist)
                {
                    min_dist = dist;
                    closest_feature_id = feature_id;
                    closest_point = feature_point;
                    closest_type = feature_type;
                }
            }
        }
    }

    return std::make_pair(closest_feature_id, std::make_pair(closest_type, closest_point));
}

//------------------------- End Feature Helper Functions Definitions ---------------------------//
