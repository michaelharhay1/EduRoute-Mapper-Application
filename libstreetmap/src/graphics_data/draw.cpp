/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 15/03/2024
 *
 * Description: Contains functions for calculating city information, such as 
 * average latitude and conversion between lat/lon and x/y. Defines functions 
 * conversions and average lat.
 */

#include "draw.h"
#include "StreetsDatabaseAPI.h"




//------------------------------------------- Global Variables --------------------------------------------//
IntersectionsGraphicsData *Intersections_graphics_data;
StreetsGraphicsData *Streets_graphics_data;
POIGraphicsData *POIs_graphics_data;
FeaturesGraphicsData *Features_graphics_data;

// Graphics PNG data
ezgl::surface* start_image = ezgl::renderer::load_png("libstreetmap/resources/POI_icons/start.png");
ezgl::surface* dest_image = ezgl::renderer::load_png("libstreetmap/resources/POI_icons/dest.png");

//---------------------------------------- Draw Function Definitions ----------------------------------------//

// Draws canvas, features, POIs, intersections, and street segments
void draw_main_canvas(ezgl::renderer *g) {  
    
    // Calculate zoom ratio
    visible_world = g->get_visible_world();
    double current_world_width = visible_world.width();
    zoom_ratio = (current_world_width)/world_width;
    
    // Draw background colour to deep blue if in Quiz Mode
    if (quiz_mode) {
        g->set_color(9, 46, 75);
    }
    else{
        g->set_color(247, 236, 222); // Pale Yellow
    }

    g->fill_rectangle(visible_world);  
    
    // Draw map
    draw_features(g);

    // Draw streets segments
    draw_streets(g);

    // Draw intersections on top of streets
    draw_intersections(g);  

    // Draw city name if zoom level is high enough
    draw_cityname(g);

    // Draw all POIs
    draw_POIs(g);
    
}


// Deletes dynamically allocated variables
void clear_map_data(){

    // Clear all city data to make space for new map
    delete Intersections_graphics_data; 
    delete Streets_graphics_data;
    delete POIs_graphics_data;
    delete Features_graphics_data;
      
    // Clear graphics data
    gtk_list_store_clear (Street_names_list);    
}

//-------------------------------------- End Draw Function Definitions --------------------------------------//





//------------------------------------------ Draw Helper Function ------------------------------------------//

// Draw Features
void draw_features(ezgl::renderer *g) {
    
    for (auto feature = Features_graphics_data->features_by_area.rbegin(); feature!= Features_graphics_data->features_by_area.rend(); feature++) {      

        // If zoomed out, hide elements 
        if (zoom_ratio > 1 && feature->first < 300000) {
            continue;
        }
       
        if (zoom_ratio > 0.75 && feature->first < 150000) {
            continue;
        }
        
        if (zoom_ratio > 0.45 && feature->first < 50000) {
            continue;
        }

        if (zoom_ratio > 0.25 && feature->first < 10000) {
            continue;
        }

        if (zoom_ratio > 0.15 && feature->first < 5000) {
            continue;
        }

        if (zoom_ratio > 0.05 && feature->first < 2500) {
            continue;
        }

        if (zoom_ratio > 0.03 && feature->first < 500) {
            continue;
        }  

        // make highlighted features red, otherwise assigned colour
        if (feature->second.highlight) {
            g->set_color(ezgl::RED);
        }
        
        else {
            if (quiz_mode) {
                g->set_color(feature->second.gameColor);
            }
            else{
                g->set_color(feature->second.color);
            }
        }

        if (feature->second.isFeatureClosed && feature->second.feature_points.size() > 1) {
            g->fill_poly(feature->second.feature_points);
        } 

        else {
            g->set_line_width(2);

            for (size_t feature_id = 0; feature_id < feature->second.feature_points.size() - 1; feature_id++) {
                g->draw_line(feature->second.feature_points[feature_id], feature->second.feature_points[feature_id + 1]);
            }
        }
    }
}

