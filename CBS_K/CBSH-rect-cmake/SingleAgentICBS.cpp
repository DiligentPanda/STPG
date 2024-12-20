#include "SingleAgentICBS.h"
#include "constrained_map_loader.h"
#include <iostream>
#include <ctime>

template<class Map>
void SingleAgentICBS<Map>::updatePath(LLNode* goal, std::vector<PathEntry> &path,ReservationTable* res_table)
{
    path.clear();
	path.resize(goal->g_val + 1);
	LLNode* curr = goal;
	num_of_conf = goal->num_internal_conf;

	// No shrink to hole any more.
	// Include the goal shrink process to path
//    list<int> goalLocs =  goal->locs;
//    goalLocs.pop_back();
//    for (int t = goal->g_val+1; t < path.size();t++){
//        path[t].location = goalLocs.front();
//        path[t].occupations = goalLocs;
//        path[t].actionToHere = goal->heading;
//        path[t].heading = goal->heading;
//        path[t].singles.clear();
//        path[t].self_conflict = false;
//        if (t!=0)
//            path[t].conflist =  res_table->findConflict(agent_id, goalLocs.front(), goalLocs, t-1);
//        else
//            path[t].conflist = NULL;
//        goalLocs.pop_back();
//	}

	for(int t = goal->g_val; t >= 0; t--)
	{

		path[t].Loc = curr->locs.front();
		path[t].occupations = curr->locs;
		path[t].actionToHere = curr->heading;
        path[t].heading = curr->heading;
        path[t].singles.clear();
        path[t].self_conflict = curr->self_conflict;
        if (t!=0) {
            std::list<int> curr_locs;
            for (auto loc:curr->locs){
                curr_locs.push_back(loc.location);
            }
            path[t].conflist =  res_table->findConflict(agent_id, curr->parent->locs.front().location, curr_locs, t-1, t==goal->g_val);
        }
        else {
            std::list<int> curr_locs;
            for (auto loc:curr->locs){
                curr_locs.push_back(loc.location);
            }
            path[t].conflist =  res_table->findConflict(agent_id, curr->locs.front().location, curr_locs, t-1);
        }

		curr = curr->parent;
	}


}

