/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 03/04/2024
 *
 * Description: 
 */

#include "m4_helper.h"



// This routine takes in a vector of D deliveries (pickUp, dropOff
// intersection pairs), another vector of N intersections that
// are legal start and end points for the path (depots), and a turn 
// penalty in seconds (see m3.h for details on turn penalties).
//
// The first vector 'deliveries' gives the delivery information.  Each delivery
// in this vector has pickUp and dropOff intersection ids.
// A delivery can only be dropped-off after the associated item has been picked-up. 
// 
// The second vector 'depots' gives the intersection ids of courier company
// depots containing trucks; you start at any one of these depots but you must
// end at the same depot you started at.
//
// This routine returns a vector of CourierSubPath objects that form a delivery route.
// The CourierSubPath is as defined above. The first street segment id in the
// first subpath is connected to a depot intersection, and the last street
// segment id of the last subpath also connects to a depot intersection.
// A package will not be dropped off if you haven't picked it up yet.
//
// The start_intersection of each subpath in the returned vector should be 
// at least one of the following (a pick-up and/or drop-off can only happen at 
// the start_intersection of a CourierSubPath object):
//      1- A start depot.
//      2- A pick-up location
//      3- A drop-off location. 
//
// You can assume that D is always at least one and N is always at least one
// (i.e. both input vectors are non-empty).
//
// It is legal for the same intersection to appear multiple times in the pickUp
// or dropOff list (e.g. you might have two deliveries with a pickUp
// intersection id of #50). The same intersection can also appear as both a
// pickUp location and a dropOff location.
//        
// If you have two pickUps to make at an intersection, traversing the
// intersection once is sufficient to pick up both packages. Additionally, 
// one traversal of an intersection is sufficient to drop off all the 
// (already picked up) packages that need to be dropped off at that intersection.
//
// Depots will never appear as pickUp or dropOff locations for deliveries.
//  
// If no valid route to make *all* the deliveries exists, this routine must
// return an empty (size == 0) vector.

