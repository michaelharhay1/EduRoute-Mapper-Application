
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

//---------------------------------------- Search Callback Functions -----------------------------------------//

// Determines functionality when new search mode is selected from dropdown
void act_on_search_mode_switch(GtkComboBoxText *self, ezgl::application *application)
{

    std::string search_mode_name;

    // Get the search mode user changed to
    const gchar *char_ptr_id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(self));
    search_mode = atoi(char_ptr_id);

    // Hide / show extra search bars & directions buttons
    if (search_mode == INTERSECTION)
    {
        gtk_widget_set_visible(GTK_WIDGET(start_entry), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(dest_entry), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(start_entry_pt2), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(dest_entry_pt2), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(directions_button), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(clear_button), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(label_1), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(label_2), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(label_3), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(depot_switch), FALSE);

        // Un-highlight route if highlighted
        if (intersections_clicked > 0)
        {
            Intersections_graphics_data->intersections[intersection_1].highlight = false;
            Intersections_graphics_data->intersections[intersection_2].highlight = false;

            for (int street_seg_idx = 0; street_seg_idx < path.size(); street_seg_idx++)
            {
                Streets_graphics_data->street_segs_by_tag
                    [Streets_graphics_data->tag_num[Streets_graphics_data->all_street_segs[path[street_seg_idx]].road_type]]
                    [Streets_graphics_data->all_street_segs[path[street_seg_idx]].index_in_street_segs_by_tag];

                Streets_graphics_data->street_segs_by_tag
                    [Streets_graphics_data->tag_num[Streets_graphics_data->all_street_segs[path[street_seg_idx]].road_type]]
                    [Streets_graphics_data->all_street_segs[path[street_seg_idx]].index_in_street_segs_by_tag]
                        .highlight = false;
            }
        }
        intersections_clicked = 0;
    }

    else if (search_mode == FASTEST_ROUTE)
    {
        gtk_widget_set_visible(GTK_WIDGET(start_entry), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(dest_entry), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(start_entry_pt2), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(dest_entry_pt2), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(directions_button), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(clear_button), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(label_1), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(label_2), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(label_3), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(depot_switch), FALSE);
    }

    // Hide / show extra search bars & directions buttons
    if (search_mode == DELIVERY)
    {
        gtk_widget_set_visible(GTK_WIDGET(start_entry), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(dest_entry), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(start_entry_pt2), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(dest_entry_pt2), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(directions_button), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(clear_button), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(label_1), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(label_2), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(label_3), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(depot_switch), TRUE);

        // Un-highlight route if highlighted
        if (intersections_clicked > 0)
        {
            Intersections_graphics_data->intersections[intersection_1].highlight = false;
            Intersections_graphics_data->intersections[intersection_2].highlight = false;

            for (int street_seg_idx = 0; street_seg_idx < path.size(); street_seg_idx++)
            {
                Streets_graphics_data->street_segs_by_tag
                    [Streets_graphics_data->tag_num[Streets_graphics_data->all_street_segs[path[street_seg_idx]].road_type]]
                    [Streets_graphics_data->all_street_segs[path[street_seg_idx]].index_in_street_segs_by_tag];

                Streets_graphics_data->street_segs_by_tag
                    [Streets_graphics_data->tag_num[Streets_graphics_data->all_street_segs[path[street_seg_idx]].road_type]]
                    [Streets_graphics_data->all_street_segs[path[street_seg_idx]].index_in_street_segs_by_tag]
                        .highlight = false;
            }
        }
        intersections_clicked = 0;
    }

    application->refresh_drawing();
}

//-------------------------------------- End Draw Callback Functions ---------------------------------------//
