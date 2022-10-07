#include "ADG_utilities.h"

int get_agentCnt(ADG adg) {
  Paths paths = get<1>(adg);
  paths.size();
}

int get_stateCnt(ADG adg, int agent) {
  vector<int> accum_stateCnts = get<2>(adg);
  if (agent == 0) return accum_stateCnts[0];
  return accum_stateCnts[agent] - accum_stateCnts[agent-1];
}

int compute_vertex(vector<int> accum_stateCnts, int agent, int state) {
  if (agent == 0) return state; // Accumulated state cnt == 0
  int accum_stateCnt = accum_stateCnts[agent - 1];
  return (state + accum_stateCnt);
}

bool is_type2_edge(ADG adg, int agent1, int state1, int agent2, int state2) {
  Graph graph = get<0>(adg);
  vector<int> accum_stateCnts = get<2>(adg);
  int n1 = compute_vertex(accum_stateCnts, agent1, state1);
  int n2 = compute_vertex(accum_stateCnts, agent2, state2);
  return (get_edge(graph, n1, n2) == TYPE2_EDGE);
}

void switch_type2_edge(ADG adg, int agent1, int state1, int agent2, int state2) {
  Graph graph = get<0>(adg);
  vector<int> accum_stateCnts = get<2>(adg);
  int n1 = compute_vertex(accum_stateCnts, agent1, state1);
  int n2 = compute_vertex(accum_stateCnts, agent2, state2);

  if (get_edge(graph, n1, n2) == TYPE2_EDGE) {
    rem_edge(graph, n1, n2);
    set_edge(graph, n1, n2, TYPE2_EDGE);
  }
}

vector<int, int> get_type2_inNeighbors(ADG adg, int agent, int state) {

}

vector<int, int> get_type2_outNeighbors(ADG adg, int agent, int state) {
  
}

Location get_state_target(ADG adg, int agent, int state) {

}