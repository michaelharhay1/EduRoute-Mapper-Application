
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

//---------------------------------------- Switch Callback Functions -----------------------------------------//
// Determine functionality of switch flip, if on, change the colour scheme to the game mode
bool act_on_game_switch_flip(GtkSwitch * /*game_switch*/, gboolean switch_state, ezgl::application *application)
{

    gtk_switch_set_state(game_switch, switch_state);
    quiz_mode = switch_state;
    
    // Don't switch if there isn't a path selected
    if(quiz_mode == true && !display_path){
        quiz_mode = false;
        gtk_switch_set_state(game_switch, false);

        application->create_popup_message("ERROR", "Please select a path before switching to game mode");
        return true;
    }


    if(!quiz_mode && intersections_clicked == 2){
        display_path = true;
    }
    else {
        display_path = false;
    }

    if(quiz_mode) {
        GObject *mode_dropdown = application->get_object("SearchModeDropdown");
        gtk_widget_set_visible(GTK_WIDGET(mode_dropdown), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(go_button), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(start_entry), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(dest_entry), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(start_entry_pt2), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(dest_entry_pt2), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(directions_button), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(clear_button), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(label_1), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(label_2), FALSE);
    } 
    
    else {
        GObject *mode_dropdown = application->get_object("SearchModeDropdown");
        gtk_widget_set_visible(GTK_WIDGET(go_button), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(mode_dropdown), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(start_entry), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(dest_entry), TRUE);

        if (search_mode == FASTEST_ROUTE)
        {
            gtk_widget_set_visible(GTK_WIDGET(start_entry_pt2), TRUE);
            gtk_widget_set_visible(GTK_WIDGET(dest_entry_pt2), TRUE);
            gtk_widget_set_visible(GTK_WIDGET(directions_button), TRUE);
            gtk_widget_set_visible(GTK_WIDGET(clear_button), TRUE);
            gtk_widget_set_visible(GTK_WIDGET(label_1), TRUE);
            gtk_widget_set_visible(GTK_WIDGET(label_2), TRUE);
        }
    }

    application->refresh_drawing();
    return true;
}

bool act_on_depot_switch_flip(GtkSwitch * /*depot_switch*/, gboolean switch_state, ezgl::application */*application*/)
{

    gtk_switch_set_state(depot_switch, switch_state);
    depot_mode = switch_state;

    return true;
}

//-------------------------------------- Switch Callback Functions ---------------------------------------//
