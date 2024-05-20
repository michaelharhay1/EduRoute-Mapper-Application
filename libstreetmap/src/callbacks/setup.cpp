
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
#include "graphics_data/city_information.h"
#include "graphics_data/graphics_globals.h"
#include "m3.h"
#include "m4.h"

#include <limits.h>
#include <thread>

//---------------------------------------- Callback Global Variables  -----------------------------------------//

// Graphics Camera View Information
ezgl::rectangle visible_world;
double world_width, world_height;
double zoom_ratio, prev_zoom_ratio;
std::string map_name = "Toronto";

// Search Bar Functionality
int search_mode = 0;
int start_idx = 0, dest_idx = 0;
int start_idx_pt2 = 0, dest_idx_pt2 = 0;
int intersections_clicked = 0, intersection_1, intersection_2;
std::vector<StreetSegmentIdx> path;
bool display_path = false;


int intersections_clicked_quiz, intersection_1_quiz, intersection_2_quiz;
std::vector<std::pair<IntersectionIdx, IntersectionIdx>> quiz_mode_intersections;
std::vector<StreetSegmentIdx> quiz_mode_path;

// POI Buttons Functionality
bool show_POI_sustenance = false;
bool show_POI_healthcare = false;
bool show_POI_transportation = false;
bool show_POI_others = false;

// Game Switch Colour Mode
bool quiz_mode = false;
int prev_inter_id;
GtkSwitch *game_switch;

// Depot Mode Switch
bool depot_mode = false;
GtkSwitch *depot_switch;
std::vector<IntersectionIdx> depots_list;
std::vector<DeliveryInf> deliveries_list;

// Search Bar Variables for AutoComplete
GtkButton *go_button;
GtkButton *help_button;
GtkButton *directions_button;
GtkButton *clear_button;
GtkListStore *Street_names_list;
GtkTreeIter iter;
GtkEntry *start_entry, *dest_entry, *start_entry_pt2, *dest_entry_pt2;
GtkEntryCompletion *start_completion, *dest_completion, *start_completion_pt2, *dest_completion_pt2;
GtkLabel *label_1, *label_2, *label_3;

//---------------------------------------- End Callback Global Variables -----------------------------------------//

void set_up_drop_downs(ezgl::application *application);
void set_up_buttons(ezgl::application *application);
void set_up_search_entry(ezgl::application *application);
void set_up_entry_completion(ezgl::application *application);
void set_up_POIs(ezgl::application *application);

//---------------------------------------- Setup Callback Functions -----------------------------------------//
// Determines functionality of zooming
void initial_setup(ezgl::application *application, bool)
{

    // If zoom detected, refresh drawing
    if (zoom_ratio != prev_zoom_ratio)
    {
        prev_zoom_ratio = zoom_ratio;
        application->refresh_drawing();
    }

    set_up_drop_downs(application);

    set_up_buttons(application);

    set_up_search_entry(application);

    set_up_entry_completion(application);

    set_up_POIs(application);
}
//----------------------------------------  Setup Callback Functions -----------------------------------------//

//---------------------------------------- Setup Callback Helper Functions -----------------------------------------//

// Set up dropdowns
void set_up_drop_downs(ezgl::application *application)
{
    GObject *map_dropdown = application->get_object("MapDropdown");
    g_signal_connect(map_dropdown, "changed", G_CALLBACK(draw_new_map), application);

    GObject *mode_dropdown = application->get_object("SearchModeDropdown");
    g_signal_connect(mode_dropdown, "changed", G_CALLBACK(act_on_search_mode_switch), application);
}

