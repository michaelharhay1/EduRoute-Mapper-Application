/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 03/04/2024
 *
 * Description: 
 */

#include "m4_helper.h"

std::vector<IntersectionIdx> remove_duplicate_intersections(const std::vector<DeliveryInf>& deliveries,
                                                            const std::vector<IntersectionIdx>& depots) 
{
    // fill vector with all intersections
    std::vector<IntersectionIdx> all_intersections = depots;

    for (const DeliveryInf& delivery : deliveries) {
        all_intersections.push_back(delivery.pickUp);
        all_intersections.push_back(delivery.dropOff);
    }

    // get rid of duplicates
    all_intersections.erase(std::unique(all_intersections.begin(), all_intersections.end()), all_intersections.end());
    return all_intersections; // from least to greatest
}

std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>>> fillPathMatrix(
                                                                        const std::vector<IntersectionIdx>& interesting_intersections, // All unique intersections
                                                                        const double turn_penalty) // Turn penalty for bfs
{
    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>>> all_interesting_paths;

    // Populate all_interesting_paths using threading
    #pragma omp parallel for
    for(int intersection_idx = 0; intersection_idx < interesting_intersections.size(); intersection_idx++){
        
        IntersectionIdx inter_id = interesting_intersections[intersection_idx];
        std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>> interesting_path_map = findAllPathsBetweenIntersections(inter_id, interesting_intersections, turn_penalty);

        #pragma omp critical
        {
            all_interesting_paths[inter_id] = interesting_path_map;
        }
    }

    return all_interesting_paths;
}

std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>> findAllPathsBetweenIntersections(
                                                        const IntersectionIdx srcID,
                                                        const std::vector<IntersectionIdx> destIDs, // Make a map?
                                                        const double turn_penalty)
{
    std::vector<Node> nodes(getNumIntersections());    

    // Priority queue to store wavefront intersections to search
    std::priority_queue<WaveElem, std::vector<WaveElem>, std::greater<WaveElem>> wavefront;
    wavefront.push(WaveElem(srcID, NO_EDGE, 0, 0)); 

    // Perform BFS until wavefront is empty
    while (wavefront.size() > 0)
    {
        // Get the intersection at the front of the wavefront
        WaveElem curr = wavefront.top(); 
        wavefront.pop();                

        // Check if this is a better path to the current node
        if (curr.travelTime < nodes[curr.nodeID].bestTime)
        {
            // Update best time and reaching edge for the current node
            nodes[curr.nodeID].reachingEdge = curr.edgeID;
            nodes[curr.nodeID].bestTime = curr.travelTime;
            nodes[curr.nodeID].found = true;

            // Get outgoing segments of the current intersection
            nodes[curr.nodeID].out_edges = findStreetSegmentsOfIntersection(curr.nodeID);

            // Explore each outgoing segment
            for (StreetSegmentIdx out_edge : nodes[curr.nodeID].out_edges)
            {
                int toNodeID = 0;

                // Determine ID of intersection 9accounting for one-way)
                if (curr.nodeID == get_street_seg_from(out_edge))
                {
                    toNodeID = get_street_seg_to(out_edge);
                }
                else if (!get_street_seg_one_way(out_edge) && (curr.nodeID == get_street_seg_to(out_edge)))
                {
                    toNodeID = get_street_seg_from(out_edge);
                }
                else
                {
                    continue;
                }

                // Calculate travel time to the reached intersection
                double travelTime;
                if (nodes[curr.nodeID].reachingEdge != NO_EDGE && get_street_seg_street_id(nodes[curr.nodeID].reachingEdge) != get_street_seg_street_id(out_edge))
                {
                    travelTime = nodes[curr.nodeID].bestTime + findStreetSegmentTravelTime(out_edge) + turn_penalty;
                }
                else
                {
                    travelTime = nodes[curr.nodeID].bestTime + findStreetSegmentTravelTime(out_edge);
                }

                // Only look at this node if there's a better travel time
                if (travelTime < nodes[toNodeID].bestTime) {
                    wavefront.push(WaveElem(toNodeID, out_edge, travelTime, 0)); 
                }
            }
        }
    }

    std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>> all_interesting_paths;

    #pragma omp parallel for
    for(int dest_idx = 0; dest_idx < destIDs.size(); dest_idx++){
        
        std::list<StreetSegmentIdx> path;
        IntersectionIdx currNodeID = destIDs[dest_idx];
        StreetSegmentIdx prevEdge = nodes[currNodeID].reachingEdge;

        // Trace back until reaching the starting intersection
        while (prevEdge != NO_EDGE)
        {
            // Add the previous edge to front of path list
            path.push_front(prevEdge);

            // Update the current intersection
            if (currNodeID == get_street_seg_from(prevEdge))
            {
                currNodeID = get_street_seg_to(prevEdge);
            }
            else
            {
                currNodeID = get_street_seg_from(prevEdge);
            }

            // Update prevEdge
            prevEdge = nodes[currNodeID].reachingEdge;
        }

        if(currNodeID != srcID){
            all_interesting_paths[destIDs[dest_idx]] = {std::begin(path), std::end(path)};
        } else {
            all_interesting_paths[destIDs[dest_idx]] = {std::begin(path), std::end(path)};
        }
    }

    nodes.clear();
    return all_interesting_paths;   
}



