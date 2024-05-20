/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 03/04/2024
 *
 * Description: 
 */

#include "m1.h"
#include "m1_globals.h"
#include "m3.h"
#include "m3_helper.h"
#include "m4.h"
#include <unordered_set>
#include <map>
#include <queue>
#include <list>
#include <vector>
#include <cmath>
#include <chrono>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <tuple> 

#define TIME_LIMIT 50


struct DepotOption {
    double distance;
    IntersectionIdx depot_id;

    // Constructor for ease of use
    DepotOption(double dist, IntersectionIdx idx)
        : distance(dist), depot_id(idx) {}

    // Comparator for sorting (if using priority queue)
    bool operator<(const DepotOption& other) const {
        return distance > other.distance; // Min heap
    }
};

struct DeliveryOption {
    double distance;
    DeliveryInf delivery;
    bool isPickup; // true for pickup, false for dropoff

    // Constructor for ease of use
    DeliveryOption(double dist, DeliveryInf deliv, bool pickup)
        : distance(dist), delivery(deliv), isPickup(pickup) {}

    // Comparator for sorting (if using priority queue)
    bool operator<(const DeliveryOption& other) const {
        return distance > other.distance; // Min heap
    }
};

// Struct to store solution for easy peturbation
struct PickDrop {
    
    // 0 = pickup, 1 = dropoff, 2 = both
    int isPickUp;
    IntersectionIdx intersection_id;
    std::vector<IntersectionIdx> pick_ups;
    std::vector<IntersectionIdx> drop_offs;
};


struct PathOptions {
    std::vector<CourierSubPath> path;
    std::vector<PickDrop> converted_path;
    double travel_time;

    PathOptions(){
    }

    // Constructor for ease of use
    PathOptions(std::vector<CourierSubPath> p, std::vector<PickDrop> c, double t)
        : path(p), converted_path(c), travel_time(t) {}

    // Comparator for sorting (if using priority queue)
    bool operator<(const PathOptions& other) const {
        return travel_time > other.travel_time; // Min heap
    }
};

std::vector<IntersectionIdx> remove_duplicate_intersections(const std::vector<DeliveryInf>& deliveries,
                                                            const std::vector<IntersectionIdx>& depots);


                                            
std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>>> fillPathMatrix(
                                                                        const std::vector<IntersectionIdx>& interesting_intersections, // All unique intersections
                                                                        const double turn_penalty); // Turn penalty for bfs
 

std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>> findAllPathsBetweenIntersections(
                                                        const IntersectionIdx srcID,
                                                        const std::vector<IntersectionIdx> destIDs, 
                                                        const double turn_penalty);


std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, double>>  fillPathCostMatrix(
                        std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>>> all_paths, 
                        const double turn_penalty);


// Simulated Annealing functions
PathOptions simulated_annealing(std::vector<PickDrop> initial_solution, double initial_cost, 
                                                std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>>> path_matrix,
                                                std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, double>>  path_cost_matrix,
                                                const std::vector<IntersectionIdx>& depots,
                                                std::chrono::time_point<std::chrono::high_resolution_clock> start_time);



bool legal_move(std::vector<PickDrop> solution);
std::vector<PickDrop> swap(std::vector<PickDrop> current_solution, double temperature); 
std::vector<PickDrop> shift(std::vector<PickDrop> current_solution, double temperature);
std::vector<PickDrop> reverseSubsequence(std::vector<PickDrop> current_solution, double temperature) ;

double solution_cost(std::vector<PickDrop> solution,
                    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, double>>  path_cost_matrix,
                    const std::vector<IntersectionIdx>& depots);

std::vector<CourierSubPath> PDDToCSP(std::vector<PickDrop> solution, 
                                    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, std::vector<StreetSegmentIdx>>> path_matrix,
                                    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, double>>  path_cost_matrix,
                                    const std::vector<IntersectionIdx>& depots);

std::tuple<std::vector<PickDrop>, int> choosePerturbation(const std::vector<PickDrop>& solution, double temperature);
