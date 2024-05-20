/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 25/03/2024
 *
 * Description: Contains helper functions for pathfinding required in M3.
 * The two functions are for performing BFS using Dijkstra's algorithm
 * and tracing back the shortest path found.
 */

#include "m3_helper.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "m1_globals.h"
#include <list>
#include <queue>

// Global vector to store node information during BFS
std::vector<Node> nodes;

// Does a BFs from source intersection to a destination
// finding shortest path using Dijkstra's algorithm.
bool bfsPath(const IntersectionIdx srcID, const IntersectionIdx destID, const double turn_penalty) {

    nodes.resize(getNumIntersections());

    // Priority queue to store wavefront intersections to search
    std::priority_queue<WaveElem, std::vector<WaveElem>, std::greater<WaveElem>> wavefront;
    wavefront.push(WaveElem(srcID, NO_EDGE, 0, 0)); 

    bool pathFound = false;

    // Perform BFS until wavefront is empty
    while (wavefront.size() > 0)
    {
        // Get the intersection at the front of the wavefront
        WaveElem curr = wavefront.top(); 
        wavefront.pop();                

        // Check if this is a better path to the current node
        if (curr.travelTime < nodes[curr.nodeID].bestTime && curr.travelTime < nodes[destID].bestTime)
        {
            // Update best time and reaching edge for the current node
            nodes[curr.nodeID].reachingEdge = curr.edgeID;
            nodes[curr.nodeID].bestTime = curr.travelTime;

            // Check if destination reached
            if (curr.nodeID == destID)
            {
                pathFound = true;
            }

            else
            {
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
                        // Add turn penalty if changing streets
                        travelTime = nodes[curr.nodeID].bestTime + findStreetSegmentTravelTime(out_edge) + turn_penalty;
                    }
                    else
                    {
                        travelTime = nodes[curr.nodeID].bestTime + findStreetSegmentTravelTime(out_edge);
                    }

                    if (travelTime < nodes[toNodeID].bestTime) {

                        // Calculate heuristic value for priority queue and add intersection to wavefront
                        double heuristic = findDistanceBetweenTwoPoints(get_intersection_position(toNodeID), get_intersection_position(destID)) / get_max_speed();
                        wavefront.push(WaveElem(toNodeID, out_edge, travelTime, heuristic)); 
                    }
                }
            }
        }
    }

    return pathFound;
}



// Traces back from the destination intersection to the starting
// intersection using reachingEdge information for each node.
std::vector<StreetSegmentIdx> bfsTraceBack(IntersectionIdx destID)
{
    std::list<StreetSegmentIdx> path;

    IntersectionIdx currNodeID = destID;

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

    nodes.clear();

    return {std::begin(path), std::end(path)};
}
