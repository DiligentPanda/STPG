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

// Return all paths, stateCnts, and sumStates
tuple<Paths, vector<int>, int> parse_soln(char* fileName) {
  Paths paths;
  vector<int> stateCnts;
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
        stateCnts.push_back(stateCnt);
        sumStates += stateCnt;
      }
    }
    file.close();
  }
  return make_tuple(paths, stateCnts, sumStates);
}