// Set up buttons  for directions, clearing the map, help button and the Go button for searching.
// Set up the switch for changing colour mdoes
void set_up_buttons(ezgl::application *application)
{
    // Set up switch
    game_switch = GTK_SWITCH(application->get_object("GameSwitch"));
    g_signal_connect(game_switch, "state-set", G_CALLBACK(act_on_game_switch_flip), application);

    depot_switch = GTK_SWITCH(application->get_object("DepotSwitch"));
    g_signal_connect(depot_switch, "state-set", G_CALLBACK(act_on_depot_switch_flip), application);
    gtk_widget_set_visible(GTK_WIDGET(depot_switch), FALSE);

    // Set up buttons
    go_button = GTK_BUTTON(application->get_object("GoButton"));
    g_signal_connect(go_button, "clicked", G_CALLBACK(act_on_go_button_click), application);

    help_button = GTK_BUTTON(application->get_object("HelpButton"));
    g_signal_connect(help_button, "clicked", G_CALLBACK(act_on_help_button_click), application);

    directions_button = GTK_BUTTON(application->get_object("DirectionsButton"));
    gtk_widget_set_visible(GTK_WIDGET(directions_button), FALSE);
    g_signal_connect(directions_button, "clicked", G_CALLBACK(act_on_directions_button_click), application);

    clear_button = GTK_BUTTON(application->get_object("ClearButton"));
    gtk_widget_set_visible(GTK_WIDGET(clear_button), FALSE);
    g_signal_connect(clear_button, "clicked", G_CALLBACK(act_on_clear_button_click), application);
}

// Get input from text boxes
void set_up_search_entry(ezgl::application *application)
{

    // Populate list of street names
    Street_names_list = GTK_LIST_STORE(application->get_object("StreetNamesList"));
    for (int street_id = 0; street_id < getNumStreets(); street_id++)
    {
        gtk_list_store_append(Street_names_list, &iter);
        gtk_list_store_set(Street_names_list, &iter, 0, Streets_graphics_data->all_streets[street_id].name.c_str(), -1);
    }

    start_entry = GTK_ENTRY(application->get_object("StartEntry"));
    gtk_entry_set_placeholder_text(GTK_ENTRY(start_entry), "Type Street 1 here...");
    g_signal_connect(start_entry, "activate", G_CALLBACK(act_on_search_match_selected), application);

    dest_entry = GTK_ENTRY(application->get_object("DestEntry"));
    gtk_entry_set_placeholder_text(GTK_ENTRY(dest_entry), "Type Street 2 here...");
    g_signal_connect(dest_entry, "activate", G_CALLBACK(act_on_search_match_selected), application);

    label_1 = GTK_LABEL(application->get_object("label_1"));
    gtk_widget_set_visible(GTK_WIDGET(label_1), FALSE);

    label_2 = GTK_LABEL(application->get_object("label_2"));
    gtk_widget_set_visible(GTK_WIDGET(label_2), FALSE);

    label_3 = GTK_LABEL(application->get_object("label_3"));
    gtk_widget_set_visible(GTK_WIDGET(label_3), FALSE);

    start_entry_pt2 = GTK_ENTRY(application->get_object("StartEntryPart2"));
    gtk_widget_set_visible(GTK_WIDGET(start_entry_pt2), FALSE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(start_entry_pt2), "Type Street 3 here...");
    g_signal_connect(start_entry_pt2, "activate", G_CALLBACK(act_on_search_match_selected), application);

    dest_entry_pt2 = GTK_ENTRY(application->get_object("DestEntryPart2"));
    gtk_widget_set_visible(GTK_WIDGET(dest_entry_pt2), FALSE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(dest_entry_pt2), "Type Street 4 here...");
    g_signal_connect(dest_entry_pt2, "activate", G_CALLBACK(act_on_search_match_selected), application);
}

