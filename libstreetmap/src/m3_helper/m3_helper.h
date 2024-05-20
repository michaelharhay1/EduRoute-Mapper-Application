/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 25/03/2024
 *
 * Description: Contains struct definitions related to pathfinding
 * and provides function declarations for bfsPath and bfsTraceBack.
 */

#include <vector>
#include <float.h>

#include "StreetsDatabaseAPI.h"

// Illegal edge ID -> no edge 
#define NO_EDGE -1  

// Structure representing an element in the wavefront during BFS 
struct WaveElem {

   // ID of the node
   IntersectionIdx nodeID; 

   // Edge used to reach this node         
   StreetSegmentIdx edgeID;

   // Total travel time to reach this node         
   double travelTime;  

   // Heuristic estimation for A* search             
   double heuristic;                

   // Constructor
   WaveElem (IntersectionIdx n, StreetSegmentIdx e, double time, double estimation) {
      nodeID = n; 
      edgeID = e; 
      travelTime = time; 
      heuristic = estimation;
   }

   // Overloading comparison operator to compare WaveElem objects by the sum of their travel times and heuristic value
   bool operator>(const WaveElem& other) const {
      return (travelTime + heuristic) > (other.travelTime + other.heuristic);
   }
};


// A node in the graph for pathfinding
struct Node {
   bool found = false;
   // Outgoing edges
   std::vector <StreetSegmentIdx> out_edges; 

   // Edge used to reach this node 
   StreetSegmentIdx reachingEdge = NO_EDGE;    

   // Shortest time found to this node so far
   double bestTime = DBL_MAX;  
};


// Function declarations for BFS
bool bfsPath (const IntersectionIdx srcID, const IntersectionIdx destID, const double turn_penalty);
std::vector<StreetSegmentIdx> bfsTraceBack (int destID);