std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, double>>  fillPathCostMatrix(
                        std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>>> all_paths, 
                        const double turn_penalty){

    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, double>> all_path_costs;

    // Go through every intersting intersection
    #pragma omp parallel for
    for (int src_idx = 0; src_idx < all_paths.size(); src_idx++) {

        auto src_it = std::next(all_paths.begin(), src_idx);
        IntersectionIdx src_id = src_it->first;

        // Go through every intersection connect to the source intersection
        for (auto dst_intersection : src_it->second){
            
            IntersectionIdx dst_id = dst_intersection.first;
            double travel_time = computePathTravelTime(turn_penalty, dst_intersection.second);

            #pragma omp critical
            {
                all_path_costs[src_id][dst_id] = travel_time;
            }
        }  
    }

    return all_path_costs;
}
std::tuple<std::vector<PickDrop>, int> choosePerturbation(const std::vector<PickDrop>& solution, double temperature) {
    // Define perturbation functions
    std::vector<std::vector<PickDrop> (*)(std::vector<PickDrop>, double)> perturbationFunctions = {
        shift,          // Minor perturbation
        swap,           // Moderate perturbation
        reverseSubsequence  // Significant perturbation
    };

    // Initial probabilities at temperature = 1
    double random_value = rand() % 100;

    if(random_value < 5 && temperature > 50)
        return {perturbationFunctions[1](solution, temperature), 1};

    if(random_value < 25 && temperature > 20)
        return {perturbationFunctions[2](solution, temperature), 2};  


    // Fallback to shift if no other perturbation is selected
    return {perturbationFunctions[0](solution, temperature), 0};
}


// Simulated Annealing function
PathOptions simulated_annealing(std::vector<PickDrop> initial_solution, double initial_cost, 
                                std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>>> path_matrix,
                                std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, double>> path_cost_matrix,
                                const std::vector<IntersectionIdx>& depots,
                                std::chrono::time_point<std::chrono::high_resolution_clock> start_time) {
    std::vector<int> perturbationSuccessCount = {0, 0, 0};  // Corresponds to shift, swap, reverseSubsequence

    std::vector<PickDrop> solution = initial_solution;
    double cost = initial_cost;
    double best_cost = initial_cost;
    std::vector<PickDrop> best_solution = initial_solution;

    double temperature = 100; // High initial temperature
    int iterations_since_improvement = 0;

    while (true) {
        auto [new_solution, chosen_perturbation_index] = choosePerturbation(solution, temperature);
        double new_cost = solution_cost(new_solution, path_cost_matrix, depots);
        double cost_difference = new_cost - cost;

        if(legal_move(new_solution)){

            if (cost_difference < 0 || exp(-cost_difference / temperature) > static_cast<double>(rand()) / RAND_MAX) {
                solution = new_solution;
                cost = new_cost;

                if (cost < best_cost) {
                    best_cost = cost;
                    best_solution = solution;
                    iterations_since_improvement = 0;
                } else {
                    iterations_since_improvement++;
                }

            } else {
                iterations_since_improvement++;
            }

            if (iterations_since_improvement > 100) {
                temperature *= 0.9;
                iterations_since_improvement = 0;
                solution = best_solution;
            } else {
                temperature *= 0.95;
            }
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        auto wallClock = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - start_time);
        if (wallClock.count() > 0.9 * TIME_LIMIT) break;
    }

    return PathOptions(PDDToCSP(best_solution, path_matrix, path_cost_matrix, depots), best_solution, best_cost);
}

