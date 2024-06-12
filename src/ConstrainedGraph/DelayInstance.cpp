#include "ConstrainedGraph/DelayInstance.h"
#include <algorithm>
#include <stdexcept>
#include <vector>
#include "graph/generate_graph.h"

namespace ConstrainedGraph {

DelayInstance::DelayInstance(const string& map_fname, const string& agent_fname, 
							 vector<Path>& paths,
							 int num_of_agents,
							 int num_of_delays,
                             bool collidingPaths, 
							 int num_of_rows, 
							 int num_of_cols, 
							 int num_of_obstacles, 
							 int warehouse_width):
	Instance(map_fname, agent_fname, num_of_agents, num_of_rows, num_of_cols,
		num_of_obstacles, warehouse_width),improvements_(0), collidingPaths_(collidingPaths)
{
	num_of_delays_ = num_of_delays;
	if (num_of_delays_ >= num_of_agents) {
		throw std::out_of_range("Number of delays must be strictly less than number of agents!");
	}
	originalPlan = paths;
	if (!collidingPaths_)
    {
        createDelay();
    }

	if (!delay_.empty())
	{
		// change starts according to delay
		changeStarts();
		// cut originalPlan until index = 0 == new_start
		for (int i = 0; i < originalPlan.size(); i++)
		{
			while (originalPlan[i][0].Loc.location != start_locations[i].location)
				originalPlan[i].erase(originalPlan[i].begin());
		}
		// make indices correct
		for (int id = 0; id < originalPlan.size(); id++)
		{
			for (int idx = 0; idx < originalPlan[id].size(); idx++)
			{
				originalPlan[id][idx].Loc.index = idx;
			}
		}
	}
    else
    {
        // make indices correct
        for (int id = 0; id < originalPlan.size(); id++) //
        {
            for (int idx = 0; idx < originalPlan[id].size(); idx++)
            {
                originalPlan[id][idx].Loc.index = idx;
            }
        }
        postDelayPlan = originalPlan;
        
        // change starts according to delay
        changeStarts();
    }
	cgGraphMaps.resize(num_of_agents, {});
	icgGraphMaps.resize(num_of_agents, {});

	// fill cg graphs
	fillcgGraphMap();
}

// Return path and stateCnt of an agent
Path DelayInstance::parse_path(string line) {
  int i, j;
  int time = 0;
  size_t comma_pos, leftPar_pos, rightPar_pos;
  Path path;

  while ((leftPar_pos = line.find("(")) != string::npos) {
    // Process an index pair
    comma_pos = line.find(",");
    i = stoi(line.substr(leftPar_pos + 1, comma_pos));
    rightPar_pos = line.find(")");
    j = stoi(line.substr(comma_pos + 1, rightPar_pos));
    line.erase(0, rightPar_pos + 1);

    // Create a location tuple and add it to the path
    int location=i*num_of_cols+j;
	Location loc(location, time);
	PathEntry entry(loc);
	path.push_back(entry);
    time++;
  }
  return path;
}

// Return all paths, accumulated counts of states, and States
std::vector<Path> DelayInstance::parse_paths(const string & path_fp) {
	std::vector<Path> paths;
	std::ifstream file(path_fp);
	if (file.is_open()) {
	string line;
	while (getline(file, line)) {
		// Sanity check that the line is a path
		if (line[0] == 'A') {
			Path path = parse_path(line);
			// Done with the agent
			paths.push_back(path);
		}
	}
	file.close();
	} else {
		std::cout << "exit\n";
		exit(0);
	}
	return paths;
}



DelayInstance::DelayInstance(
    const shared_ptr<Graph> & graph,
	const string& map_fp, 
	const std::string & scen_fp,
	const std::string & path_fp,
	const std::vector<int> & states,
	const std::vector<int> & delay_steps
):
Instance(map_fp, scen_fp, (int) states.size(), 0, 0,
		0, 0), improvements_(0), collidingPaths_(false)
{

	num_of_delays_=-1;

	// read init paths
	// auto full_paths = parse_paths(path_fp);

	// move to starts and add delays
	auto compressed_paths_ptr = ::parse_soln(path_fp.c_str());

	originalPlan.resize(states.size(), {});

	for (int i = 0; i < states.size(); i++)
	{

		// move to start
		int start = states[i];
		int delay_step = delay_steps[i];

		auto & compressed_path=compressed_paths_ptr->at(i);
		
		int idx=0;
		for (int k=0;k<delay_step;k++)
		{
			int location=compressed_path[start].first.first*num_of_cols+compressed_path[start].first.second;
			Location loc(location, idx);
			originalPlan[i].emplace_back(loc);
			++idx;
		}

		for (int j = start; j < compressed_path.size(); j++)
		{
			int location=compressed_path[j].first.first*num_of_cols+compressed_path[j].first.second;
			Location loc(location, idx);
			originalPlan[i].emplace_back(loc);
			++idx;
		}

		start_locations[i] = originalPlan[i][0].Loc;
		goal_locations[i] = originalPlan[i].back().Loc;

	}

	postDelayPlan=originalPlan;

	cgGraphMaps.resize(num_of_agents, {});
	icgGraphMaps.resize(num_of_agents, {});

	// fill cg graphs
	fillcgGraphMap();
}

void DelayInstance::fillcgGraphMap()
{
	// give every location in original_plan a list of neighbors, 
	// which change if it is a rep. point
	for (int a = 0; a < postDelayPlan.size(); a++) 
	{
		for (int k = 0; k < postDelayPlan[a].size(); k++)
		{
			Location curr = postDelayPlan[a][k].Loc;
			if ( postDelayPlan[a].back().Loc.index > (curr.index ) ) {
                cgGraphMaps[a].insert({curr, {postDelayPlan[a][curr.index + 1].Loc, curr}});
			} else {
				cgGraphMaps[a].insert({curr, {curr}});
			}
		}
	}
} 

void DelayInstance::fillicgGraphMap()
{
	// give every location in original_plan a list of neighbors, 
	// which change if it is a rep. point
	for (int a = 0; a < postDelayPlan.size(); a++)
	{
		for (int k = 0; k < postDelayPlan[a].size(); k++)
		{
			// std::cout << originalPlan[a][k].Loc.location << ", " << originalPlan[a][k].Loc.index;
			Location curr = postDelayPlan[a][k].Loc;
			if (std::find_if(rep_points_.begin(), rep_points_.end(),
    			[curr](const Location& l) { return (l.location == curr.location);}) == rep_points_.end())
			{
				// not a rep point, must move on if possible
				if (postDelayPlan[a].back().Loc.index > curr.index)
				{
					// must move on, add to unordered map
					icgGraphMaps[a].insert({curr, {postDelayPlan[a][k + 1].Loc}});
				}
				else
				{
					// at end of path, must self transition
					icgGraphMaps[a].insert({curr, {curr}});
				}
			}
			else
			{
				// curr IS a rep point, either move to next OR self transition
				if (postDelayPlan[a].back().Loc.index > curr.index)
					icgGraphMaps[a].insert({curr, {postDelayPlan[a][k + 1].Loc, curr}});
				else
					icgGraphMaps[a].insert({curr, {curr}});
			}
		}
	}
	// std::cout << "printing icgGraphMaps" << std::endl;
	// for (int a = 0; a < icgGraphMaps.size(); a++)
	// {
	// 	std::cout << a << std::endl;
	// 	std::cout << "start: " << start_locations[a].location << "," << start_locations[a].index << std::endl;
	// 	for (auto itr = icgGraphMaps[a].begin(); itr != icgGraphMaps[a].end(); itr++)
	// 	{
	// 		std::cout << (*itr).first.location << "," << (*itr).first.index << std::endl;
	// 		// print neighbors
	// 		for (Location l : (*itr).second)
	// 		{
	// 			std::cout << "	" << l.location << "," << l.index << std::endl;
	// 		}
	// 	}
	// 	// exit(1);
	// }
}

list<Location> DelayInstance::getNeighbors(Location curr)
{
	if (improvements_ == 0)
		return cgGraphMaps[agent_idx][curr];  // return CG neighbors
	else if (improvements_ == 1)
		return icgGraphMaps[agent_idx][curr];  // return ICG neighbors
	else
		return {};  // behaviour undefined
}

list<Location> DelayInstance::improvedCG_getNeighbors(Location curr)
{
	// now, use logic to determine the list of neighbors
	if (std::find_if(rep_points_.begin(), rep_points_.end(),
    		[curr](const Location& l) { return (l.location == curr.location);}) == rep_points_.end())
	{
		// curr is NOT a rep point, must move to next in path
		// original constrained graph
		if (postDelayPlan[agent_idx].back().Loc.index > curr.index)
			return {postDelayPlan[agent_idx][curr.index + 1].Loc}; // must move on
		else
			return {curr}; // at end of path, must self transition
	}
	else
	{
		// curr IS a rep point, either move to next OR self transition
		// original constrained graph
		if (postDelayPlan[agent_idx].back().Loc.index > curr.index)
			return {postDelayPlan[agent_idx][curr.index + 1].Loc, curr}; // move on or self transition
		else
			return {curr}; // at end of path, must selt transition
	}
}

void DelayInstance::calcRepresentativePoints()
{
	std::vector<Location> intersecting_points{};
	for (auto itr = postDelayPlan.begin(); itr != postDelayPlan.end(); itr++)
	{
		for (auto itr2 = postDelayPlan.begin(); itr2 != postDelayPlan.end(); itr2++)
		{
			if (itr != itr2)
			{
				// iterate through the two paths (itr* and itr2*) 
				for (auto pp1 = itr->begin(); pp1 != itr->end(); pp1++)
				{
					for (auto pp2 = itr2->begin(); pp2 != itr2->end(); pp2++)
					{
						if (((*pp1).Loc.location ==  (*pp2).Loc.location) && 
							((*pp1).Loc.location ==  (*pp2).Loc.location))
						{
							auto toFind = (*pp1).Loc;
							if (std::find_if(intersecting_points.begin(), intersecting_points.end(),
    							[&toFind](const Location& l) { return l.location == toFind.location;}) == intersecting_points.end())
								intersecting_points.push_back( (*pp1).Loc );
						}
					}
				}
			}
		}
	}

	if (!intersecting_points.empty())
	{
		// // sort based on time 
		// std::sort(intersecting_points.begin(), intersecting_points.end(), [](const auto& l, const auto& r){
    	// 	return (*l).index < (*r).index;});

		// add a representative point between each of the intersecting points
		// and add a representative point at each intersecting point
		for (auto i = intersecting_points.begin(); i != intersecting_points.end(); i++)
		{
			// only test i if not already inside rep_point
			auto toFind = *i;
			if (std::find_if(rep_points_.begin(), rep_points_.end(),
    			[&toFind](const Location& l) { return l.location == toFind.location;}) == rep_points_.end())
			{
				// add every intersecting point to rep point
				rep_points_.push_back(*i);

				// now add a rep. point between this intersecting point and the next one
				// do this for every path that consists of intersecting_point i

				for (auto pathPtr = postDelayPlan.begin(); pathPtr != postDelayPlan.end(); pathPtr++)
				{
					// for every path, find if Location i is inside
					for (auto pp = pathPtr->begin(); pp != pathPtr->end(); pp++)
					{
						if ( (*pp).Loc.location == (*i).location )
						{
							// found current intersection point, now see if next point is in intersecting
							// add next point to rep points if it is NOT in intersection points
							auto pp_nxt = std::next(pp);
							if (std::find_if(intersecting_points.begin(), intersecting_points.end(),
    							[pp_nxt](const Location& l) { return l.location == (*pp_nxt).Loc.location;}) == intersecting_points.end())
							{
								// pp_nxt is NOT an intersection point, now add to rep point
								rep_points_.push_back((*pp_nxt).Loc);
							}
							// 
							// for (auto i_nxt = intersecting_points.begin(); i_nxt != intersecting_points.end(); i_nxt++)
							// {
							// 	if ( (*pp_nxt).Loc.location == (*i_nxt).location )
							// 	{
							// 		// finally, can add pp_nxt.Loc as rep point
							// 		add_pp_nxt = false;
							// 		break;
							// 	}
							// }
							// if (!add_pp_nxt)
							// 	rep_points_.push_back((*pp_nxt).Loc);
						}
					}

				}
			}
		}
		assert(intersecting_points.size() <= rep_points_.size());
		// std::cout << "int size: " << intersecting_points.size() << std::endl;
		// std::cout << "rep size: " << rep_points_.size() << std::endl;
	}
	
	// add new start locations rep points as well
	for (auto s : start_locations)
	{
		auto toFind = s;
		if (std::find_if(rep_points_.begin(), rep_points_.end(),
    			[&toFind](const Location& l) { return l.location == toFind.location;}) == rep_points_.end())
			rep_points_.push_back(s);
	}
}

void DelayInstance::createDelay()
{
	int numAttempts = 0;
	std::vector<int> agents(num_of_agents);
	for (int i = 0; i < num_of_agents; i++) {
		agents[i] = i;
	}

	while ( numAttempts < 50 && delay_.empty() )
	{
		std::vector<int> delayedAgents;

		std::sample(agents.begin(), agents.end(), std::back_inserter(delayedAgents),
            num_of_delays_, std::mt19937{std::random_device{}()});

		vector<int> path_lengths;
		// int minSize = INT_MAX;
		// int minSizeIdx = -1;
		for (auto &a : delayedAgents) {
			int pathSize = originalPlan[a].size();
			path_lengths.push_back(pathSize);
			// if (pathSize < minSize) {
				// minSize = pathSize;
				// minSizeIdx = a;
			// }
		}

		// now we have a vector of paths and a min size of paths
		auto pItr = std::min_element(path_lengths.begin(), path_lengths.end());
		int minSize = *pItr; 
		
		if (minSize > 3) {
			const int delayed_point = std::rand() % (minSize - 2);
			for (int i = 0; i < num_of_delays_; i++) {
				delay_.push_back(new Delay(delayedAgents[i], delayed_point));
			}

			// if we do not need to replan, need to try another delay!
			if (isCollisionFree())
			{
				postDelayPlan.clear();
				for (int i = 0; i < num_of_delays_; i++) {
					delete delay_[i];
					delay_[i] = nullptr;
				}
				delay_.clear();
			}
			else {
				// make sure delayed agent does not collide immediately after delay
				bool isValid = true;
				for (auto &delay : delay_) {
					if (!isValid) {
						break;
					}
					Path delayed_path = postDelayPlan[delay->first];
					for (int aid = 0; aid < postDelayPlan.size(); aid++)
					{
						if (aid != delay->first)
						{
							if (postDelayPlan[aid].size() > (delay->second + 1) )
							{
								// cannot have a delay where an agent 
								// collides immediately after delay
								if (postDelayPlan[aid][(*delay).second + 1].Loc.location == delayed_path[(*delay).second + 1].Loc.location)
								{
									postDelayPlan.clear();
									for (int i = 0; i < num_of_delays_; i++) {
										delete delay_[i];
										delay_[i] = nullptr;
									}
									delay_.clear();
									isValid = false;
									break;
								}
							}
						}
					}
				}
			}
		}
		numAttempts++;
		// // for (auto pe : originalPlan[delayed_agent])
		// // 	std::cout << getRowCoordinate(pe.Loc.location) << "," << 
		// // 		getColCoordinate(pe.Loc.location) << std::endl;

		// if (path_length > 3)
		// {
		// 	const int delayed_point = std::rand() % (path_length - 2);
		// 	// const int delayed_point = 7; // only for bug fix
		// 	// set delay parameter
		// 	delay_ = new Delay(delayed_agent, delayed_point);

		// 	// for debugging purposes ONLY
		// 	// delay_ = new Delay(7, 0);

		// 	// if we do not need to replan, need to try another delay!
		// 	if (isCollisionFree())
		// 	{
		// 		postDelayPlan.clear();
		// 		delete delay_;
		// 		delay_ = nullptr;
		// 	}
		// 	else
		// 	{
		// 		// make sure delayed agent does not collide immediately after delay
		// 		Path delayed_path = postDelayPlan[(*delay_).first];
		// 		for (int aid = 0; aid < postDelayPlan.size(); aid++)
		// 		{
		// 			if (aid != delay_->first)
		// 			{
		// 				if (postDelayPlan[aid].size() > (delay_->second + 1) )
		// 				{
		// 					// cannot have a delay where an agent 
		// 					// collides immediately after delay
		// 					if (postDelayPlan[aid][(*delay_).second + 1].Loc.location == delayed_path[(*delay_).second + 1].Loc.location)
		// 					{
		// 						postDelayPlan.clear();
		// 						delete delay_;
		// 						delay_ = nullptr; 
		// 						break;
		// 					}
		// 				}
		// 			}
		// 		}
		// 	}
		// }
		
	}
	if (!delay_.empty())
	{
		std::cout << "Found a set of " << delay_.size() << " delays" << endl;
		// std::cout << "Found Delay after " << numAttempts << " attempts!" << std::endl;
		// std::cout << "Agent: " << (*delay_).first << std::endl;
		// std::cout << "Time: " << (*delay_).second << std::endl;
		std::cout << "Collision occurs at time: " << delay_[0]->second << std::endl;

		// int i = 13;
		// std::cout << "Agent " << i << std::endl;
		// for (auto pe : postDelayPlan[(*delay_).first])
		// {
		// 	std::cout << "	" << pe.Loc.index << " " << pe.Loc.location << std::endl;
		// }
		// std::cout << "leaving createDelay" << std::endl;
	}
	else
	{
		std::cout << "No delay after " << numAttempts << " attempts." << std::endl;
		exit(1);
	}
}

bool DelayInstance::isCollisionFree()
{
	// make a deep copy of existing solution
	for (int p = 0; p < originalPlan.size(); p++)
	{
		vector<PathEntry> new_path = {};
		for (int idx = 0; idx < originalPlan[p].size(); idx++)
		{
			PathEntry pe(originalPlan[p][idx]);
			pe.Loc.index = idx;
			new_path.push_back(pe);
		}
        postDelayPlan.push_back(new_path);
	}

	for (auto &delay : delay_) {
		// insert the delay to plan
		PathEntry delay_entry = originalPlan[(*delay).first][(*delay).second];
		PathEntry new_entry(delay_entry.Loc.location);
		postDelayPlan[(*delay).first].insert(postDelayPlan[(*delay).first].begin()+(*delay).second+1, new_entry);
	}
	

	int i = 0;
	for (auto &path : postDelayPlan)
	{
		// std::cout << "Agent " << i << std::endl;
		bool needIncrement = false;
		for (auto &pe : path)
		{
			if (pe.Loc.index == -1)
			{
				pe.Loc.index = (*delay_[0]).second;
				needIncrement = true;
			}
			if (needIncrement)
				pe.Loc.index = pe.Loc.index + 1;
			// std::cout << "	" << pe.Loc.index << " " << pe.Loc.location << std::endl;
		}
		i++;
	}

	vector<Path*> path_ptrs{};
	int soc = 0;
	for (auto &path : postDelayPlan)
	{
		soc += path.size() - 1;
        path_ptrs.push_back(&path);
    }

	// simulate plan and return if conflict exists or not
	return validateSolution(path_ptrs, soc, 0);
}

void DelayInstance::changeStarts()
{
	// for (int p_idx = 0; p_idx < postDelayPlan.size();  p_idx++)
	// {
	// 	// get new start
	// 	Location new_start(-1, 0);
	// 	auto p_itr = postDelayPlan.begin() + p_idx;
	// 	if ( (*p_itr).size() > (*delay_).second + 1 )
	// 	{
	// 		new_start.location = (*((*p_itr).begin() + (*delay_).second + 1)).Loc.location;
	// 		new_start.index = (*((*p_itr).begin() + (*delay_).second + 1)).Loc.index;
	// 	}
	// 	else
	// 	{
	// 		new_start.location = (*p_itr).back().Loc.location;
	// 		new_start.index = (*p_itr).back().Loc.index;
	// 	}
	// 	assert(new_start.location >= 0 && new_start.index >= 0);
	// 	std::cout << new_start.index << std::endl;
	// 	// place it into the start locations vector
	// 	start_locations[p_idx] = new_start;
	// }
    if (!collidingPaths_)
    {
        int i = 0;
        for (auto path : postDelayPlan)
        {
            Location new_start(-1, -1);
            if (path.size() > (*delay_[0]).second + 1)
            {
                new_start.location = path[(*delay_[0]).second + 1].Loc.location;
                new_start.index = path[(*delay_[0]).second + 1].Loc.index;
            }
            else
            {
                new_start.location = path.back().Loc.location;
                new_start.index = path.back().Loc.index;
            }
            // std::cout << new_start.index << std::endl;
            start_locations[i] = new_start;
            goal_locations[i].index = path.back().Loc.index;
            i++;
        }
    }
    else
    {
        int i = 0;
        for (auto &path : postDelayPlan)
        {
            Location new_start(path[0].Loc.location, path[0].Loc.index);
            start_locations[i] = new_start;
            goal_locations[i].index = path.back().Loc.index;
            i++;
        }
    }
}

};