// Draw POIs
void draw_POIs(ezgl::renderer *g){
    
    // Set color and width
    g->set_color (ezgl::BLACK); 
    double width = 3;

    if (zoom_ratio < 0.02) {

        // rectangle = world_to_screen()
        for (auto poi_amenity = POIs_graphics_data->POI_amenities.begin(); poi_amenity != POIs_graphics_data->POI_amenities.end(); poi_amenity++){ 

            if((show_POI_sustenance  && poi_amenity->first == "sustenance") ||  (show_POI_healthcare && poi_amenity->first == "healthcare") 
              || (show_POI_transportation && poi_amenity->first == "transportation")  ||  (show_POI_others && poi_amenity->first == "others")){

                for(int poi_id = 0; poi_id < poi_amenity->second.size(); poi_id++){

                    if (g->get_visible_world().contains(poi_amenity->second[poi_id].xy_loc.x, poi_amenity->second[poi_id].xy_loc.y + 1)) {
                        
                        g->set_color(poi_amenity->second[poi_id].colour);

                        g->draw_surface(poi_amenity->second[poi_id].image, {poi_amenity->second[poi_id].xy_loc.x, poi_amenity->second[poi_id].xy_loc.y}, 0.5);
                        
                        // Write text if suficiently zoomed in
                        if (zoom_ratio < 0.005) {
                            g->set_color(ezgl::BLACK);
                            g->set_font_size (8); 
                            g->draw_text ({poi_amenity->second[poi_id].xy_loc.x, poi_amenity->second[poi_id].xy_loc.y + width}, poi_amenity->second[poi_id].name);
                        }
                    }     
                }
            }
        }
    }
}