std::vector<PickDrop> legalize(std::vector<PickDrop> current_solution) {
    std::unordered_map<IntersectionIdx, size_t> seenDropoffs; // Map to store the index of seen drop-offs
    bool legal = legal_move(current_solution);
    
    if (legal) { 
        return current_solution;
    }
    else {
        // Iterate through the solution
        for (size_t i = 0; i < current_solution.size(); ++i) {
            // If it's a drop-off, store its index
            if (current_solution[i].isPickUp == 1 || current_solution[i].isPickUp == 2) {
                seenDropoffs[current_solution[i].intersection_id] = i;
            } 
            // If it's a pick-up, check if any of its drop-offs are out of order
            else {
                for (const auto& dropoff : current_solution[i].drop_offs) {
                    auto it = seenDropoffs.find(dropoff);
                    if (it != seenDropoffs.end()) {
                        auto temp = current_solution[it->second];
                        // Erase the drop-off from its previous position
                        current_solution.erase(current_solution.begin() + it->second);
                        // Insert the drop-off into the index right after the pick-up
                        current_solution.insert(current_solution.begin() + i + 1, temp);
                        // Update the index of the moved drop-off
                        seenDropoffs[dropoff] = i + 1;
                    }
                }
            }
        }
        return current_solution;
    }
}

// Check if solution is legal
bool legal_move(std::vector<PickDrop> solution) {
    std::unordered_set<IntersectionIdx> seen_drop_off;

    // Iterate through solution
    for (auto pick_drop = solution.rbegin(); pick_drop != solution.rend(); pick_drop++){
        
        // If intersection is a drop off or both a drop off and pickup
        if(pick_drop->isPickUp == 1 || pick_drop->isPickUp == 2){
            seen_drop_off.insert(pick_drop->intersection_id);
        }   

        // Make sure the drop off happens after a pick up - search through all the drop offs associated with the pick up and make sure it happesn after
        if(pick_drop->isPickUp == 0 || pick_drop->isPickUp == 2){
            for(int i = 0; i < pick_drop->drop_offs.size(); i++){
                if(seen_drop_off.find(pick_drop->drop_offs[i]) == seen_drop_off.end()){
                    return false;
                }
            }
        }
    }

    return true;
}

// ------------------------- PERTUBATION FUNCTION -------------------------
std::vector<PickDrop> shift(std::vector<PickDrop> current_solution, double temperature) {
    if (current_solution.size() < 2) {
        return current_solution; // No shift possible if there's 0 or 1 item
    }

    int bound = temperature/10;

    if(bound < 1){
        bound = 1;
    }

    std::vector<PickDrop> solution = current_solution;
    int old_index = rand() % solution.size();

    // Calculate the valid bounds for new_index
    int lower_bound = std::max(0, (old_index - 10));
    int upper_bound = std::min(static_cast<int>(solution.size()) - 1, old_index + 10);

    // Generate new_index within the bounds
    int range = upper_bound - lower_bound + 1; // +1 because upper_bound is inclusive
    int new_index = lower_bound + (rand() % range);

    // Perform the shift
    PickDrop temp = solution[old_index];
    solution.erase(solution.begin() + old_index);

    // Adjust new_index for the change in vector size after erase if necessary
    if (new_index > old_index) {
        new_index--; // Decrease by 1 because the original list size has reduced by 1
    }

    solution.insert(solution.begin() + new_index, temp);

    return solution;
}

