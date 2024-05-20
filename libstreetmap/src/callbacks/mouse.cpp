
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
#include "m4.h"
#include "graphics_data/city_information.h"
#include "graphics_data/graphics_globals.h"
#include <limits.h>

bool check_if_path_seleected();

//---------------------------------------- Mouse Movement Callback Functions -----------------------------------------//
// Determines functionality of mouse clicks
void act_on_mouse_click(ezgl::application *application, GdkEventButton * /*event*/, double x, double y)
{
    // Get current position of mouse and closest intersection
    LatLon position = LatLon(lat_from_y(y), lon_from_x(x));

    // If zoom level sufficient
    if (zoom_ratio < 1 && search_mode == FASTEST_ROUTE)
    {

        // Find and highlight closest intersection, and route if enough intersections clicked
        int inter_id = findClosestIntersection(position);

        if (findDistanceBetweenTwoPoints(position, Intersections_graphics_data->intersections[inter_id].position) < (8 + 40 * zoom_ratio)){
            
            if(!quiz_mode) {
                std::string title = "Intersection " + std::to_string(inter_id);

                // Highlight intersection
                if (Intersections_graphics_data->intersections[inter_id].highlight == false) {
                    Intersections_graphics_data->intersections[inter_id].highlight = true;
                    intersections_clicked += 1;

                    if (intersections_clicked == 1){
                        intersection_1 = inter_id;
                    }
                }

                // Un-highlight intersection
                else if (Intersections_graphics_data->intersections[inter_id].highlight == true) {
                    Intersections_graphics_data->intersections[inter_id].highlight = false;
                    intersections_clicked -= 1;

                    if (Intersections_graphics_data->intersections[intersection_1].highlight == false) {
                        intersection_1 = intersection_2;
                    }
                }

                // If 2 intersections selected, highlight route
                if (intersections_clicked == 2 && inter_id != intersection_2) {
                    intersection_2 = inter_id;
                    Intersections_graphics_data->intersections[intersection_2].highlight = true;

                    path = findPathBetweenIntersections(15, std::pair{intersection_1, intersection_2});
                    
                    if (path.size() == 0) {
                        application->create_popup_message_with_callback(act_on_popup_features_done_button, "Error: No path", "");
                        Intersections_graphics_data->intersections[intersection_1].highlight = false;
                        Intersections_graphics_data->intersections[intersection_2].highlight = false;
                        intersections_clicked = 0;
                        return;
                    }
                }

                // If new route is selected, un-highlight old one
                if (intersections_clicked == 3) {
                    Intersections_graphics_data->intersections[intersection_1].highlight = false;
                    Intersections_graphics_data->intersections[intersection_2].highlight = false;

                    intersection_1 = inter_id;
                    intersections_clicked = 1;
                }

                if(intersections_clicked == 2) {
                    display_path = true;
                }
                else {
                    display_path = false;
                }

                application->refresh_drawing();
                return;
            }

            else {
                // Highlight intersection
                if (Intersections_graphics_data->intersections[inter_id].clicked_quiz == false) {
                    Intersections_graphics_data->intersections[inter_id].clicked_quiz = true;
                    intersections_clicked_quiz += 1;

                    if (intersections_clicked_quiz == 1){
                        intersection_1_quiz = inter_id;
                    }
                }

                // Un-highlight intersection
                else if (Intersections_graphics_data->intersections[inter_id].clicked_quiz == true) {
                    Intersections_graphics_data->intersections[inter_id].clicked_quiz = false;
                    intersections_clicked_quiz -= 1;

                    if (Intersections_graphics_data->intersections[intersection_1_quiz].clicked_quiz == false) {
                        intersection_1_quiz = intersection_2_quiz;
                    }
                }

                // If 2 intersections selected, highlight route
                if (intersections_clicked_quiz == 2 && inter_id != intersection_1_quiz) {
                    intersection_2_quiz = inter_id;
                    Intersections_graphics_data->intersections[intersection_2_quiz].clicked_quiz = true;
                    quiz_mode_intersections.push_back({intersection_1_quiz, intersection_2_quiz});

                    std::vector<StreetSegmentIdx> temp_path = findPathBetweenIntersections(15, std::pair{intersection_1_quiz, intersection_2_quiz});
                    quiz_mode_path.insert(quiz_mode_path.end(), temp_path.begin(), temp_path.end());

                    intersection_1_quiz = intersection_2_quiz;
                    intersections_clicked_quiz = 1;
                }

                application->refresh_drawing();
                return;
            }
        }
    }

    // Travelling Saleman mode
    else if (zoom_ratio < 1 && search_mode == DELIVERY)
    { 

        // Find and highlight closest intersection
        int inter_id = findClosestIntersection(position);
        
        if (findDistanceBetweenTwoPoints(position, Intersections_graphics_data->intersections[inter_id].position) < (8 + 40 * zoom_ratio))
        {
            Intersections_graphics_data->intersections[inter_id].highlight ^= true;

            // If switch set to depots mode, add to depot array and display depot png
            if (!depot_mode)
            {
                // Check if already in array, if so, delete it
                auto depot_it = std::find(depots_list.begin(), depots_list.end(), inter_id);
                if (depot_it != depots_list.end())
                {
                    depots_list.erase(depot_it);
                }

                // Else add to array
                else
                {
                    depots_list.push_back(inter_id);
                }
                
            }

            // If switch set to deliveries, add to deliveries array and display appropriate png
            else 
            {
                intersections_clicked++;

                // If first intersection, pickup location
                if (intersections_clicked == 1) 
                {
                    intersection_1 = inter_id;
                    intersection_2 = -1;
                }

                // If same intersection, deselect
                if (intersections_clicked == 2 && inter_id == intersection_1)
                {
                    intersections_clicked = 0;
                }

                // If second intersection, dropoff location
                else if (intersections_clicked == 2) 
                {
                    intersection_2 = inter_id;
                }

                // Put intersections into DeliveryInf object, add to array
                if (intersection_2 != -1)
                {
                    DeliveryInf new_delivery(intersection_1, intersection_2);
                    deliveries_list.push_back(new_delivery);
                    intersections_clicked = 0;
                }
            }

            std::cout << "Depots list: ";
            for (int i = 0; i < depots_list.size(); i++) {
                std::cout << depots_list[i] << " | ";
            }
            std::cout << std::endl;

            std::cout << "Deliveries list: ";
            for (int i = 0; i < deliveries_list.size(); i++) {
                std::cout << deliveries_list[i].pickUp << ", " << deliveries_list[i].dropOff << " | ";
            }
            std::cout << std::endl;

            application->refresh_drawing();
        }
    }
}

