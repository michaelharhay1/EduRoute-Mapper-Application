
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

// Search Helper functions
std::vector<IntersectionIdx> get_share_intersections(GtkEntry *start_entry, GtkEntry *dest_entry, ezgl::application *application, int intersection);
void move_camera(std::vector<IntersectionIdx> shared_intersections, ezgl::application *application, std::stringstream &ss, int intersection);

//---------------------------------------- Search Callback Functions -----------------------------------------//

// Determines functionality of go button press, when the go button is pressed, the user can either search
// for an intersection between 2 streets or the path between 2 intersections

void act_on_go_button_click(GtkButton * /*go_button*/, ezgl::application *application, ezgl::renderer * /*g*/)
{

    std::stringstream ss;

    // Get the shared intersections between the 2 streets input in the first search bar
    start_entry = GTK_ENTRY(application->get_object("StartEntry"));
    dest_entry = GTK_ENTRY(application->get_object("DestEntry"));
    std::vector<IntersectionIdx> shared_intersections = get_share_intersections(start_entry, dest_entry, application, 1);

    if (shared_intersections.size() == 0)
    {
        return;
    }

    // In the first mode, move to the intersection searched by the first 2 search bars
    if (search_mode == INTERSECTION)
    {
        move_camera(shared_intersections, application, ss, 1);
    }

    // In the second mode, find the path between 2 search bars
    if (search_mode == FASTEST_ROUTE)
    {

        // Get the shared intersections between the 2 streets input in the first search bar
        start_entry_pt2 = GTK_ENTRY(application->get_object("StartEntryPart2"));
        dest_entry_pt2 = GTK_ENTRY(application->get_object("DestEntryPart2"));
        std::vector<IntersectionIdx> shared_intersections_2 = get_share_intersections(start_entry_pt2, dest_entry_pt2, application, 2);

        if (shared_intersections_2.size() == 0)
        {
            return;
        }

        // Highlight the start and end intersections
        intersection_1 = shared_intersections[0];
        intersection_2 = shared_intersections_2[0];
        path = findPathBetweenIntersections(15, std::pair{intersection_1, intersection_2});

        // Defensive coding, make sure a path exists
        if (path.size() == 0)
        {
            Intersections_graphics_data->intersections[intersection_1].highlight = false;
            Intersections_graphics_data->intersections[intersection_2].highlight = false;
            intersections_clicked = 0;
            application->create_popup_message_with_callback(act_on_popup_features_done_button, "Error: No path", "");
            return;
        }

        intersections_clicked = 2;
        Intersections_graphics_data->intersections[intersection_1].highlight = true;
        Intersections_graphics_data->intersections[intersection_2].highlight = true;

        // Move the camera to the destination
        move_camera(shared_intersections_2, application, ss, 2);
    }

    // Reset the indexes
    start_idx = 0;
    dest_idx = 0;

    start_idx_pt2 = 0;
    dest_idx_pt2 = 0;

    // Update status bar, refresh drawing
    application->update_message(ss.str());
    application->refresh_drawing();
}

// Takes care of finding the intersetions between the 2 streets entered in the search entry,
// Gives the user the option to decide which search entry they want to select through multi-match
std::vector<IntersectionIdx> get_share_intersections(GtkEntry *start_entry_in, GtkEntry *dest_entry_in, ezgl::application *application, int intersection)
{

    const gchar *start_gchar = gtk_entry_get_text(start_entry_in);
    const gchar *dest_gchar = gtk_entry_get_text(dest_entry_in);
    std::string start_str(start_gchar);
    std::string dest_str(dest_gchar);

    // Find street IDs from partial name, add to vector
    std::vector<StreetIdx> street_ids_start = findStreetIdsFromPartialStreetName(start_str);
    std::vector<StreetIdx> street_ids_dest = findStreetIdsFromPartialStreetName(dest_str);

    // Defensive code to prevent segfault if no valid street name input
    if (street_ids_start.size() < 1 || street_ids_dest.size() < 1)
    {
        application->create_popup_message_with_callback(act_on_popup_features_done_button, "Error: Invalid Entry", "Please enter a valid street name.");
        return {};
    }

    // If multiple streets with same name, prompt user for selection
    if (street_ids_start.size() > 1)
    {
        create_popup_dialog(application, street_ids_start, "start", intersection);
    }
    if (street_ids_dest.size() > 1)
    {
        create_popup_dialog(application, street_ids_dest, "dest", intersection);
    }

    // Depending on which search bar is being used, set the index correctly
    int start_index, dest_index;
    if (intersection == 1)
    {
        start_index = start_idx;
        dest_index = dest_idx;
    }
    else
    {
        start_index = start_idx_pt2;
        dest_index = dest_idx_pt2;
    }

    // Place street IDs into pair, find all shared intersections and store in vector
    std::pair<StreetIdx, StreetIdx> street_ids = {street_ids_start[start_index], street_ids_dest[dest_index]};

    // Shared intersections
    std::vector<IntersectionIdx> shared_intersections = findIntersectionsOfTwoStreets(street_ids);

    if (shared_intersections.size() < 1)
    {
        std::string no_output_msg("No shared shared intersections between\n");
        no_output_msg.append(getStreetName(street_ids.first));
        no_output_msg.append(" and ");
        no_output_msg.append(getStreetName(street_ids.second));
        application->create_popup_message_with_callback(act_on_popup_features_done_button, "Error: No shared intersections", no_output_msg.c_str());
        return {};
    }

    return shared_intersections;
}