std::vector<PickDrop> swap(std::vector<PickDrop> current_solution, double temperature) {
    int n = current_solution.size();
    if (n < 2) {
        return current_solution; // No swap possible if there's less than two items
    }

    int index1 = rand() % n;
    int max_distance = std::max(1, static_cast<int>(n * temperature / 10000)); // Dynamic distance based on temperature

    int index2 = (index1 + (rand() % (2 * max_distance + 1) - max_distance) + n) % n; // Wrap around using modulo

    if (index1 != index2) {
        std::swap(current_solution[index1], current_solution[index2]);
    }

    return current_solution;
}

std::vector<PickDrop> reverseSubsequence(std::vector<PickDrop> current_solution, double temperature) {
    int n = current_solution.size();
    if (n < 2) {
        return current_solution; // No reverse possible if there's less than two items
    }

    int start = rand() % n;
    int length = std::max(2, static_cast<int>(n /** temperature / 10000*/)); // Subsequence length depends on temperature
    int end = std::min(start + length - 1, n - 1);

    std::reverse(current_solution.begin() + start, current_solution.begin() + end + 1);

    return current_solution;
}




// Solution cost calculation function - TODO
double solution_cost(std::vector<PickDrop> solution,
                    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, double>>  path_cost_matrix,
                    const std::vector<IntersectionIdx>& depots) {

    // Find the depot closest to any of the delivery pick-up locations
    std::priority_queue<DepotOption> depot_options;
    for (auto depot : depots) {
        double distance = (path_cost_matrix[depot][solution[0].intersection_id] + path_cost_matrix[solution[solution.size() - 1].intersection_id][depot])/2;
        depot_options.push(DepotOption(distance, depot));
    }

    DepotOption best_depot = depot_options.top();
    IntersectionIdx nearest_depot = best_depot.depot_id;

    double cost = path_cost_matrix[nearest_depot][solution[0].intersection_id];

    // Go thorugh the rest of the solution
    for(int index = 0 ; index < solution.size() - 1; index++){
        cost += path_cost_matrix[solution[index].intersection_id][solution[index + 1].intersection_id];
    }

    cost += path_cost_matrix[solution[solution.size() - 1].intersection_id][nearest_depot];

    return cost;
}

// Conversion function PDD to CSP - TODO
std::vector<CourierSubPath> PDDToCSP(std::vector<PickDrop> solution, 
                                    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>>> path_matrix,
                                    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, double>>  path_cost_matrix,
                                    const std::vector<IntersectionIdx>& depots) {

    std::vector<CourierSubPath> converted_solution;
    CourierSubPath cur_sub_path;

    // Find the depot closest to any of the delivery pick-up locations
    std::priority_queue<DepotOption> depot_options;
    for (auto depot : depots) {
        double distance = (path_cost_matrix[depot][solution[0].intersection_id] + path_cost_matrix[solution[solution.size() - 1].intersection_id][depot])/2;
        depot_options.push(DepotOption(distance, depot));
    }

    DepotOption best_depot = depot_options.top();
    IntersectionIdx nearest_depot = best_depot.depot_id;
    
    cur_sub_path.intersections = std::make_pair(nearest_depot, solution[0].intersection_id);
    cur_sub_path.subpath = path_matrix[cur_sub_path.intersections.first][cur_sub_path.intersections.second];
    converted_solution.push_back(cur_sub_path);

    // Go thorugh the rest of the solution
    for(int index = 0 ; index < solution.size() - 1; index++){
        IntersectionIdx current_intersection = solution[index].intersection_id;
        IntersectionIdx next_intersection = solution[index + 1].intersection_id;
        cur_sub_path.intersections = std::make_pair(current_intersection, next_intersection);
        cur_sub_path.subpath = path_matrix[cur_sub_path.intersections.first][cur_sub_path.intersections.second];
        converted_solution.push_back(cur_sub_path);
    }

    // Last intersection back to depot
    cur_sub_path.intersections = std::make_pair(solution[solution.size() - 1].intersection_id, nearest_depot);
    cur_sub_path.subpath = path_matrix[cur_sub_path.intersections.first][cur_sub_path.intersections.second];
    converted_solution.push_back(cur_sub_path);

    return converted_solution;
}
