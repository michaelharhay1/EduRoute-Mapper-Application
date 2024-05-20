
/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 26/02/2024
 *
 * Description: contains global variables and callback functions definitions. Includes variables with info
 * about visible world, search bar functionality, POI buttons, game mode, and search bar auto-complete functionality.
 * Callback function definitions are for drawing new map, initializing GUI components, handling user interactions, etc.
*/

#include "callbacks.h"
#include "m3.h"
#include "graphics_data/city_information.h"
#include "graphics_data/graphics_globals.h"

#include <limits.h>
#include <thread>

// Directions Helper Functions
double calculateTurnDirection(int ss_id);
std::string getCompas();

//---------------------------------------- Direction Callback Functions -----------------------------------------//

// Determines functionality of directions button press
void act_on_directions_button_click(GtkButton * /*directions_button*/, ezgl::application *application)
{

    // If not enough intersections selected, return error message
    if (intersections_clicked < 2)
    {
        application->create_popup_message_with_callback(act_on_popup_features_done_button, "Error: No route selected", "Please select a valid route!");
        return;
    }

    if (path.size() > 0)
    {

        // Get directions based on path
        std::string directions, compass;

        compass = getCompas();

        // First street
        directions.append("\n");
        directions.append(std::to_string(1));
        directions.append(") Head " + compass + " onto ");

        // Append the street name
        if (Streets_graphics_data->all_street_segs[path[0]].name == "<unknown>")
        {
            directions.append("an unnamed street");
        }
        else
        {
            directions.append(Streets_graphics_data->all_street_segs[path[0]].name);
        }

        int count = 2;
        double distance = findStreetSegmentLength(path[0]);
        int lastStreetID = Streets_graphics_data->all_street_segs[path[0]].streetID;

        for (int ss_id = 1; ss_id < path.size(); ss_id++)
        {
            // Check if we're still on the same street
            if (Streets_graphics_data->all_street_segs[path[ss_id]].streetID == lastStreetID)
            {
                distance += findStreetSegmentLength(path[ss_id]);
                continue;
            }
            else
            {
                int magnitude = std::log10(std::abs(distance));
                double normalized = distance / std::pow(10, magnitude);
                double firstDigit = std::floor(normalized);
                int result = firstDigit * std::pow(10, magnitude);

                // If we've reached a new street, finalize the directions for the previous street
                directions.append(", continue for " + std::to_string(result) + " meters.\n");

                // Reset distance for the new street and update lastStreetID
                distance = findStreetSegmentLength(path[ss_id]);
                lastStreetID = Streets_graphics_data->all_street_segs[path[ss_id]].streetID;
            }

            // Append the direction number
            directions.append("\n");
            directions.append(std::to_string(count));
            count++;

            directions.append(") ");

            double turn_direction = calculateTurnDirection(ss_id);

            if(findAngleBetweenStreetSegments(path[ss_id - 1], path[ss_id]) < M_PI/12){
                directions.append(" Move straight onto ");
            } else {
                // Decide on the turn direction based on the angle
                if (turn_direction > 0)
                {
                    directions.append(" Turn left onto ");
                }
                else
                {
                    directions.append(" Turn right onto ");
                }
            }

            // Append the street name
            if (Streets_graphics_data->all_street_segs[path[ss_id]].name == "<unknown>")
            {
                directions.append("an unnamed street");
            }
            else
            {
                directions.append(Streets_graphics_data->all_street_segs[path[ss_id]].name);
            }
        }

        int magnitude = std::log10(std::abs(distance));
        double normalized = distance / std::pow(10, magnitude);
        double firstDigit = std::floor(normalized);
        int result = firstDigit * std::pow(10, magnitude);

        // After the loop, add the distance for the last street segment
        directions.append(", continue for " + std::to_string(result) + " meters until you reach your destination.");

        // Create popup with directions
        application->create_popup_message_with_callback(act_on_popup_features_done_button, "Route Directions", directions.c_str());
    }
}
//-------------------------------------- End Direction Callback Functions ---------------------------------------//

// ---------------------------------------- Directions Helper Functions -----------------------------------------//

