#include "MDD.h"

#include <iostream>

template<class Map>
bool MDD<Map>::buildMDD( ConstraintTable& constraints, int numOfLevels, SingleAgentICBS<Map> & solver, bool train, bool shrink)
{
	MDDNode* root = new MDDNode(std::list<Location>(), nullptr,train); // Root
	root->locs.resize(solver.kRobust+1, solver.start_location);
	root->heading = solver.start_heading;
	root->row = solver.start_location.location / solver.num_col;
	root->col = solver.start_location.location % solver.num_col;
	std::queue<MDDNode*> open;
	std::list<MDDNode*> closed;
	open.push(root);
	closed.push_back(root);
	levels.resize(numOfLevels);
	//cout << "start: " << solver.start_heading << endl;
	//cout << "goal: " << solver.goal_location << endl;
	bool done = false;
//	cout<<"start"<<endl;
    int max_level = 0;
	while (!open.empty())
	{
		MDDNode* node = open.front();
		open.pop();
		if (node->level > max_level){
//		    cout<<max_level<<endl;
		    max_level = node->level;
		}
		// Here we suppose all edge cost equals 1
		if (node->level == numOfLevels - 1)
		{
			levels[numOfLevels - 1].push_back(node);

			if(!shrink && train && !open.empty()){
			    while (!open.empty()){
			        MDDNode* n = open.front();

			        bool duplicate = false;
			        for (auto* n_l: levels[numOfLevels - 1]){
			            if (*(n_l) == *(n)){
			                duplicate = true;
			                break;
			            }
			        }

			        levels[numOfLevels - 1].push_back(n);
			        if (duplicate)
			            break;

			        open.pop();
			    }
			}

			if(!open.empty() )
			{
			    cout << " Expand node: " << node->locs.front().location<<","<< node->locs.back().location<<","<<node->locs.size() << " heading: " << node->heading<<" h "<< solver.my_heuristic[node->locs.front().location].get_hval(node->heading)<<", level: "<<node->level << endl;
				while (!open.empty())
				{
					MDDNode* n = open.front();
					open.pop();
					cout << "loc: ";
					for(auto loc: n->locs){
					    cout<<loc.location<<"|";
					}
					cout <<" level: "<< n->level << " heading: " << n->heading<<" h "<< solver.my_heuristic[n->locs.front().location].get_hval(n->heading) <<", level: "<<n->level<< endl;

				}
				
				std::cerr << "Failed to build MDD!" << std::endl;
				assert(false);
				exit(1);
			}
			done = true;
			break;
		}
		// We want (g + 1)+h <= f = numOfLevels - 1, so h <= numOfLevels - g. -1 because it's the bound of the children.
//		double heuristicBound = numOfLevels - node->level - 2+ 0.001;

//        cout << " Expand node: " << node->locs.front()<<","<< node->locs.back()<<","<<node->locs.size() << " heading: " << node->heading<<" h "<< solver.my_heuristic[node->locs.front()].heading[node->heading]<<", level: "<<node->level << endl;



		vector<pair<Location, int>> transitions;
		transitions = solver.ml->get_transitions(node->locs.front(), node->heading,false);

		if (shrink && node->locs.front() == solver.goal_location){
		    transitions.emplace_back(node->locs.front(), -2); //-2 indicate shrinking
		}
		assert(!transitions.empty());
		for (pair<Location, int> & move : transitions)
		{
			int new_heading;
			if (node->heading == -1) //heading == -1 means no heading info
				new_heading = -1;
			else
			    if (move.second == 4 || move.second == -2) //move == 4 means wait
					new_heading = node->heading;
				else
					new_heading = move.second;
			Location & newLoc = move.first;


			std::list<Location> new_locs;
			bool no_self_collision = true;
			if (shrink && move.second == -2){
			    new_locs = node->locs;
			    new_locs.pop_back();
			}
			else
			no_self_collision = getOccupations(new_locs, newLoc, node,solver.kRobust);
			if (new_locs.empty())
			    continue;
            //check does head have edge constraint or body have vertex constraint.
            bool constrained = false;
            if (node->locs.front().location != -1 && constraints.is_constrained(node->locs.front().location * solver.map_size + newLoc.location, node->level + 1))
                constrained = true;


            for(auto loc:new_locs){
                if (loc.location == -1)
                    break;
                if (constraints.is_constrained(loc.location, node->level + 1, loc != new_locs.front()) )
                    constrained = true;
                if(!train)
                    break;
            }
            double heuristicBound;
            if (train)
                heuristicBound =  double(numOfLevels) - double(node->level) - 2.0+ 0.001;
            else
                heuristicBound = numOfLevels - node->level - 2+ 0.001;

            int heuristic;
            heuristic = solver.my_heuristic[newLoc.location].get_hval(new_heading);

            assert(newLoc == solver.goal_location || heuristic!=0);

//            if(shrink)
//                heuristicBound += node->locs.size();

//            cout << "newLoc " << newLoc << " heading " << new_heading<<" h "<< solver.my_heuristic[newLoc].heading[new_heading] <<", hb"<<heuristicBound<<", c "<<constrained <<endl;

			if (heuristic < heuristicBound &&
				!constrained) // valid move
			{
				std::list<MDDNode*>::reverse_iterator child = closed.rbegin();
				bool find = false;
				for (; child != closed.rend() && ((*child)->level == node->level + 1); ++child) {
					std::list<int> _child_locs;
					for (auto & loc : (*child)->locs) {
						_child_locs.push_back(loc.location);
					}
					std::list<int> _new_locs;
					for (auto & loc : new_locs) {
						_new_locs.push_back(loc.location);
					}

					if (equal_occupation(_child_locs, _new_locs) || (!train && _child_locs.front()==_new_locs.front()))  // If the child node exists
					{
						if ((*child)->heading == -1) { //if no heading info
							(*child)->parents.push_back(node); // then add corresponding parent link and child link
							find = true;
							break;
						}
						else if ((*child)->locs.front().location == solver.goal_location.location) { //if goal location ignore heading
							(*child)->parents.push_back(node); // then add corresponding parent link and child link
							find = true;
							break;
						}
						else if ((*child)->heading == new_heading) {//child heading equal to node heading
							(*child)->parents.push_back(node); // then add corresponding parent link and child link
							find = true;
							break;
						}

						
					}
				}
				if (!find) // Else generate a new mdd node
				{

					MDDNode* childNode = new MDDNode(new_locs, node,train);
					childNode->parent = node;
					childNode->heading = new_heading;
					childNode->row = newLoc.location / solver.num_col;
					childNode->col = newLoc.location % solver.num_col;
					if(move.second == -2 || node->shrinking)
					    childNode->shrinking = true;

					open.push(childNode);
					closed.push_back(childNode);
				}
			}
		}
	}

	// Backward
	for (int t = numOfLevels - 1; t > 0; t--)
	{
		for (std::list<MDDNode*>::iterator it = levels[t].begin(); it != levels[t].end(); ++it)
		{
			for (std::list<MDDNode*>::iterator parent = (*it)->parents.begin(); parent != (*it)->parents.end(); parent++)
			{
				if ((*parent)->children.empty()) // a new node
				{
					levels[t - 1].push_back(*parent);
				}
				(*parent)->children.push_back(*it); // add forward edge	
			}
		}
	}
	assert(!levels[0].empty());


    // Delete useless nodes (nodes who don't have any children)
	for (std::list<MDDNode*>::iterator it = closed.begin(); it != closed.end(); ++it)
		if ((*it)->children.empty() && (*it)->level < numOfLevels - 1){
//            auto node = std::find(levels[(*it)->level].begin(),levels[(*it)->level].end(),*it);
//            if(node != levels[(*it)->level].end()){
//                levels[(*it)->level].erase(node);
//            }
			delete (*it);
		}

	//in train pathfinding, agent may reach same location with di
	assert(done);

	if(done) {
        level_locs.resize(levels.size());
        for (int t = numOfLevels - 1; t >= 0; t--) {
            level_locs[t].resize(solver.kRobust + 1);
            assert(!levels[t].empty());
            for (std::list<MDDNode *>::iterator it = levels[t].begin(); it != levels[t].end(); ++it) {
                int position = 0;
                for (auto loc_it = (*it)->locs.begin(); loc_it != (*it)->locs.end(); loc_it++, position++) {
                    if (!level_locs[t][position].count(loc_it->location))
                        level_locs[t][position].insert(loc_it->location);
                }

            }
        }
    }

	return done;
}

