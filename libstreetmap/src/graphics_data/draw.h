/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 15/03/2024
 *
 * Description: Contains functions declarations for calculating city information, 
 * such as average latitude and conversion between lat/lon and x/y and 
 * global variable delcarations for city information and pointers to 
 * graphics data structures.
*/


#include "city_information.h"
#include "graphics_globals.h"
#include "callbacks.h"

#ifndef DRAW_H
#define DRAW_H

//----------------------------------------- End Global Variables ------------------------------------------//
void draw_main_canvas(ezgl::renderer *g);
void draw_features(ezgl::renderer *g);
void draw_POIs(ezgl::renderer *g);
void draw_streets(ezgl::renderer *g);
void draw_streets_one_way_arrows(ezgl::renderer *g, StreetSegsData street_seg, double dynamic_width_ratio);
void draw_streets_directional_arrows(ezgl::renderer *g, StreetSegsData street_seg, double dynamic_width_ratio);
void draw_streets_names(ezgl::renderer *g, StreetSegsData street_seg, double dynamic_width_ratio, int tag_num);
void draw_intersections(ezgl::renderer *g);
void draw_cityname(ezgl::renderer *g);

#endif