// Draw street segments
void draw_streets(ezgl::renderer *g) {
    
    // Use zoom ratio to determine which street segs are drawn
    int max_feature_tag;
    double dynamic_width_ratio = 1;
    double arrow_length = 5;
 

    // Zoom level 1
    if (zoom_ratio >= 1) {
        max_feature_tag = 1;
    }

    // Zoom level 2
    else if (zoom_ratio >= 0.75) {
        max_feature_tag = 2;
    }

    // Zoom level 3
    else if (zoom_ratio >= 0.45) {
        max_feature_tag = 3;
    }

    // Zoom level 4
    else if (zoom_ratio >= 0.25) {
        max_feature_tag = 4;
    }

    // Zoom level 5
    else if (zoom_ratio >= 0.15) {
        max_feature_tag = 5;
    }

    // Zoom level 6
    else if (zoom_ratio >= 0.05) {
        max_feature_tag = 6;
        dynamic_width_ratio = 1.5;
    }

    // Zoom level 7
    else if (zoom_ratio >= 0.03) {
        max_feature_tag = 10;
        dynamic_width_ratio = 1.5;
    }

    // Zoom level 8
    else if (zoom_ratio >= 0.02) {
        max_feature_tag = 28;
        dynamic_width_ratio = 1.75;
        arrow_length = 20;
    }

    // Zoom level 9
    else if (zoom_ratio >= 0.01) {
        max_feature_tag = 28;
        dynamic_width_ratio = 2;
        arrow_length = 15;
    }

    // Zoom level 10
    else if (zoom_ratio >= 0.005) {
        max_feature_tag = 28;
        dynamic_width_ratio = 3;
        arrow_length = 12;
    }

    // Zoom level 11
    else {
        max_feature_tag = 28;
        dynamic_width_ratio = 5;
        arrow_length = 10;
    }

    // Draw street segs
    for (size_t tag_num = 0; tag_num < max_feature_tag; tag_num++) {
        for(const auto &street_seg : Streets_graphics_data->street_segs_by_tag[tag_num]){
                double line_width = 0;

                // Quiz mode colours
                if (quiz_mode) {

                    if (tag_num == 0) {
                        line_width = 4*dynamic_width_ratio;
                        g->set_line_width(line_width);
                        g->set_color(255, 255, 255); // White for major roads
                    }

                    else if (tag_num < 3) {
                        line_width = 3*dynamic_width_ratio;
                        g->set_line_width(line_width);
                        g->set_color(207,207,207); // Light Grey
                    }

                    else {
                        // if(zoom_ratio)
                        line_width = 2*dynamic_width_ratio;
                        g->set_line_width(line_width);
                        g->set_color(151,151,151); // Darker Grey
                    }
                }


                else{
                    if (tag_num == 0) {
                        line_width = 4*dynamic_width_ratio;
                        g->set_line_width(line_width);
                        g->set_color(255, 178, 102);
                    }

                    else if (tag_num < 3) {
                        line_width = 3*dynamic_width_ratio;
                        g->set_line_width(line_width);
                        g->set_color(255, 102, 102);
                    }

                    else {
                        line_width = 2.75*dynamic_width_ratio;
                        g->set_line_width(line_width);
                        g->set_color(204, 155, 153);
                    }
                }

                if(quiz_mode && street_seg.highlight){
                    g->set_color(255, 0, 0);
                }
                
                // If no curve points in street segment
                if (street_seg.num_curve_points == 0) {
                    g->draw_line(street_seg.from_xy_loc, street_seg.to_xy_loc);
                }

                // If curve points in street segment
                else {
                    // Draw line from "from" to first curve point
                    g->draw_line(street_seg.from_xy_loc, street_seg.curve_points_xy_loc[0]);

                    // Draw lines for all curve points
                    for (int curve_id = 1; curve_id < street_seg.num_curve_points; ++curve_id) {
                        g->draw_line(street_seg.curve_points_xy_loc[curve_id - 1], street_seg.curve_points_xy_loc[curve_id]);
                    }

                    // Draw line from last curve point to "to"
                    g->draw_line(street_seg.curve_points_xy_loc[street_seg.num_curve_points - 1], street_seg.to_xy_loc);
                }
                
                
                // Call functions to draw arrows for one-way streets and paths
                if(zoom_ratio < 0.01){
                    if (street_seg.oneWay) {
                        draw_streets_one_way_arrows(g, street_seg, dynamic_width_ratio);
                    }
                }

                // Call function to draw the street names
                if (g->get_visible_world().contains(street_seg.from_xy_loc.x, street_seg.from_xy_loc.y) && street_seg.name != "<unknown>"){
                   draw_streets_names(g, street_seg, dynamic_width_ratio, tag_num); 
                }
        }
    } 

    if(display_path){
        if(path.size() > 0){
            g->set_line_width(10);
            
            if (quiz_mode) {
                g->set_color(173, 216, 230);
            }else {   
                g->set_color(58, 36, 59);
            }

            StreetSegsData street_seg = Streets_graphics_data->street_segs_by_tag
            [Streets_graphics_data->tag_num[Streets_graphics_data->all_street_segs[path[0]].road_type]]
            [Streets_graphics_data->all_street_segs[path[0]].index_in_street_segs_by_tag];
            

            // dont draw over highlighted streets
            if(!street_seg.highlight){
            
                // If no curve points in street segment
                if (street_seg.num_curve_points == 0) {
                    g->draw_line(street_seg.from_xy_loc, street_seg.to_xy_loc);
                }

                // If curve points in street segment
                else {
                    // Draw line from "from" to first curve point
                    g->draw_line(street_seg.from_xy_loc, street_seg.curve_points_xy_loc[0]);

                    // Draw lines for all curve points
                    for (int curve_id = 1; curve_id < street_seg.num_curve_points; ++curve_id) {
                        g->draw_line(street_seg.curve_points_xy_loc[curve_id - 1], street_seg.curve_points_xy_loc[curve_id]);
                    }

                    // Draw line from last curve point to "to"
                    g->draw_line(street_seg.curve_points_xy_loc[street_seg.num_curve_points - 1], street_seg.to_xy_loc);
                }
            }
            
            bool prev_quiz_mode = quiz_mode;
            int tag_num = 1;

            if(!quiz_mode) {
                quiz_mode = true;
            } else{
                tag_num = 0;
            }

            draw_streets_names(g, street_seg, dynamic_width_ratio, tag_num); 

            quiz_mode = prev_quiz_mode;
        }

       for (int ss_id = 1; ss_id < path.size(); ss_id++) {
            g->set_line_width(10);
            if (quiz_mode) {
                g->set_color(173, 216, 230);
            }else {   
                g->set_color(58, 36, 59);
            }

            StreetSegsData street_seg = Streets_graphics_data->street_segs_by_tag
            [Streets_graphics_data->tag_num[Streets_graphics_data->all_street_segs[path[ss_id]].road_type]]
            [Streets_graphics_data->all_street_segs[path[ss_id]].index_in_street_segs_by_tag];

            // If no curve points in street segment
            if (street_seg.num_curve_points == 0) {
                g->draw_line(street_seg.from_xy_loc, street_seg.to_xy_loc);
            }

            // If curve points in street segment
            else {

                // Draw line from "from" to first curve point
                g->draw_line(street_seg.from_xy_loc, street_seg.curve_points_xy_loc[0]);

                // Draw lines for all curve points
                for (int curve_id = 1; curve_id < street_seg.num_curve_points; ++curve_id) {
                    g->draw_line(street_seg.curve_points_xy_loc[curve_id - 1], street_seg.curve_points_xy_loc[curve_id]);
                }

                // Draw line from last curve point to "to"
                g->draw_line(street_seg.curve_points_xy_loc[street_seg.num_curve_points - 1], street_seg.to_xy_loc);
            }

            // Draw directional arrows
            if (zoom_ratio < 0.02) {
                
                // Calculate the coordinates for the arrow lines
                double angle = street_seg.angle;
                double arrow_thickness = 1 + dynamic_width_ratio;
                double arrow_angle = M_PI / 4; // 45 degrees in radians
                
                
                if(getStreetSegmentInfo(path[ss_id]).from == getStreetSegmentInfo(path[ss_id - 1]).from  || getStreetSegmentInfo(path[ss_id]).from  ==  getStreetSegmentInfo(path[ss_id - 1]).to ){
                    angle += M_PI;
                }

                ezgl::point2d arrow_start;
                ezgl::point2d arrow_end1;
                ezgl::point2d arrow_end2;

                // If no curve points, place the arrow in the middle of the street
                if (street_seg.num_curve_points == 0) {
                    arrow_start = ezgl::point2d((street_seg.from_xy_loc.x + street_seg.to_xy_loc.x) / 2,
                                                (street_seg.from_xy_loc.y + street_seg.to_xy_loc.y) / 2);
                } else {
                    arrow_start = street_seg.curve_points_xy_loc[0];
                }

                arrow_end1 = ezgl::point2d(arrow_start.x + arrow_length * cos(angle + arrow_angle), arrow_start.y + arrow_length * sin(angle + arrow_angle));
                arrow_end2 = ezgl::point2d(arrow_start.x + arrow_length * cos(angle - arrow_angle), arrow_start.y + arrow_length * sin(angle - arrow_angle));

                // Draw arrow lines
                g->set_line_width(arrow_thickness);
                g->draw_line(arrow_start, arrow_end1);
                g->draw_line(arrow_start, arrow_end2);
            
                bool prev_quiz_mode = quiz_mode;
                int tag_num = 1;

                if(!quiz_mode) quiz_mode = true;
                else{
                    tag_num = 0;
                }

                draw_streets_names(g, street_seg, dynamic_width_ratio, tag_num); 

                quiz_mode = prev_quiz_mode;
            }
        
        } 
    }


    if(quiz_mode){
        if(quiz_mode_path.size() > 0){
            g->set_line_width(10);
            
            if (quiz_mode) {
                g->set_color(173, 216, 230);
            }else {   
                g->set_color(58, 36, 59);
            }

            StreetSegsData street_seg = Streets_graphics_data->street_segs_by_tag
            [Streets_graphics_data->tag_num[Streets_graphics_data->all_street_segs[quiz_mode_path[0]].road_type]]
            [Streets_graphics_data->all_street_segs[quiz_mode_path[0]].index_in_street_segs_by_tag];

            // If no curve points in street segment
            if (street_seg.num_curve_points == 0) {
                g->draw_line(street_seg.from_xy_loc, street_seg.to_xy_loc);
            }

            // If curve points in street segment
            else {

                // Draw line from "from" to first curve point
                g->draw_line(street_seg.from_xy_loc, street_seg.curve_points_xy_loc[0]);

                // Draw lines for all curve points
                for (int curve_id = 1; curve_id < street_seg.num_curve_points; ++curve_id) {
                    g->draw_line(street_seg.curve_points_xy_loc[curve_id - 1], street_seg.curve_points_xy_loc[curve_id]);
                }

                // Draw line from last curve point to "to"
                g->draw_line(street_seg.curve_points_xy_loc[street_seg.num_curve_points - 1], street_seg.to_xy_loc);
            }
            
            bool prev_quiz_mode = quiz_mode;
            int tag_num = 1;

            if(!quiz_mode) {
                quiz_mode = true;
            } else{
                tag_num = 0;
            }

            draw_streets_names(g, street_seg, dynamic_width_ratio, tag_num); 

            quiz_mode = prev_quiz_mode;
        }

        for (int ss_id = 1; ss_id < quiz_mode_path.size(); ss_id++) {
            
            g->set_line_width(10);
            
            if (quiz_mode) {
                g->set_color(173, 216, 230);
            }else {   
                g->set_color(58, 36, 59);
            }

            StreetSegsData street_seg = Streets_graphics_data->street_segs_by_tag
            [Streets_graphics_data->tag_num[Streets_graphics_data->all_street_segs[quiz_mode_path[ss_id]].road_type]]
            [Streets_graphics_data->all_street_segs[quiz_mode_path[ss_id]].index_in_street_segs_by_tag];

            // If no curve points in street segment
            if (street_seg.num_curve_points == 0) {
                g->draw_line(street_seg.from_xy_loc, street_seg.to_xy_loc);
            }

            // If curve points in street segment
            else {

                // Draw line from "from" to first curve point
                g->draw_line(street_seg.from_xy_loc, street_seg.curve_points_xy_loc[0]);

                // Draw lines for all curve points
                for (int curve_id = 1; curve_id < street_seg.num_curve_points; ++curve_id) {
                    g->draw_line(street_seg.curve_points_xy_loc[curve_id - 1], street_seg.curve_points_xy_loc[curve_id]);
                }

                // Draw line from last curve point to "to"
                g->draw_line(street_seg.curve_points_xy_loc[street_seg.num_curve_points - 1], street_seg.to_xy_loc);
            }
        }
    }
    
}


