#ifndef M1_GLOBALS_H
#define M1_GLOBALS_H

#include "StreetsDatabaseAPI.h"

float get_max_speed();
int get_street_seg_street_id(int street_seg_id);
int get_street_seg_from(int street_seg_id);
int get_street_seg_to(int street_seg_id);
bool get_street_seg_one_way(int street_seg_id);
LatLon get_intersection_position(int intersection_id);


#endif