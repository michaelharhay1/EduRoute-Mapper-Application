/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 25/03/2024
 *
 * Description: implements functions related to map routing and pathfinding using 
 * BFS functions improved by Dijkstra's algorithm. Has a function to
 * computesthe total travel time for a path and finds path between two intersections.
 */

#include "m3.h"
#include "m1.h"

#include "m3_helper/m3_helper.h"
#include "m1_helper/m1_globals.h"

// Iterates through each street segment in the provided path, adding
// up the time for each and adding turn penalties for each change of street.
double computePathTravelTime(const double turn_penalty,
                             const std::vector<StreetSegmentIdx> &path)
{
  double total_travel_time = 0.0;

  if (path.size() == 0)
  {
    return total_travel_time;
  }

  StreetSegmentIdx prev_segment_id = path[0];

  // Add travel time for the first street segment
  total_travel_time += findStreetSegmentTravelTime(prev_segment_id);

  // Loop through remaining
  for (size_t i = 1; i < path.size(); ++i)
  {

    StreetSegmentIdx current_segment_id = path[i];
    total_travel_time += findStreetSegmentTravelTime(current_segment_id);

    // Check if there is a turn and add turn penalty
    if (get_street_seg_street_id(prev_segment_id) != get_street_seg_street_id(current_segment_id))
    {

      total_travel_time += turn_penalty;
    }

    // Update the previous segment
    prev_segment_id = current_segment_id;
  }
  return total_travel_time;
}

// Finds a path between 2 intersections by calling bfsPath that uses Dijkstra's
// algorithm from first to end intersection. If path found, traces back using
// bfsTraceBack function that returns vector of street segment indexes.
std::vector<StreetSegmentIdx> findPathBetweenIntersections(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids)
{
  bool found = bfsPath(intersect_ids.first, intersect_ids.second, turn_penalty);

  if (found)
  {
    return bfsTraceBack(intersect_ids.second);
  }

  return {};
}