// If a street is one way, draws one way arrows from the from to to direction
void draw_streets_one_way_arrows(ezgl::renderer *g, StreetSegsData street_seg, double dynamic_width_ratio){
    g->set_line_width(1*dynamic_width_ratio);
    
    if(quiz_mode){
        g->set_color(30,120,181); // Deep Blue
    }
    else{
        g->set_color(ezgl::WHITE);
    }

    // Calculate the coordinates for the arrow lines
    double angle = street_seg.angle + M_PI;
    double arrow_length = 1*dynamic_width_ratio; // Length of arrow lines
    double arrow_angle = M_PI / 4; // 45 degrees in radians

    ezgl::point2d arrow_start;
    ezgl::point2d arrow_end1;
    ezgl::point2d arrow_end2;

    if (street_seg.num_curve_points == 0) {
        // If no curve points, place the arrow in the middle of the street
        arrow_start = ezgl::point2d((street_seg.from_xy_loc.x + street_seg.to_xy_loc.x) / 2,
                                    (street_seg.from_xy_loc.y + street_seg.to_xy_loc.y) / 2);
    } else {
        arrow_start = street_seg.curve_points_xy_loc[0];
    }

    arrow_end1 = ezgl::point2d(arrow_start.x + arrow_length * cos(angle + arrow_angle), arrow_start.y + arrow_length * sin(angle + arrow_angle));
    arrow_end2 = ezgl::point2d(arrow_start.x + arrow_length * cos(angle - arrow_angle), arrow_start.y + arrow_length * sin(angle - arrow_angle));

    // Draw arrow lines
    g->draw_line(arrow_start, arrow_end1);
    g->draw_line(arrow_start, arrow_end2);

}