template<class Map>
bool MDD<Map>::getOccupations(list<Location> & next_locs, Location next_id, MDDNode* curr, int k){
    next_locs.push_back(next_id);
    auto parent = curr;
    Location & pre_loc = next_id;
    bool conf_free = true;
    while( next_locs.size() <= k){
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
//    for (auto i : next_locs){
//        cout<<i<<",";
//    }
//    cout<<endl;
    return conf_free;
}


template<class Map>
void MDD<Map>::deleteNode(MDDNode* node)
{
	levels[node->level].remove(node);
	for (std::list<MDDNode*>::iterator child = node->children.begin(); child != node->children.end(); ++child)
	{
		(*child)->parents.remove(node);
		if((*child)->parents.empty())
			deleteNode(*child);
	}
	for (std::list<MDDNode*>::iterator parent = node->parents.begin(); parent != node->parents.end(); ++parent)
	{
		(*parent)->children.remove(node);
		if ((*parent)->children.empty())
			deleteNode(*parent);
	}
}

template<class Map>
void MDD<Map>::clear()
{
	if(levels.empty())
		return;
	for (int i = 0; i < levels.size(); i++)
	{

		for (std::list<MDDNode*>::iterator it = levels[i].begin(); it != levels[i].end(); ++it)
        {

                if (*it != nullptr) {
                    delete (*it);
                }

        }
	}
}

template<class Map>
MDDNode* MDD<Map>::find(list<int> locs, int level)
{
	if(level < levels.size())
		for (std::list<MDDNode*>::iterator it = levels[level].begin(); it != levels[level].end(); ++it) {
			if( equal_occupation(locs, locs))
				return (*it);
		}
	return NULL;
}

template<class Map>
MDD<Map>::MDD(MDD & cpy) // deep copy
{
	levels.resize(cpy.levels.size());
	MDDNode* root = new MDDNode(cpy.levels[0].front()->locs, NULL,cpy.levels[0].front()->train);
	levels[0].push_back(root);
	for(int t = 0; t < levels.size() - 1; t++)
	{
		for (std::list<MDDNode*>::iterator node = levels[t].begin(); node != levels[t].end(); ++node)
		{
			std::list<int> node_locs;
			for (Location & loc: (*node)->locs)
				node_locs.push_back(loc.location);
			MDDNode* cpyNode = cpy.find(node_locs, (*node)->level);
			for (std::list<MDDNode*>::const_iterator cpyChild = cpyNode->children.begin(); cpyChild != cpyNode->children.end(); ++cpyChild)
			{
				std::list<int> child_locs;
				for (Location & loc: (*cpyChild)->locs)
					child_locs.push_back(loc.location);
				MDDNode* child = find(child_locs, (*cpyChild)->level);
				if (child == NULL)
				{
					child = new MDDNode((*cpyChild)->locs, (*node),(*cpyChild)->train);
					levels[child->level].push_back(child);
					(*node)->children.push_back(child);
				}
				else
				{
					child->parents.push_back(*node);
					(*node)->children.push_back(child);
				}
			}
		}
		
	}
}

template<class Map>
MDD<Map>::~MDD()
{
	clear();
}

template class MDD<MapLoader>;


