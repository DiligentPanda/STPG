#include "generate_ADG.h"

// Return path and stateCnt of an agent
tuple<Path, int> parse_path(string line) {
  int i, j, stateCnt = 0;
  size_t comma_pos, leftPar_pos, rightPar_pos;
  Path path;

  while ((leftPar_pos = line.find("(")) != string::npos) {
    // Process an index pair
    comma_pos = line.find(",");
    i = stoi(line.substr(leftPar_pos + 1, comma_pos));
    rightPar_pos = line.find(")");
    j = stoi(line.substr(comma_pos + 1, rightPar_pos));
    line.erase(0, leftPar_pos + 1);
    stateCnt ++;

    // Create a location tuple and add it to the path
    tuple<int, int> index = make_tuple(i, j);
    path.push_back(index);
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

int compute_vertex(vector<int> accum_stateCnts, int agent, int state) {
  return 0;
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
        set_edge(graph, prev_vertex, curr_vertex);
      }
    }
  }
}

void add_type2_edges(Graph graph, Paths paths, vector<int> accum_stateCnts) {

}

Graph construct_ADG(char* fileName) {
  Paths paths, vector<int> accum_stateCnts;
  tie(paths, accum_stateCnts) = parse_soln(fileName);
  int sumStates = accum_stateCnts.back();

  Graph graph = new_graph(sumStates, sumStates);
  add_type1_edges(graph, paths, accum_stateCnts);
  add_type2_edges(graph, paths, accum_stateCnts);
}