// Draws the name of the street on each street segment when the name is deemed to fit properly
void draw_streets_names(ezgl::renderer *g, StreetSegsData street_seg, double dynamic_width_ratio, int tag_num){

    // Calculate rotation of the text
    double rotation = 0;
    if(street_seg.to_xy_loc.x - street_seg.from_xy_loc.x != 0){
        rotation = 180 * street_seg.angle / M_PI; 

        // Adjust rotation to ensure text alignment
        if(rotation < 0) rotation += 360;

        // Flip the text 180 degrees for streets written from right to left or bottom to top
        if (rotation > 90 && rotation < 270) {
            rotation += 180; 
        }

        if(rotation > 360) rotation -= 360;
    
    }
    g->set_text_rotation(rotation);

    // Calculate length of segment
    double length = sqrt(pow((street_seg.to_xy_loc.x - street_seg.from_xy_loc.x ),2) + pow(street_seg.to_xy_loc.y - street_seg.from_xy_loc.y ,2));
    double bufferLength = 0.9 * length; 

    // Set to bold, OpenSans, size 10 font
    int lineWidth = 3*dynamic_width_ratio;
    ezgl::font_slant slant {CAIRO_FONT_SLANT_NORMAL}; 
    ezgl::font_weight weight {CAIRO_FONT_WEIGHT_BOLD};
    g->format_font("OpenSans",slant,weight);
    g->set_font_size(10);
    if(quiz_mode && tag_num!=0) g->set_color(ezgl::WHITE);
    else g->set_color(ezgl::BLACK);

    ezgl::point2d text_position = street_seg.center_xy_loc;
    g->draw_text(text_position, street_seg.name, bufferLength, lineWidth);

}