template<class Map>
void SingleAgentICBS<Map>::compute_postDelay_heuristics(Path &path)
{
    // heuristic is simply number of steps to goal in original path
    this->ml->set_agent_idx(this->agent_id);
    // std::cout << "computing heuristics for agent " << instance->agent_idx << std::endl;
    // my_heuristic.resize((size_t)(ml->rows*ml->cols), hvals(MAX_COST));
    int value = 0;
    for (auto ritr = path.rbegin(); ritr != path.rend(); ++ritr )
    {
        // std::cout << (*ritr).Loc.location << "-->" << value << std::endl;
        my_heuristic[ritr->Loc] = hvals(value);

        // only record the last one.
        if (my_heuristic_i.find(ritr->Loc.location) == my_heuristic_i.end())
            my_heuristic_i[ritr->Loc.location] = hvals(value);
        // if (instance->agent_idx == 12)
        // {
        //     std::cout << "h: " << (*ritr).Loc.location << " --> " << my_heuristic[(*ritr).Loc.location] << std::endl;
        // }
        value++;
    }
    // int index = 0;
    // start_location.index = 0;
    // // while we are at it, make sure to change indices
    // for (auto itr = path.begin(); itr != path.end(); ++itr )
    // {
    //     (*itr).Loc.index = index;
    //     index++;
    // }
    // goal_location.index = index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// return true if a path found (and updates vector<int> path) or false if no path exists
template<class Map>
bool SingleAgentICBS<Map>::findPath(std::vector<PathEntry> &path, double f_weight, ConstraintTable& constraint_table,
	ReservationTable* res_table, size_t max_plan_len, double lowerbound, std::clock_t start_clock ,int time_limit, bool train)
{
    // std::cout<<start_location.location<<","<<start_location.index<<std::endl;
    // std::cout<<goal_location.location<<","<<goal_location.index<<std::endl;

    ml->set_agent_idx(agent_id);

	if (constraint_table.is_constrained(start_location.location, 0)) {
		return false;
	}
	num_expanded = 0;
	num_generated = 0;

	hashtable_t::iterator it;  // will be used for find()

    // std::cout<<"agent "<<agent_id<<", h-start:"<<my_heuristic[start_location].get_hval(start_heading)<<std::endl;

    // std::cout<<"path: "<<ml->postDelayPlan[agent_id].size()<<" ";
    // for (auto & pe: ml->postDelayPlan[agent_id]){
    //     std::cout<<pe.Loc.location<<",";
    // }
    // std::cout<<std::endl;

	 // generate start and add it to the OPEN list
	LLNode* start = new LLNode(list<Location>(), 0, my_heuristic[start_location].get_hval(start_heading),
	        NULL, 0, 0, false, train);
    
    start->locs.resize(kRobust+1,start_location);
	start->heading = start_heading;
	num_generated++;
	start->open_handle = open_list.push(start);
	start->focal_handle = focal_list.push(start);
	start->in_openlist = true;
	start->time_generated = 0;
	start->num_internal_conf= 0;
	if(option.shrink)
	    start->h_val += start->locs.size() -1;


	allNodes_table.insert(start);
	min_f_val = start->getFVal();

	max_plan_len = std::max((int)max_plan_len,(int)constraint_table.latest_timestep);
//    max_plan_len = std::max((int)max_plan_len,(int)constraint_table.length_min);


	lowerbound = std::max(lowerbound, (double)constraint_table.length_min);
	lowerbound = std::max(lowerbound, (double)min_end_time);
	bound =  f_weight * min_f_val;
//	cout<<"initial bound "<<bound<<", "<<constraint_table.length_min<<","<<min_end_time<<","<<min_f_val<<f_weight<<","<<lowerbound<<
//	", length_max "<< constraint_table.length_max <<", latest_timestep"<<constraint_table.latest_timestep<<endl;
	int time_generated = 0;
	int time_check_count = 0;
	std:clock_t runtime;


	while (!focal_list.empty()) 
	{
		if (num_generated / 10000 > time_check_count && time_limit != 0) {
			runtime = std::clock() - start_clock;
			time_check_count = num_generated / 10000;
			 if (runtime > time_limit) {
			 	return false;
			 }
		}

		LLNode* curr = focal_list.top(); 
        focal_list.pop();
		open_list.erase(curr->open_handle);

		curr->in_openlist = false;
		num_expanded++;
//		if(goal_location == 435 ){
//		    std::cout << "Pick node current: (";
//		    for (auto loc : curr->locs){
//		        cout<< loc<<"|";
//		    }
//		    cout<<"," << curr->heading << "," << curr->getFVal() << ") "
//		    <<" g:"<<curr->g_val <<" shrinking:"<<curr->shrinking<<" timestep:"<<curr->timestep<<" length_max:"<<constraint_table.length_max <<" max_plan_len:"<<max_plan_len<< std::endl;
//		}

		// check if the popped node is a goal
		if (curr->locs.front() == goal_location && curr->timestep >= lowerbound)
		{

			if (curr->parent == NULL || curr->shrinking || curr->parent->locs.front() != goal_location)
			{
				//cout << num_generated << endl;

				updatePath(curr, path, res_table);

				releaseClosedListNodes(&allNodes_table);

				open_list.clear();
				focal_list.clear();

				allNodes_table.clear();
				goal_nodes.clear();

				return true;
			}
		}
		

		if (curr->g_val >= constraint_table.length_max) {
			continue;
		}

		vector<pair<Location, int>> transitions;
	    transitions = ml->get_transitions(curr->locs.front(), curr->heading,false);

		for (const pair<Location, int> & move : transitions)
		{
			Location next_id = move.first;
			list<Location> next_locs;

            bool no_self_conflict = true;
            if (option.shrink && move.second == -2){
                next_locs = curr->locs;
                next_locs.pop_back();
            }
            else
                no_self_conflict = getOccupations(next_locs, next_id, curr);

            if (train and !no_self_conflict)
			    continue;

			assert(next_locs.size()<= kRobust+1);
			time_generated += 1;
			int next_timestep = curr->timestep + 1;

            if (max_plan_len <= curr->timestep && move.second!= -2 && !curr->shrinking )
            {
                if (next_id == curr->locs.front())
                {
                    continue;
                }
                next_timestep-- ;
            }

            //check does head have edge constraint or body have vertex constraint.
            bool constrained = false;

            //Check edge constraint on head
            if (curr->locs.front().location != -1 && constraint_table.is_constrained(curr->locs.front().location * map_size + next_id.location, next_timestep))
                constrained = true;

            //Check vertex constraint on body and head
            for(auto loc:next_locs){
                if (loc.location == -1)
                    continue;
                if (constraint_table.is_constrained(loc.location, next_timestep, loc != next_locs.front()) )
                    constrained = true;
                if(!train) //if not train, only check head
                break;
            }

			if (constrained)
			    continue;

            // compute cost to next_id via curr node
            int next_g_val = curr->g_val + 1;
            int next_heading;

            if (curr->heading == -1) //heading == 4 means no heading info
                next_heading = -1;
            else
                if (move.second == 4 || move.second == -2) //move == 4 means wait
                    next_heading = curr->heading;
                else
                    next_heading = move.second;

            int next_h_val;
            if (ml->flatland && next_id.location == -1)
                next_h_val = my_heuristic[start_location].get_hval(next_heading) ;
            else
                next_h_val = my_heuristic[next_id].get_hval(next_heading);

            if (option.shrink)
                next_h_val = next_h_val + next_locs.size() - 1;


            if (next_g_val + next_h_val > constraint_table.length_max) {
                continue;
            }

            std::list<int> _next_locs;
            for(auto & loc:next_locs){
                _next_locs.push_back(loc.location);
            }
            int next_internal_conflicts = curr->num_internal_conf +  res_table->countConflict(agent_id, curr->locs.front().location, _next_locs, curr->timestep);

            // generate (maybe temporary) node
            LLNode* next = new LLNode(next_locs, next_g_val, next_h_val,	curr, next_timestep, next_internal_conflicts, false, train);
            next->heading = next_heading;
            next->actionToHere = move.second;
            next->time_generated = time_generated;
            next->self_conflict = !no_self_conflict;
            if(move.second == -2 || curr->shrinking)
                next->shrinking = true;
//            std::cout << "----current: (" << curr->locs.front() << "," << curr->heading << "," << curr->getFVal() << ") " << "next: (" << next->locs.front() << "," << next->heading << "," << next->getFVal() << ")" << std::endl;

//            if (agent_id == 0 && ( curr->locs.front() == 519)){
//                cout << "current: " <<curr->locs.front()<<","<<curr->locs.back()<<","<< curr->g_val<<","<<curr->h_val<<","<<curr->num_internal_conf<<curr->getFVal() << endl;
//                cout << "child: " <<next->locs.front()<<","<<next->locs.back()<<","<< next->g_val<<","<<next->h_val<<","<<next->num_internal_conf<<next->getFVal() << endl;
//                cout << "min_f"<<min_f_val<<",l b"<<bound<<","<<open_list.top()->getFVal()<<endl;
//
//            }
            // try to retrieve it from the hash table
            it = allNodes_table.find(next);
            if (it == allNodes_table.end() || (next_id == goal_location && (constraint_table.length_min > 0 || min_end_time >0)) )
            {

                //cout << "Possible child loc: " << next->loc << " heading: " << next->heading << " f: " << next->getFVal() << " g: " << next->g_val << " h: " << next->h_val<< " num_internal_conf: " << next->num_internal_conf << endl;
                //cout << "h: " << my_heuristic[next_id].get_hval(next_heading) << endl;


                next->open_handle = open_list.push(next);
                next->in_openlist = true;
                num_generated++;
                if (next->getFVal() <= bound) {
                    //cout << "focal size " << focal_list.size() << endl;
                    //cout << "put in focal list" << endl;
                    next->focal_handle = focal_list.push(next);
                    next->in_focallist = true;
                    //cout << "focal size " << focal_list.size() << endl;


                }

                if (it == allNodes_table.end())
                    allNodes_table.insert(next);
                else
                    goal_nodes.push_back(next);

            }
            else
            {  // update existing node's if needed (only in the open_list)
                delete(next);  // not needed anymore -- we already generated it before
                LLNode* existing_next = (*it);

                if (existing_next->in_openlist == true)
                {  // if its in the open list
                    if (existing_next->getFVal() > next_g_val + next_h_val ||
                        (existing_next->getFVal() == next_g_val + next_h_val && existing_next->num_internal_conf > next_internal_conflicts))
                    {
                        // if f-val decreased through this new path (or it remains the same and there's less internal conflicts)
                        bool add_to_focal = false;  // check if it was above the focal bound before and now below (thus need to be inserted)
                        bool update_in_focal = false;  // check if it was inside the focal and needs to be updated (because f-val changed)
                        bool update_open = false;
                        if ((next_g_val + next_h_val) <= bound)
                        {  // if the new f-val qualify to be in FOCAL
                            if (existing_next->getFVal() > bound)
                                add_to_focal = true;  // and the previous f-val did not qualify to be in FOCAL then add
                            else
                                update_in_focal = true;  // and the previous f-val did qualify to be in FOCAL then update
                        }
                        if (existing_next->getFVal() > next_g_val + next_h_val)
                            update_open = true;
                        // update existing node
                        existing_next->g_val = next_g_val;
                        existing_next->h_val = next_h_val;
                        existing_next->timestep = next_timestep;
                        existing_next->self_conflict = !no_self_conflict;
                        existing_next->locs = next_locs;

                        existing_next->parent = curr;
                        existing_next->num_internal_conf = next_internal_conflicts;
                        if (update_open)
                            open_list.increase(existing_next->open_handle);  // increase because f-val improved
                        if (add_to_focal)
                            existing_next->focal_handle = focal_list.push(existing_next);
                        if (update_in_focal)
                            focal_list.update(existing_next->focal_handle);  // should we do update? yes, because number of conflicts may go up or down
                    }
                }
                else
                {  // if its in the closed list (reopen)
                    if (existing_next->getFVal() > next_g_val + next_h_val ||
                        (existing_next->getFVal() == next_g_val + next_h_val && existing_next->num_internal_conf > next_internal_conflicts))
                    {
                        // if f-val decreased through this new path (or it remains the same and there's less internal conflicts)
                        existing_next->g_val = next_g_val;
                        existing_next->h_val = next_h_val;
                        existing_next->timestep = next_timestep;
                        existing_next->self_conflict = !no_self_conflict;
                        existing_next->locs = next_locs;



                        existing_next->parent = curr;
                        existing_next->num_internal_conf = next_internal_conflicts;
                        existing_next->open_handle = open_list.push(existing_next);
                        existing_next->in_openlist = true;
                        if (existing_next->getFVal() <= bound)
                            existing_next->focal_handle = focal_list.push(existing_next);
                    }
                }  // end update a node in closed list
            }  // end update an existing node

//            if (agent_id == 10 && curr->locs.front() == 458){
//                auto it1 = allNodes_table.find(next);
//                assert(it1 != allNodes_table.end());
//                LLNode* existing_next = (*it1);
//                cout<<"find "<<existing_next->locs.front()<<","<<existing_next->locs.back()<<","<< existing_next->g_val<<","<<existing_next->h_val << ","<<existing_next->num_internal_conf<<","<<existing_next->getFVal()<<","<<existing_next->in_focallist<< endl;
//            }
		}  // end for loop that generates successors
		//cout << "focal list size"<<focal_list.size() << endl;
		// update FOCAL if min f-val increased
		if (open_list.size() == 0)  // in case OPEN is empty, no path found
			break;
		LLNode* open_head = open_list.top();

        assert(open_head->getFVal() >= min_f_val);

		if (open_head->getFVal() > min_f_val) 
		{

			double new_min_f_val = open_head->getFVal();
			double new_bound = std::max(lowerbound, f_weight * new_min_f_val);

			for (LLNode* n : open_list) 
			{

				if (!n->in_focallist && n->getFVal() > bound && n->getFVal() <= new_bound) {

					n->focal_handle = focal_list.push(n);
					n->in_focallist = true;
				}
			}

			min_f_val = new_min_f_val;
			bound = new_bound;

		}

	}  // end while loop

	  // no path found
	releaseClosedListNodes(&allNodes_table);
	open_list.clear();
	focal_list.clear();
	allNodes_table.clear();
	goal_nodes.clear();

	return false;
}

template<class Map>
bool SingleAgentICBS<Map>::getOccupations(list<Location> & next_locs, Location next_id, LLNode* curr){
    next_locs.push_back(next_id);
    auto parent = curr;
    Location & pre_loc = next_id;
    bool conf_free = true;
    while(next_locs.size()<=kRobust){
        if (parent == nullptr){
            next_locs.push_back(next_locs.back());
        }
        else {
            if (pre_loc != parent->locs.front()) {
                next_locs.push_back(parent->locs.front());
                pre_loc = parent->locs.front();
                if (next_locs.front() == next_locs.back()) {
                    conf_free = false;
                }
            }
            parent = parent->parent;
        }

    }

    return conf_free;
}

template<class Map>
inline void SingleAgentICBS<Map>::releaseClosedListNodes(hashtable_t* allNodes_table)
{

	hashtable_t::iterator it;
	for (it = allNodes_table->begin(); it != allNodes_table->end(); ++it) {

			delete (*it);
	}
	for (auto node : goal_nodes)
		delete node;

}

template<class Map>
        SingleAgentICBS<Map>::SingleAgentICBS(Location start_location, Location goal_location,  Map* ml1, int agent_id, options cbs_option, int start_heading, int kRobust, int min_end):ml(ml1)
{
	this->agent_id = agent_id;
	this->start_heading = start_heading;

	this->start_location = start_location;
	this->goal_location = goal_location;
	this->min_end_time = min_end;

	this->map_size = ml->cols*ml->rows;

	this->num_expanded = 0;
	this->num_generated = 0;

	this->bound = 0;
	this->min_f_val = 0;

	this->num_col = ml->cols;

	this->kRobust = kRobust;
	this->option = cbs_option;
	// initialize allNodes_table (hash table)
	empty_node = new LLNode();
	empty_node->locs.push_back(Location(-1,-1));

    deleted_node = new LLNode();
	deleted_node->locs.push_back(Location(-2,-1));


}

template<class Map>
SingleAgentICBS<Map>::~SingleAgentICBS()
{
    if(empty_node!=NULL){
	    delete (empty_node);
        empty_node=NULL;
    }
    if(deleted_node!=NULL) {
        delete (deleted_node);
        deleted_node=NULL;
    }
}

template class SingleAgentICBS<ConstrainedMapLoader>;
