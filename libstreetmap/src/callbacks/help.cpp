
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

//---------------------------------------- Help Callback Functions -----------------------------------------//

// Determines functionality of help button press
void act_on_help_button_click(GtkButton * /*help_button*/, ezgl::application *application)
{

    // Provide help message
    application->create_popup_message_with_callback(act_on_popup_features_done_button, "Help", "To search for all shared intersections of two streets,"
                                                                                               "select 'Shared Intersections'\nfrom the dropdown, enter the street names, and press 'GO'.\n\nTo search for the fastest route between two intersections, "
                                                                                               "select 'Fastest Route'\n from the dropdown, enter the intersection names, and press 'GO'.");
}

//-------------------------------------- Help Draw Callback Functions ---------------------------------------//