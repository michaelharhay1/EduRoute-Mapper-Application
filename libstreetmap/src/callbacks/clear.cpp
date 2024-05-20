
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

//---------------------------------------- Clear Callback Functions -----------------------------------------/

// Determines functionality of directions button press
void act_on_clear_button_click(GtkButton * /*directions_button*/, ezgl::application *application)
{
    if (search_mode == FASTEST_ROUTE)
    {
        // Clear all intersections pressed
        intersections_clicked = 0;
        Intersections_graphics_data->intersections[intersection_1].highlight = false;
        Intersections_graphics_data->intersections[intersection_2].highlight = false;

        // Clear path highlighted
        for (int ss_id = 0; ss_id < path.size(); ss_id++)
        {
            Streets_graphics_data->street_segs_by_tag
                [Streets_graphics_data->tag_num[Streets_graphics_data->all_street_segs[path[ss_id]].road_type]]
                [Streets_graphics_data->all_street_segs[path[ss_id]].index_in_street_segs_by_tag]
                    .highlight = false;
        }
    }

    if (search_mode == DELIVERY)
    {
        // Clear all intersections pressed
        intersections_clicked = 0;
        Intersections_graphics_data->intersections[intersection_1].highlight = false;

        for (const auto depot_id : depots_list) {
            Intersections_graphics_data->intersections[depot_id].highlight = false;
        }

        for (const auto &delivery_id : deliveries_list) {
            Intersections_graphics_data->intersections[delivery_id.pickUp].highlight = false;
            Intersections_graphics_data->intersections[delivery_id.dropOff].highlight = false;
        }

        depots_list.clear();
        deliveries_list.clear();
    }

    quiz_mode_path.clear();

    for(int inter_id = 0; inter_id < quiz_mode_intersections.size(); inter_id++){
        Intersections_graphics_data->intersections[quiz_mode_intersections[inter_id].first].clicked_quiz = false;
        Intersections_graphics_data->intersections[quiz_mode_intersections[inter_id].second].clicked_quiz = false;
    }

    application->refresh_drawing();
}

//-------------------------------------- End Clear Callback Functions ---------------------------------------//
