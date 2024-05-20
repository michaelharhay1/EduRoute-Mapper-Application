
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

//---------------------------------------- Draw Callback Functions -----------------------------------------//

// The next four functions determine functionality of POI buttons, whenever one of the buttons on
// the top bar is pressed, one of the 4 functions below will be activated, this will allow for small
// POI icons to show up on the screen
void act_on_sustenance_button_click(GtkButton * /*POI_button*/, ezgl::application *application)
{
    show_POI_sustenance ^= true;
    application->refresh_drawing();
}

void act_on_healthcare_button_click(GtkButton * /*POI_button*/, ezgl::application *application)
{
    show_POI_healthcare ^= true;
    application->refresh_drawing();
}

void act_on_transportation_button_click(GtkButton * /*POI_button*/, ezgl::application *application)
{
    show_POI_transportation ^= true;
    application->refresh_drawing();
}

void act_on_others_button_click(GtkButton * /*POI_button*/, ezgl::application *application)
{
    show_POI_others ^= true;
    application->refresh_drawing();
}
//-------------------------------------- End Draw Callback Functions ---------------------------------------//
