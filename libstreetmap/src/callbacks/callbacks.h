/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 15/03/2024
 *
 * Description: Contains callback functions and global variables. Includes functionality for 
 * setting up the initial GI, drawing a new map, handling user interactions, and managing search 
 * bar functionality, etc.
*/

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "StreetsDatabaseAPI.h"
#include "m4.h"


// ----------------------------------------- Global Variables ----------------------------------------- // 

// Graphics Camera View Information
extern ezgl::rectangle visible_world;
extern double world_width, world_height;
extern double zoom_ratio, prev_zoom_ratio;
extern std::string map_name;

// Search Bar Functionality
extern int search_mode;
#define INTERSECTION 0
#define FASTEST_ROUTE 1
#define DELIVERY 2

extern int start_idx, dest_idx;
extern int start_idx_pt2, dest_idx_pt2;
extern int intersections_clicked, intersection_1, intersection_2;
extern bool display_path;
extern std::vector<StreetSegmentIdx> path;

extern int intersections_clicked_quiz, intersection_1_quiz, intersection_2_quiz;
extern std::vector<std::pair<IntersectionIdx, IntersectionIdx>> quiz_mode_intersections;
extern std::vector<StreetSegmentIdx> quiz_mode_path;

// POI Buttons Functionality
extern bool show_POI_sustenance;
extern bool show_POI_healthcare;
extern bool show_POI_transportation;
extern bool show_POI_others;

// Game Switch Colour Mode
extern bool quiz_mode;
extern int prev_inter_id;
extern GtkSwitch *game_switch;

// Depot Mode Switch
extern bool depot_mode;
extern GtkSwitch *depot_switch;
extern std::vector<IntersectionIdx> depots_list;
extern std::vector<DeliveryInf> deliveries_list;

// Search Bar Variables for AutoComplete
extern GtkButton *go_button;
extern GtkButton *help_button;
extern GtkButton *directions_button;
extern GtkButton *clear_button;
extern GtkListStore *Street_names_list;
extern GtkTreeIter iter;
extern GtkEntry *start_entry, *dest_entry, *start_entry_pt2, *dest_entry_pt2;
extern GtkEntryCompletion *start_completion, *dest_completion, *start_completion_pt2, *dest_completion_pt2;
extern GtkLabel *label_1, *label_2, *label_3;
// ----------------------------------------- End of Global Variables ----------------------------------------- // 


// ----------------------------------------- Callback functions ----------------------------------------- // 
void initial_setup(ezgl::application *, bool);
void draw_new_map(GtkComboBoxText *self, ezgl::application *application);

// Callback for switching the game mode
bool act_on_game_switch_flip(GtkSwitch * /*game_switch*/, gboolean switch_state, ezgl::application *application);
bool act_on_depot_switch_flip(GtkSwitch * /*depot_switch*/, gboolean switch_state, ezgl::application */*application*/);

// Callback for switching the search mode
void act_on_search_mode_switch(GtkComboBoxText *self, ezgl::application *application);

// Callback function for clicking on features on intersections
void act_on_mouse_click(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_mouse_move(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_popup_done_button(GtkDialog *self, gint response_id, ezgl::application *application);
void act_on_popup_features_done_button(GtkDialog *self, gint response_id, ezgl::application *application);

// Callbacks for search bar and autocomplete
void act_on_go_button_click(GtkButton *go_button, ezgl::application *application, ezgl::renderer *g);
void create_popup_dialog(ezgl::application *application, std::vector<StreetIdx> street_ids, std::string type, int intersection);
void on_combo_changed_1(GtkComboBox *widget, gpointer data);
void on_combo_changed_2(GtkComboBox *widget, gpointer data);

// Callbacks for other buttons
void act_on_help_button_click(GtkButton *help_button, ezgl::application *application);
void act_on_directions_button_click(GtkButton *directions_button, ezgl::application *application);
void act_on_clear_button_click(GtkButton *clear_button, ezgl::application *application);

// Callbacks for search entry
void act_on_search_match_selected(GtkEntry *entry, gpointer user_data, ezgl::application *application);
gboolean act_on_search_match_completion_selected(GtkEntryCompletion *widget, GtkTreeModel *model, GtkTreeIter *iter_in, gpointer user_data);

// Callbacks for POI buttons
void act_on_sustenance_button_click(GtkButton *POI_button, ezgl::application *application);
void act_on_healthcare_button_click(GtkButton *POI_button, ezgl::application *application);
void act_on_transportation_button_click(GtkButton *POI_button, ezgl::application *application);
void act_on_others_button_click(GtkButton *POI_button, ezgl::application *application);

#endif

// ----------------------------------------- End of Callback Functions ----------------------------------------- // 