std::vector<CourierSubPath> travelingCourier(
                            const float turn_penalty,
                            const std::vector<DeliveryInf>& deliveries,
                            const std::vector<IntersectionIdx>& depots){

    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<IntersectionIdx> interesting_intersections = remove_duplicate_intersections(deliveries, depots);
    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>>> path_matrix = fillPathMatrix(interesting_intersections, turn_penalty);
    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, double>>  path_cost_matrix = fillPathCostMatrix(path_matrix, turn_penalty);
    std::priority_queue<PathOptions> path_options;


    // For every dropoff, store the pick ups that need to happen; for every pick up, store the drop offs that happen after it
    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, bool>> dropOffDependencies;
    std::unordered_map<IntersectionIdx, std::unordered_set<IntersectionIdx>> pickUpDependencies;
    std::vector<IntersectionIdx> r_drop_offs;

    for (const auto& delivery : deliveries) {
        if(dropOffDependencies.find(delivery.dropOff) == dropOffDependencies.end()){
            r_drop_offs.push_back(delivery.dropOff);
        }
        dropOffDependencies[delivery.dropOff][delivery.pickUp] = false; // map every drop_off to its pick_up that need to happen before it
        pickUpDependencies[delivery.pickUp].insert(delivery.dropOff); // map every delivery that needs to be made for a pickup

        for(const auto& delivery_2 : deliveries) {
            if(delivery.dropOff != delivery_2.dropOff){
                if((path_matrix[delivery.dropOff][delivery_2.dropOff]).empty()){
                    return {};
                }
            }

            if(delivery.dropOff != delivery_2.pickUp){  
                if((path_matrix[delivery.dropOff][delivery_2.pickUp]).empty()){
                    return {};
                }
            }

            if(delivery.pickUp != delivery_2.dropOff){
                if((path_matrix[delivery.pickUp][delivery_2.dropOff]).empty()){
                    return {};
                }
            }

            if(delivery.pickUp != delivery_2.pickUp){    
                if((path_matrix[delivery.pickUp][delivery_2.pickUp]).empty()){
                    return {};
                }
            }
           
        }
    }

    // std::vector <PathOptions> paths(2000);

    // #pragma omp parallel for
    for(int i = 0; i < 2000; i++){
        std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, bool>> drop_off_dependencies = dropOffDependencies;
        std::unordered_map<IntersectionIdx, std::unordered_set<IntersectionIdx>> pick_up_dependencies = pickUpDependencies;

        std::vector<CourierSubPath> all_sub_paths;
        std::vector<PickDrop> converted_solution;

        CourierSubPath cur_sub_path;
        int random;

        double travel_time = 0;
        //srand(i);

        IntersectionIdx nearest_depot;

        // Find the depot closest to any of the delivery pick-up locations
        std::priority_queue<DepotOption> depot_options;
        for (auto depot : depots) {
            for (auto delivery : deliveries) {
                double distance = path_cost_matrix[depot][delivery.pickUp];
                depot_options.push(DepotOption(distance, depot));
            }
        }

        random = rand() % 100;
        if(random < 3 && depot_options.size() >= 2){
            depot_options.pop();
            DepotOption second_best_depot = depot_options.top();
            nearest_depot = second_best_depot.depot_id;
        }
        else {
            DepotOption second_best_depot = depot_options.top();
            nearest_depot = second_best_depot.depot_id;
        }


        // Create a copy of the deliveries vector
        std::vector<DeliveryInf> remaining_deliveries = deliveries;
        std::vector<IntersectionIdx> remaining_drop_offs = r_drop_offs;

        // Keep track of visited pick-up locations
        std::unordered_set<IntersectionIdx> visited_pickups;

        // Iterate through deliveries
        IntersectionIdx current_intersection = nearest_depot;
        while (!remaining_drop_offs.empty()) {
            // print_dropOffDependencies(dropOffDependencies);
            // print_pickUpDependencies(pickUpDependencies);
            IntersectionIdx pickUpIntersection = -1;
            IntersectionIdx dropOffIntersection = -1; // might cause trouble

            // Find the nearest legal pick-up or drop-off location
            std::priority_queue<DeliveryOption> delivery_options;

            for (auto& delivery : remaining_deliveries) {
                // If a pickup intersection hasn't been visited
                if (visited_pickups.count(delivery.pickUp) == 0 ) {
                    // std::cout << "Pickup hasn't been visited: " << delivery.pickUp << std::endl;
                    double pickup_distance = path_cost_matrix[current_intersection][delivery.pickUp];
                    delivery_options.push(DeliveryOption(pickup_distance, delivery, true));
                }

                // Check if all pickups required for this drop-off have been completed
                bool allPickupsDone = true;
                // std::cout << "  Checking for allPickupsDone for drop-off " << delivery.dropOff << ":" << std::endl;
                if (drop_off_dependencies.find(delivery.dropOff) != drop_off_dependencies.end()) {
                    for (const auto& pickup : drop_off_dependencies[delivery.dropOff]) {
                        // std::cout << "Pickup: " << pickup.first << ", Completed: " << std::boolalpha << pickup.second << std::endl;
                        if (!pickup.second) { // If any pickup is not completed
                            allPickupsDone = false;
                            break;
                        }
                    }
                } 
                else {
                    allPickupsDone = false;
                }  

                // You can go to the drop-off only if all the pickups have been done
                if (allPickupsDone) {
                    double dropoff_distance = path_cost_matrix[current_intersection][delivery.dropOff];
                    delivery_options.push(DeliveryOption(dropoff_distance, delivery, false));
                }
            }

            DeliveryInf real_delivery(pickUpIntersection, dropOffIntersection);
            bool pickup;
            
            random = rand() % 100;
            if(random < 3 && delivery_options.size() >= 2){
                delivery_options.pop();
                DeliveryOption second_best_delivery =  delivery_options.top();          
                real_delivery = second_best_delivery.delivery;
                pickup = second_best_delivery.isPickup;
            }

            else {
                DeliveryOption second_best_delivery =  delivery_options.top();          
                real_delivery = second_best_delivery.delivery;
                pickup = second_best_delivery.isPickup;
            }
            
            PickDrop temp;

            // Add the path to the nearest location to the solution
            if (pickup) {
                visited_pickups.insert(real_delivery.pickUp);
                cur_sub_path.intersections = std::make_pair(current_intersection, real_delivery.pickUp);
                current_intersection = real_delivery.pickUp;

                temp.intersection_id = real_delivery.pickUp;
                temp.isPickUp = 0;

                // go through every delivery associated with this pickup and mark the pick up as complete
                if(pick_up_dependencies.find(current_intersection) != pick_up_dependencies.end()) {
                    for(auto delivery: pick_up_dependencies[current_intersection]){
                        /*
                        if(drop_off_dependencies.find(delivery) != drop_off_dependencies.end() && 
                            drop_off_dependencies[delivery].find(current_intersection) != drop_off_dependencies[delivery].end()){*/
                            drop_off_dependencies[delivery][current_intersection] = true;  
                        // }
                        temp.drop_offs.push_back(delivery);
                    }
                }
            } 
            
            else {
                visited_pickups.insert(real_delivery.dropOff); // same intersection can be pickUp and dropOff
                cur_sub_path.intersections = std::make_pair(current_intersection, real_delivery.dropOff);
                current_intersection = real_delivery.dropOff;

                temp.intersection_id = real_delivery.dropOff;
                temp.isPickUp = 1;

                // go through every delivery associated with this pickup and mark the pick up as complete
                if(pick_up_dependencies.find(current_intersection) != pick_up_dependencies.end()) {
                    for(auto delivery: pick_up_dependencies[current_intersection]){
                        /*if(drop_off_dependencies.find(delivery) != drop_off_dependencies.end() && 
                            drop_off_dependencies[delivery].find(current_intersection) != drop_off_dependencies[delivery].end()){ */
                            drop_off_dependencies[delivery][current_intersection] = true;
                        //}

                        // This delivery is also a pick up
                        temp.isPickUp = 2;
                        temp.drop_offs.push_back(delivery);
                    }
                }

                remaining_deliveries.erase(
                    std::remove_if(
                        remaining_deliveries.begin(), 
                        remaining_deliveries.end(),
                        [current_intersection](const DeliveryInf& d) { return d.dropOff == current_intersection; }
                    ),
                    remaining_deliveries.end()
                );

                remaining_drop_offs.erase(
                    std::remove_if(
                        remaining_drop_offs.begin(), 
                        remaining_drop_offs.end(),
                        [current_intersection](const IntersectionIdx& idx) { return idx == current_intersection; }
                    ),
                    remaining_drop_offs.end()
                ); 

            }

            converted_solution.push_back(temp);
            cur_sub_path.subpath = path_matrix[cur_sub_path.intersections.first][cur_sub_path.intersections.second];
            all_sub_paths.push_back(cur_sub_path);
            travel_time += path_cost_matrix[cur_sub_path.intersections.first][cur_sub_path.intersections.second];
        }

        // check if remaining intersections empty too

        // Go back to the starting depot
        cur_sub_path.intersections = std::make_pair(current_intersection, nearest_depot);
        cur_sub_path.subpath = path_matrix[cur_sub_path.intersections.first][cur_sub_path.intersections.second];
        all_sub_paths.push_back(cur_sub_path);
        travel_time += path_cost_matrix[cur_sub_path.intersections.first][cur_sub_path.intersections.second];
        PathOptions new_path(all_sub_paths, converted_solution, travel_time);
        path_options.push(new_path);
        // paths[i] = new_path;
    }

    /*
    for(int i = 0; i < 2000; i++){
        path_options.push(paths[i]);
    }
    */


    std::vector <PathOptions> multi_start_paths;
    for(int i = 0; i < 4 && !path_options.empty(); i++){
        PathOptions temp = path_options.top();
        path_options.pop();
        std::cout << "ORIGINAL TRAVEL TIME: " << temp.travel_time << std::endl;
        multi_start_paths.push_back(temp);
    }

    #pragma omp parallel for
    for(int i = 0; i < multi_start_paths.size(); i++){
        multi_start_paths[i] = simulated_annealing(multi_start_paths[i].converted_path, multi_start_paths[i].travel_time, path_matrix, path_cost_matrix, depots, start_time);
    }

    // Finding the PathOptions with the smallest travel time
    double min_travel_time = std::numeric_limits<double>::max();
    std::vector<CourierSubPath> best_path_option;

    for (auto& path_option : multi_start_paths) {
        if (path_option.travel_time < min_travel_time) {
            min_travel_time = path_option.travel_time;
            best_path_option = path_option.path;
        }
    }

    return best_path_option;
}

