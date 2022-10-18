#include "generate_ADG.h"

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
      path.push_back(location);
      prev_location = location;
    }
  }
  return make_tuple(path, stateCnt);
}

// Return all paths, accumulated counts of states, and States
tuple<Paths, vector<int>> parse_soln(char* fileName) {
  Paths paths;
  vector<int> accum_stateCnts;
  int sumStates = 0;

  string fileName_string = fileName;
  ifstream file(fileName_string);
  if (file.is_open()) {
    string line;
    while (getline(file, line)) {
      // Sanity check that the line is a path
      if (line[0] == 'A') {
        tuple<Path, int> parse_result = parse_path(line);
        Path path = get<0>(parse_result);
        int stateCnt = get<1>(parse_result);

        // Done with the agent
        paths.push_back(path);
        sumStates += stateCnt;
        accum_stateCnts.push_back(sumStates);
      }
    }
    file.close();
  }
  return make_tuple(paths, accum_stateCnts);
}

void add_type1_edges(Graph graph, Paths paths, vector<int> accum_stateCnts) {
  int agentCnt = paths.size();
  for (int agent = 0; agent < agentCnt; agent++) {
    Path path = paths[agent];
    int stateCnt = path.size();
    int prev_vertex = -1;
    
    for (int state = 0; state < stateCnt; state++) {
      int curr_vertex = compute_vertex(accum_stateCnts, agent, state);
      if (prev_vertex >= 0) {
        set_type1_edge(graph, prev_vertex, curr_vertex);
      }
    }
  }
}

void add_type2_edges(Graph graph, Paths paths, vector<int> accum_stateCnts) {
  int agentCnt = paths.size();
  // Looping through agents
  for (int agent1 = 0; agent1 < agentCnt; agent1++) {
    Path path1 = paths[agent1];
    int stateCnt1 = path1.size();
    for (int agent2 = agent1 + 1; agent2 < agentCnt; agent2 ++) {
      Path path2 = paths[agent2];
      int stateCnt2 = path2.size();

      // Looping through states
      for (int state1 = 0; state1 < stateCnt1; state1++) {
        Location location1 = path1[state1];
        for (int state2 = 0; state2 < stateCnt2; state2++) {
          Location location2 = path2[state2];

          if (same_locations(location1, location2)) {
            // Add a type2 edge
            int vertex1 = compute_vertex(accum_stateCnts, agent1, state1);
            int vertex2 = compute_vertex(accum_stateCnts, agent2, state2);
            if (state1 < state2) set_type2_switchable_edge(graph, vertex1, vertex2);
            else set_type2_switchable_edge(graph, vertex2, vertex1);
          }
        }
      }
    }
  }
}

ADG construct_ADG(char* fileName) {
  Paths paths;
  vector<int> accum_stateCnts;
  tie(paths, accum_stateCnts) = parse_soln(fileName);
  int sumStates = accum_stateCnts.back();

  Graph graph = new_graph(sumStates);
  add_type1_edges(graph, paths, accum_stateCnts);
  add_type2_edges(graph, paths, accum_stateCnts);

  return make_tuple(graph, paths, accum_stateCnts);
}

// For testing purpose
int main(int argc, char** argv) {
  char* fileName = argv[1];
  Paths paths;
  vector<int> accum_stateCnts;
  tie(paths, accum_stateCnts) = parse_soln(fileName);
  
  for (Path path: paths) {
    std::cout << "Agent: ";
    for (Location location: path) {
      std::cout << location.first << ' ' << location.second << "=>";
    }
    std::cout << "\n";
  }

  for (int stateCnt: accum_stateCnts) {
    std::cout << stateCnt << ' ';
  }
  ADG adg = construct_ADG(fileName);
  std::cout << "agent cnt: " << get_agentCnt(adg) << "\n";
  std::cout << "state cnt: " << get_stateCnt(adg, 0) << "\n";
}