// Get the direction the first street segement is heading in (NORTH/SOUTH/EAST/WEST)
std::string getCompas()
{
    std::string compass;
    double rotation = 0, rotation_rad = 0;

    // Calculate initial path rotation_rad if size > 1
    if (path.size() > 1)
    {
        if (Streets_graphics_data->all_street_segs[path[1]].from == Streets_graphics_data->all_street_segs[path[0]].to ||
            Streets_graphics_data->all_street_segs[path[1]].to == Streets_graphics_data->all_street_segs[path[0]].to)
        {
            rotation_rad = Streets_graphics_data->all_street_segs[path[0]].angle;
        }

        else
        {
            rotation_rad = Streets_graphics_data->all_street_segs[path[0]].angle;

            if (rotation_rad < 0)
            {
                rotation_rad += M_PI;
            }
            else
            {
                rotation_rad -= M_PI;
            }
        }
    }

    // Calculate initial path angle if size == 1
    else
    {
        if (getIntersectionPosition(intersection_1) == Streets_graphics_data->all_street_segs[path[0]].from)
        {
            rotation_rad = Streets_graphics_data->all_street_segs[path[0]].angle;
        }

        else
        {
            rotation_rad = Streets_graphics_data->all_street_segs[path[0]].angle;

            if (rotation_rad < 0)
            {
                rotation_rad += M_PI;
            }
            else
            {
                rotation_rad -= M_PI;
            }
        }
    }

    // Convert from [-pi, pi] to [0, 360]
    rotation_rad += M_PI;
    rotation = (180 / M_PI) * rotation_rad;
    if (rotation < 0)
    {
        rotation += 360;
    }

    if (rotation <= 45 || rotation > 315)
    {
        compass = "west";
    }
    else if (rotation > 45 && rotation <= 135)
    {
        compass = "south";
    }
    else if (rotation > 135 && rotation <= 225)
    {
        compass = "east";
    }
    else if (rotation > 225 && rotation <= 315)
    {
        compass = "north";
    }

    return compass;
}

// Calculate the direction of the current street segement with respect to the previous one whevener the user changes streets (LEFT/RIGHT)
double calculateTurnDirection(int ss_id)
{
    ezgl::point2d a, b, c;

    StreetSegsData prev_seg = Streets_graphics_data->all_street_segs[path[ss_id - 1]];
    StreetSegsData curr_seg = Streets_graphics_data->all_street_segs[path[ss_id]];

    // Check which intersection the streets are sharing and get the points of the triangle created
    if (getStreetSegmentInfo(path[ss_id - 1]).from == getStreetSegmentInfo(path[ss_id]).from)
    {
        a = (prev_seg.num_curve_points == 0) ? prev_seg.to_xy_loc : prev_seg.curve_points_xy_loc[0];
        b = prev_seg.from_xy_loc;
        c = (curr_seg.num_curve_points == 0) ? curr_seg.to_xy_loc : curr_seg.curve_points_xy_loc[0];
    }

    else if (getStreetSegmentInfo(path[ss_id - 1]).from == getStreetSegmentInfo(path[ss_id]).to)
    {
        a = (prev_seg.num_curve_points == 0) ? prev_seg.to_xy_loc : prev_seg.curve_points_xy_loc[0];
        b = prev_seg.from_xy_loc;
        c = (curr_seg.num_curve_points == 0) ? curr_seg.from_xy_loc : curr_seg.curve_points_xy_loc[curr_seg.num_curve_points - 1];
    }

    else if (getStreetSegmentInfo(path[ss_id - 1]).to == getStreetSegmentInfo(path[ss_id]).from)
    {
        a = (prev_seg.num_curve_points == 0) ? prev_seg.from_xy_loc : prev_seg.curve_points_xy_loc[prev_seg.num_curve_points - 1];
        b = prev_seg.to_xy_loc;
        c = (curr_seg.num_curve_points == 0) ? curr_seg.to_xy_loc : curr_seg.curve_points_xy_loc[0];
    }

    else if (getStreetSegmentInfo(path[ss_id - 1]).to == getStreetSegmentInfo(path[ss_id]).to)
    {
        a = (prev_seg.num_curve_points == 0) ? prev_seg.from_xy_loc : prev_seg.curve_points_xy_loc[prev_seg.num_curve_points - 1];
        b = prev_seg.to_xy_loc;
        c = (curr_seg.num_curve_points == 0) ? curr_seg.from_xy_loc : curr_seg.curve_points_xy_loc[curr_seg.num_curve_points - 1];
    }

    // calculate and return the cross product
    double turn_direction = (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x);
    return turn_direction;
}
//-------------------------------------- End Direction Helper Functions ---------------------------------------//