// Draw intersections as black dots
void draw_intersections(ezgl::renderer *g){
    double dynamic_width_ratio = 1;

    // Zoom level 7
    if (zoom_ratio >= 0.03) {
        dynamic_width_ratio = 1;
    }

    // Zoom level 8
    else if (zoom_ratio >= 0.02) {
        dynamic_width_ratio = 1.25;
    }

    // Zoom level 9
    else if (zoom_ratio >= 0.01) {
        dynamic_width_ratio = 1.5;
    }

    // Zoom level 10
    else if (zoom_ratio >= 0.005) {
        dynamic_width_ratio = 1.75;
    }

    // Zoom level 11
    else {
        dynamic_width_ratio = 2;
    }
    
    // Check if zoom level is sufficient
    if (zoom_ratio < 0.03) {
        
        // Set width and height in meters
        double radius = 2;
        double start_angle = 0;
        double extent_angle = 360;

        for (size_t inter_id = 0; inter_id < Intersections_graphics_data->intersections.size(); inter_id++) { 

            // Find location
            ezgl::point2d inter_loc = Intersections_graphics_data->intersections[inter_id].xy_loc;

            if (g->get_visible_world().contains(inter_loc.x, inter_loc.y)){
                
                // Determine whether intersection is highlighted or not
                if (Intersections_graphics_data->intersections[inter_id].highlight || Intersections_graphics_data->intersections[inter_id].clicked_quiz ) {
                    g->set_color(ezgl::RED);
                }
                else {
                    g->set_color(ezgl::BLACK);
                }
                
                g->fill_arc(inter_loc, radius, start_angle, extent_angle);
               
            }
        }
    }

    if(intersections_clicked > 0){
        ezgl::point2d inter_loc = Intersections_graphics_data->intersections[intersection_1].xy_loc;
        g->draw_surface(start_image, inter_loc, 0.5 * dynamic_width_ratio);
    }

    if(intersections_clicked > 1){
        ezgl::point2d inter_loc = Intersections_graphics_data->intersections[intersection_2].xy_loc;
        g->draw_surface(dest_image, {inter_loc.x, inter_loc.y + (20 - 4 * dynamic_width_ratio)}, 0.8 * dynamic_width_ratio);
    }
}

// Draws name of city
void draw_cityname(ezgl::renderer *g){

    g->set_text_rotation(0);

    // Check if the zoom level is high enough to display the city name
    if (zoom_ratio >= 0.5) {
        
        // Find the center of the city (you need to implement this function)
        double city_center_lat = (max_lat + min_lat) / 2.0;
        double city_center_lon = (max_lon + min_lon) / 2.0;

        // Set the font size and color for the city name
        g->set_font_size(40);
        if (quiz_mode) {
            g->set_color(223, 223, 223);
        }
        else {
            g->set_color(128, 128, 128);
        }
        
        // Set font to bold, OpenSans, size 30
        ezgl::font_slant slant {CAIRO_FONT_SLANT_NORMAL};
        ezgl::font_weight weight {CAIRO_FONT_WEIGHT_BOLD};
        g->set_text_rotation(0);
        g->format_font("OpenSans",slant,weight);
        g->set_font_size(30);

        // Calculate the position to draw the city name at the center of the city
        ezgl::point2d text_position(x_from_lon(city_center_lon), y_from_lat(city_center_lat));

        // Draw the city name on the canvas
        g->draw_text(text_position, map_name);
    }
}