bool check_if_path_seleected(){
    // check if the path is correctly selected
    return false;
}


// Determines what happens when a feature popup done button is pressed
void act_on_popup_features_done_button(GtkDialog *self, gint /*response_id*/, ezgl::application *application)
{

    // Check if within feature
    for (auto &feature : Features_graphics_data->features_by_area)
    {
        feature.second.highlight = false;
    }

    gtk_widget_destroy(GTK_WIDGET(self));
    application->refresh_drawing();
}

// Determines functionality of mouse movement
void act_on_mouse_move(ezgl::application *application, GdkEventButton * /*event*/, double x, double y)
{

    // Show intersection data if sufficiently zoomed in
    if (zoom_ratio < 0.03 && search_mode == 0)
    {

        std::stringstream ss;

        // Get current position of mouse and closest intersection
        LatLon position = LatLon(lat_from_y(y), lon_from_x(x));
        int inter_id = findClosestIntersection(position);

        // Turn off highlight when moving off of intersection
        if (inter_id != prev_inter_id || findDistanceBetweenTwoPoints(position, Intersections_graphics_data->intersections[inter_id].position) > 5)
        {
            Intersections_graphics_data->intersections[prev_inter_id].highlight = false;
            prev_inter_id = inter_id;

            ss << "";
            application->update_message(ss.str());
        }

        // If closest intersection within 5m of mouse, highlight intersection + display name in status bar
        if (findDistanceBetweenTwoPoints(position, Intersections_graphics_data->intersections[inter_id].position) < 5)
        {
            if (Intersections_graphics_data->intersections[inter_id].highlight == false)
            {
                Intersections_graphics_data->intersections[inter_id].highlight = true;
            }

            ss << "Intersection: " << Intersections_graphics_data->intersections[inter_id].name;
            application->update_message(ss.str());
        }

        application->refresh_drawing();
    }
}