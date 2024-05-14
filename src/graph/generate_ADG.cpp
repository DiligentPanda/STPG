#include "graph/generate_ADG.h"
#include "util/Timer.h"

bool same_locations(Location location1, Location location2) {
  int i1 = location1.first;
  int j1 = location1.second;
  int i2 = location2.first;
  int j2 = location2.second;
  
  return (i1 == i2 && j1 == j2);
}

// Return path and stateCnt of an agent
tuple<Path, int> parse_path(string line) {
  int i, j, stateCnt = 0;
  int time = 0;
  size_t comma_pos, leftPar_pos, rightPar_pos;
  Path path;
  Location prev_location = make_pair(-1, -1);

  while ((leftPar_pos = line.find("(")) != string::npos) {
    // Process an index pair
    comma_pos = line.find(",");
    i = stoi(line.substr(leftPar_pos + 1, comma_pos));
    rightPar_pos = line.find(")");
    j = stoi(line.substr(comma_pos + 1, rightPar_pos));
    line.erase(0, rightPar_pos + 1);

    // Create a location tuple and add it to the path
    Location location = make_pair(i, j);
    if (!same_locations(location, prev_location)) {
      stateCnt ++;
      path.push_back(make_pair(location, time));
      prev_location = location;
    }
    time++;
  }
  return make_tuple(path, stateCnt);
}

// Return all paths, accumulated counts of states, and States
shared_ptr<Paths> parse_soln(const char* fileName) {
  auto paths_ptr=make_shared<Paths>();
  auto & paths = * paths_ptr;

  string fileName_string = fileName;
  ifstream file(fileName_string);
  if (file.is_open()) {
    string line;
    while (getline(file, line)) {
      // Sanity check that the line is a path
      if (line[0] == 'A') {
        tuple<Path, int> parse_result = parse_path(line);
        Path path = get<0>(parse_result);
        // Done with the agent
        paths.push_back(path);
      }
    }
    file.close();
  } else {
    std::cout << "exit\n";
    exit(0);
  }
  return paths_ptr;
}

// duplication=true means allowing consecutive vertices in a path to be duplicate.
shared_ptr<Graph> construct_ADG(const shared_ptr<Paths> &paths, bool duplication) {
  if (duplication) {
    std::cout<<"not supported duplication now"<<std::endl;
    exit(-1);
  }

  auto graph = make_shared<Graph>(paths);
  return graph;
}

shared_ptr<Graph> construct_ADG(const char* fileName) {
  auto paths = parse_soln(fileName);
  return construct_ADG(paths, false);
}

shared_ptr<Graph> construct_delayed_ADG(const shared_ptr<Graph> & graph, const vector<int> & delay_steps, const vector<int> & states) {
  auto new_paths=make_shared<Paths>();

  int delay_sum = 0;

  for (int agent = 0; agent < graph->get_num_agents(); ++agent) {
    if (delay_steps[agent] == 0) {
      new_paths->push_back((*graph->paths)[agent]);
    } else { // this is a delayed agent
      Path &ori_path = (*graph->paths)[agent];
      Path new_path;
      int delayed_state = states[agent];

      for (int state = 0; state <= delayed_state; state ++) {
        new_path.push_back(ori_path[state]);
      }
      // insert repeated current states for a multi-step delay.
      // <Location, timestep>
      // NOTE(rivers): it is not a bug if we don't specify the timestep here and change the timestep later.
      // because now we still want to stick to the original plan.
      pair<Location, int> repeat = make_pair(get<0>(new_path.back()), -1);
      int delay = delay_steps[agent];
      delay_sum += delay;
      for (int state = 0; state < delay; state ++) {
        new_path.push_back(repeat);
      }
      int ori_size = ori_path.size();
      for (int state = delayed_state + 1; state < ori_size; state ++) {
        new_path.push_back(ori_path[state]);
      }
      new_paths->push_back(new_path);
    }
  }

  // g_timer.record_p("make_graph_s");
  auto new_graph=make_shared<Graph>(new_paths, states);
  // g_timer.record_d("make_graph_s","make_graph");

  // g_timer.print_all_d();

  return new_graph;
}