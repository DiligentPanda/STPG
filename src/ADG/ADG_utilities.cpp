#include "ADG_utilities.h"

int get_agentCnt(ADG adg) {
  Paths paths = get<1>(adg);
  return paths.size();
}

int get_stateCnt(ADG adg, int agent) {
  vector<int> accum_stateCnts = get<2>(adg);
  if (agent == 0) return accum_stateCnts[0];
  return accum_stateCnts[agent] - accum_stateCnts[agent-1];
}

int get_totalStateCnt(ADG adg) {
  vector<int> accum_stateCnts = get<2>(adg);
  return accum_stateCnts.back();
}

int compute_vertex(vector<int> accum_stateCnts, int agent, int state) {
  if (agent == 0) return state; // Accumulated state cnt == 0
  int accum_stateCnt = accum_stateCnts[agent - 1];
  return (state + accum_stateCnt);
}

pair<int, int> compute_agent_state(vector<int> accum_stateCnts, int v) {
  int agent = 0;
  int prevStateCnt = 0;
  for (int stateCnt: accum_stateCnts) {
    if (v <= stateCnt) return make_pair(agent, v - prevStateCnt);
    prevStateCnt = stateCnt;
    agent ++;
  }
}

bool is_type2_edge(ADG adg, int agent1, int state1, int agent2, int state2) {
  Graph graph = get<0>(adg);
  vector<int> accum_stateCnts = get<2>(adg);
  int v1 = compute_vertex(accum_stateCnts, agent1, state1);
  int v2 = compute_vertex(accum_stateCnts, agent2, state2);
  return (get_type2_switchable_edge(graph, v1, v2) || 
          get_type2_nonSwitchable_edge(graph, v1, v2));
}

void fix_type2_edge(ADG adg, int agent1, int state1, int agent2, int state2) {
  Graph graph = get<0>(adg);
  vector<int> accum_stateCnts = get<2>(adg);
  int v1 = compute_vertex(accum_stateCnts, agent1, state1);
  int v2 = compute_vertex(accum_stateCnts, agent2, state2);

  if (get_type2_switchable_edge(graph, v1, v2)) {
    rem_type2_switchable_edge(graph, v1, v2);
    set_type2_nonSwitchable_edge(graph, v1, v2);
  }
}

void fix_type2_edge_reversed(ADG adg, int agent1, int state1, int agent2, int state2) {
  Graph graph = get<0>(adg);
  vector<int> accum_stateCnts = get<2>(adg);
  int v1 = compute_vertex(accum_stateCnts, agent1, state1);
  int v2 = compute_vertex(accum_stateCnts, agent2, state2);

  if (get_type2_switchable_edge(graph, v1, v2)) {
    rem_type2_switchable_edge(graph, v1, v2);
    set_type2_nonSwitchable_edge(graph, v2, v1);
  }
}

vector<pair<int, int>> get_switchable_inNeibPair(ADG adg, int agent, int state) {
  Graph graph = get<0>(adg);
  vector<int> accum_stateCnts = get<2>(adg);
  int v = compute_vertex(accum_stateCnts, agent, state);
  vector<int> inNeighbors_vertex = get_switchable_inNeib(graph, v);

  vector<pair<int, int>> inNeighbors_pair;
  for (int vertex: inNeighbors_vertex) {
    inNeighbors_pair.push_back(compute_agent_state(accum_stateCnts, vertex));
  }
  return inNeighbors_pair;
}

vector<pair<int, int>> get_switchable_outNeibPair(ADG adg, int agent, int state) {
  Graph graph = get<0>(adg);
  vector<int> accum_stateCnts = get<2>(adg);
  int v = compute_vertex(accum_stateCnts, agent, state);
  vector<int> outNeighbors_vertex = get_switchable_outNeib(graph, v);

  vector<pair<int, int>> outNeighbors_pair;
  for (int vertex: outNeighbors_vertex) {
    outNeighbors_pair.push_back(compute_agent_state(accum_stateCnts, vertex));
  }
  return outNeighbors_pair;
}

vector<pair<int, int>> get_nonSwitchable_inNeibPair(ADG adg, int agent, int state) {
  Graph graph = get<0>(adg);
  vector<int> accum_stateCnts = get<2>(adg);
  int v = compute_vertex(accum_stateCnts, agent, state);
  vector<int> inNeighbors_vertex = get_nonSwitchable_inNeib(graph, v);

  vector<pair<int, int>> inNeighbors_pair;
  for (int vertex: inNeighbors_vertex) {
    inNeighbors_pair.push_back(compute_agent_state(accum_stateCnts, vertex));
  }
  return inNeighbors_pair;
}

vector<pair<int, int>> get_nonSwitchable_outNeibPair(ADG adg, int agent, int state) {
  Graph graph = get<0>(adg);
  vector<int> accum_stateCnts = get<2>(adg);
  int v = compute_vertex(accum_stateCnts, agent, state);
  vector<int> outNeighbors_vertex = get_nonSwitchable_outNeib(graph, v);

  vector<pair<int, int>> outNeighbors_pair;
  for (int vertex: outNeighbors_vertex) {
    outNeighbors_pair.push_back(compute_agent_state(accum_stateCnts, vertex));
  }
  return outNeighbors_pair;
}

vector<pair<int, int>> get_inNeibPair(ADG adg, int agent, int state) {
  Graph graph = get<0>(adg);
  vector<int> accum_stateCnts = get<2>(adg);
  int v = compute_vertex(accum_stateCnts, agent, state);
  vector<int> inNeighbors_vertex = get_inNeighbors(graph, v);

  vector<pair<int, int>> inNeighbors_pair;
  for (int vertex: inNeighbors_vertex) {
    inNeighbors_pair.push_back(compute_agent_state(accum_stateCnts, vertex));
  }
  return inNeighbors_pair;
}

vector<pair<int, int>> get_outNeibPair(ADG adg, int agent, int state) {
  Graph graph = get<0>(adg);
  vector<int> accum_stateCnts = get<2>(adg);
  int v = compute_vertex(accum_stateCnts, agent, state);
  vector<int> outNeighbors_vertex = get_outNeighbors(graph, v);

  vector<pair<int, int>> outNeighbors_pair;
  for (int vertex: outNeighbors_vertex) {
    outNeighbors_pair.push_back(compute_agent_state(accum_stateCnts, vertex));
  }
  return outNeighbors_pair;
}

Location get_state_target(ADG adg, int agent, int state) {
  Paths paths = get<1>(adg);
  Path path = paths[agent];
  return path[state];
}

ADG copy_ADG(ADG adg) {
  Graph graph = get<0>(adg);
  Paths paths = get<1>(adg);
  vector<int> accum_stateCnts = get<2>(adg);

  Graph newGraph = copy_graph(graph);
  Paths newPaths = paths;
  vector<int> newAccum_stateCnts = accum_stateCnts;
  return make_tuple(newGraph, newPaths, newAccum_stateCnts);
}