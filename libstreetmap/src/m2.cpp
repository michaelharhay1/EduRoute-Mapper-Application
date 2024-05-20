/*  
 * Copyright 2024 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "m1.h"
#include "m2.h"
#include "graphics_data/draw.h"

//--------------------------------------------- Main function ---------------------------------------------//

void drawMap() {
    // Congfigure EZGL application settings
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";

    // Create new EZGL application with determined settings
    ezgl::application application(settings);

    // Initialize graphics data classes   
    calculateLatAvg();


    Intersections_graphics_data = new IntersectionsGraphicsData();
    Streets_graphics_data = new StreetsGraphicsData();
    POIs_graphics_data = new POIGraphicsData();
    Features_graphics_data = new FeaturesGraphicsData();

    // Create canvas
    ezgl::rectangle initial_world({x_from_lon(min_lon), y_from_lat(min_lat)},
                                  {x_from_lon(max_lon), y_from_lat(max_lat)});

    world_width = x_from_lon(max_lon) - x_from_lon(min_lon);
    world_height = y_from_lat(max_lat) - y_from_lat(min_lat);

    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);

    // Run the application
    application.run(initial_setup, act_on_mouse_click, act_on_mouse_move, nullptr);
}

//------------------------------------------- End Main function -------------------------------------------//