// Function to move the camera to the intersection the user has searched, in the find path mode, it centres the user around
// the destination intersection
void move_camera(std::vector<IntersectionIdx> shared_intersections, ezgl::application *application, std::stringstream &ss, int intersection)
{

    IntersectionIdx bottom_intersection = shared_intersections[0];
    IntersectionIdx top_intersection = shared_intersections[0];
    IntersectionIdx left_intersection = shared_intersections[0];
    IntersectionIdx right_intersection = shared_intersections[0];

    // Change highlight state of intersection, update status bar message, and set top, right, bottom, and left intersections
    for (int inter_id = 0; inter_id < shared_intersections.size(); inter_id++)
    {

        if (getIntersectionPosition(shared_intersections[inter_id]).latitude() < getIntersectionPosition(bottom_intersection).latitude())
        {
            bottom_intersection = shared_intersections[inter_id];
        }

        if (getIntersectionPosition(shared_intersections[inter_id]).latitude() > getIntersectionPosition(top_intersection).latitude())
        {
            top_intersection = shared_intersections[inter_id];
        }

        if (getIntersectionPosition(shared_intersections[inter_id]).longitude() < getIntersectionPosition(left_intersection).longitude())
        {
            left_intersection = shared_intersections[inter_id];
        }

        if (getIntersectionPosition(shared_intersections[inter_id]).longitude() > getIntersectionPosition(right_intersection).longitude())
        {
            right_intersection = shared_intersections[inter_id];
        }

        Intersections_graphics_data->intersections[shared_intersections[inter_id]].highlight = true;

        if (intersection == 1 && search_mode == 0)
        {
            ss << Intersections_graphics_data->intersections[shared_intersections[inter_id]].name << std::endl;
        }
    }

    // Add 70 to edges of world as padding
    ezgl::point2d bottom_left_point(x_from_lon(getIntersectionPosition(left_intersection).longitude()) - 70, y_from_lat(getIntersectionPosition(bottom_intersection).latitude()) - 70);
    ezgl::point2d top_right_point(x_from_lon(getIntersectionPosition(right_intersection).longitude()) + 70, y_from_lat(getIntersectionPosition(top_intersection).latitude()) + 70);
    ezgl::rectangle fitted_world(bottom_left_point, top_right_point);

    // Update camera to show all highlighted intersections
    double dx_camera = ((fitted_world.left() + fitted_world.right()) / 2) - ((visible_world.left() + visible_world.right()) / 2);
    double dy_camera = ((fitted_world.top() + fitted_world.bottom()) / 2) - ((visible_world.top() + visible_world.bottom()) / 2);
    ezgl::canvas *main_canvas = application->get_canvas("MainCanvas");
    ezgl::translate(main_canvas, dx_camera, dy_camera);
    while (fitted_world.bottom() > visible_world.bottom() && fitted_world.top() < visible_world.top() && fitted_world.left() > visible_world.left() && fitted_world.right() < visible_world.right())
    {
        zoom_in(main_canvas, 1.2);
    }
    while (fitted_world.bottom() < visible_world.bottom() && fitted_world.top() > visible_world.top() && fitted_world.left() < visible_world.left() && fitted_world.right() > visible_world.right())
    {
        zoom_out(main_canvas, 1.2);
    }
}

// Function to create and handle the popup dialog for whene multiple streets share the same name
void create_popup_dialog(ezgl::application * /*application*/, std::vector<int> street_ids, std::string type, int intersection)
{

    // Create the dialog window
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Multiple streets with the same name. Select a street:",
        NULL,
        GTK_DIALOG_MODAL,
        ("OK"),
        GTK_RESPONSE_OK,
        NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, -1); // -1 for default height

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Create combo box
    GtkWidget *combo_box = gtk_combo_box_text_new();

    // Add street names to combo box
    for (auto id : street_ids)
    {
        std::string street_name = getStreetName(id);
        street_name.append(" (Intersection ID: [");
        street_name.append(std::to_string(id));
        street_name.append("])");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), street_name.c_str());
    }

    // Connect the callback function to combo box
    if (intersection == 1)
    {
        g_signal_connect(G_OBJECT(combo_box), "changed", G_CALLBACK(on_combo_changed_1), const_cast<char *>(type.c_str()));
    }
    else
    {
        g_signal_connect(G_OBJECT(combo_box), "changed", G_CALLBACK(on_combo_changed_2), const_cast<char *>(type.c_str()));
    }

    // Add combo box to dialog
    gtk_container_add(GTK_CONTAINER(content_area), combo_box);

    // Show all widgets
    gtk_widget_show_all(dialog);

    // Run the dialog
    gtk_dialog_run(GTK_DIALOG(dialog));

    // Destroy the dialog after selection
    gtk_widget_destroy(dialog);
}

// Callback function when the user selects an item from the combo box for the first 2 search bars
void on_combo_changed_1(GtkComboBox *widget, gpointer data)
{

    const char *type = static_cast<const char *>(data);

    if (strcmp(type, "start") == 0)
    {
        start_idx = gtk_combo_box_get_active(widget);
    }

    else if (strcmp(type, "dest") == 0)
    {
        dest_idx = gtk_combo_box_get_active(widget);
    }
}

// Callback function when the user selects an item from the combo box the last 2 search bars
void on_combo_changed_2(GtkComboBox *widget, gpointer data)
{

    const char *type = static_cast<const char *>(data);

    if (strcmp(type, "start") == 0)
    {
        start_idx_pt2 = gtk_combo_box_get_active(widget);
    }

    else if (strcmp(type, "dest") == 0)
    {
        dest_idx_pt2 = gtk_combo_box_get_active(widget);
    }
}
//-------------------------------------- End Draw Callback Functions ---------------------------------------//