// Set up entry completion
void set_up_entry_completion(ezgl::application *application)
{

    start_completion = GTK_ENTRY_COMPLETION(application->get_object("StartCompletion"));
    g_signal_connect(start_completion, "match_selected", G_CALLBACK(act_on_search_match_completion_selected), application);
    gtk_entry_completion_set_model(start_completion, GTK_TREE_MODEL(Street_names_list));
    gtk_entry_set_completion(GTK_ENTRY(start_entry), start_completion);

    dest_completion = GTK_ENTRY_COMPLETION(application->get_object("DestCompletion"));
    g_signal_connect(dest_completion, "match_selected", G_CALLBACK(act_on_search_match_completion_selected), application);
    gtk_entry_completion_set_model(dest_completion, GTK_TREE_MODEL(Street_names_list));
    gtk_entry_set_completion(GTK_ENTRY(dest_entry), dest_completion);

    start_completion_pt2 = GTK_ENTRY_COMPLETION(application->get_object("StartCompletion_2"));
    g_signal_connect(start_completion_pt2, "match_selected", G_CALLBACK(act_on_search_match_completion_selected), application);
    gtk_entry_completion_set_model(start_completion_pt2, GTK_TREE_MODEL(Street_names_list));
    gtk_entry_set_completion(GTK_ENTRY(start_entry_pt2), start_completion_pt2);

    dest_completion_pt2 = GTK_ENTRY_COMPLETION(application->get_object("DestCompletion_2"));
    g_signal_connect(dest_completion_pt2, "match_selected", G_CALLBACK(act_on_search_match_completion_selected), application);
    gtk_entry_completion_set_model(dest_completion_pt2, GTK_TREE_MODEL(Street_names_list));
    gtk_entry_set_completion(GTK_ENTRY(dest_entry_pt2), dest_completion_pt2);
}

// Get input for when POI buttons are pressed
void set_up_POIs(ezgl::application *application)
{
    GtkButton *sustenance_button = GTK_BUTTON(application->get_object("sustenanceButton"));
    g_signal_connect(sustenance_button, "clicked", G_CALLBACK(act_on_sustenance_button_click), application);

    GtkButton *healthcare_button = GTK_BUTTON(application->get_object("healthcareButton"));
    g_signal_connect(healthcare_button, "clicked", G_CALLBACK(act_on_healthcare_button_click), application);

    GtkButton *transportation_button = GTK_BUTTON(application->get_object("transportationButton"));
    g_signal_connect(transportation_button, "clicked", G_CALLBACK(act_on_transportation_button_click), application);

    GtkButton *others_button = GTK_BUTTON(application->get_object("othersButton"));
    g_signal_connect(others_button, "clicked", G_CALLBACK(act_on_others_button_click), application);
}

//---------------------------------------- End Setup Callback Helper Functions -----------------------------------------//




//---------------------------------------- Initial Draw Callback Function -----------------------------------------//

// Used to redraw map when new city is loaded
void draw_new_map(GtkComboBoxText *self, ezgl::application *application)
{

    // Get the map user changed to
    GtkComboBox *map_dropdown = GTK_COMBO_BOX(application->get_object("MapDropdown"));
    const gchar *char_ptr_id = gtk_combo_box_get_active_id(map_dropdown);
    std::string str_id(char_ptr_id);

    map_name = gtk_combo_box_text_get_active_text(self);

    std::cout << "New map: " << str_id << std::endl;

    // Clear all graphics data, load new data
    closeMap();
    clear_map_data();
    loadMap(str_id);

    // Initialize graphics data classes
    calculateLatAvg();

    Intersections_graphics_data = new IntersectionsGraphicsData();
    Streets_graphics_data = new StreetsGraphicsData();
    POIs_graphics_data = new POIGraphicsData();
    Features_graphics_data = new FeaturesGraphicsData();

    // Populate list of street names
    Street_names_list = GTK_LIST_STORE(application->get_object("StreetNamesList"));
    for (int street_id = 0; street_id < getNumStreets(); street_id++)
    {
        gtk_list_store_append(Street_names_list, &iter);
        gtk_list_store_set(Street_names_list, &iter, 0, Streets_graphics_data->all_streets[street_id].name.c_str(), -1);
    }

    // Redraw map
    ezgl::rectangle new_world({x_from_lon(min_lon), y_from_lat(min_lat)}, {x_from_lon(max_lon), y_from_lat(max_lat)});
    application->change_canvas_world_coordinates("MainCanvas", new_world);

    application->refresh_drawing();
}
//-------------------------------------- End Initial Draw Callback Functions ---------------------------------------//
