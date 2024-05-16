import mip
from mip import BINARY, CONTINUOUS, INF, MINIMIZE, minimize, xsum
from mip.entities import Var
import time
from typing import Dict, List, Tuple

class MILPSolver:
    def __init__(self, time_limit=90, eps=0, verbose=False) -> None:
        self.time_limit=time_limit  # time limit in seconds
        self.EPSILON=eps
        self.verbose=verbose
        
    def __str__(self):
        return "MILPSolver(time_limit={}, eps={})".format(self.time_limit, self.EPSILON)
    
    def test(self):
        print(self)
        
    def print(self,*args,**kwargs):
        if not self.verbose:
            return
        print(*args,**kwargs)
        
    def solve(
        self, 
        paths: List[List[int]],
        non_switchable_edges: List[Tuple[int,int,int,int, float]],
        switchable_edge_groups: Dict[int, List[Tuple[int, int, int, int, float]]]
    ) -> Tuple[int, float, float, List[int]]:
        curr_states=[path[0] for path in paths]
        
        # TODO(rivers): don't have any idea why M set to a too large constant, e.g. 10^8, the result is wrong. you can try it with the example
        # so we will estimate it in the solve.
        # or you can set to <=10000000, which is fine at least for the example
        # NOTE(rivers): M influence time a lot.
        # a better way is to set M separately for each switchable edge, so that M=max over two agents' path length.
        # this is also one drawback of this algorithm.
        # sum is a worst estimation for now if we don't consider the future delay.
        # we add the cost in the loop for non_switchable_edges
        # TODO(rivers): M should be larger than optimal_cost+max_edge_cost?
        M = 1000000 
        raise NotImplementedError("We should set M to a better value before experiments.")
        
        # create MILP dictorary
        # TODO(rivers): we should try the commercial Gurobi solver later, which should be faster than CBC
        m_opt = mip.Model(name="SwitchableTPG",sense=MINIMIZE, solver_name=mip.CBC)
        m_opt.clear()
        
        for agent_id, state_ids in enumerate(paths):
            for state_id in state_ids:
                m_opt.add_var(
                    name=f"state_{agent_id}_{state_id}",
                    var_type=CONTINUOUS,
                    lb=-INF
                )
        
        for edge_id, edge in enumerate(non_switchable_edges):
            head_agent_id, head_state_id, tail_agent_id, tail_state_id, cost = edge
            
            if head_state_id==curr_states[head_agent_id] and tail_state_id==curr_states[tail_agent_id]:
                print(edge)
            
            var_1 = m_opt.var_by_name(f"state_{head_agent_id}_{head_state_id}")
            var_2 = m_opt.var_by_name(f"state_{tail_agent_id}_{tail_state_id}")
            
            m_opt.add_constr(
                lin_expr=var_2 - var_1 >= cost + self.EPSILON,
                name=f"non_switchable_{edge_id}"
            )
        
        for group_id, group in switchable_edge_groups.items():
            
            b: Var = m_opt.add_var(
                name=f"switchable_{group_id}",
                var_type=BINARY
            )
            
            for edge in group:
                head_agent_id, head_state_id, tail_agent_id, tail_state_id, cost = edge
                var_1 = m_opt.var_by_name(f"state_{head_agent_id}_{head_state_id}")
                var_2 = m_opt.var_by_name(f"state_{tail_agent_id}_{tail_state_id}")
                
                m_opt.add_constr(
                    lin_expr=var_2 - var_1 >= cost + self.EPSILON - b * M,
                    name=f"switchable_fwd_{group_id}_{edge}"
                )
                
                reverse_head_agent_id = tail_agent_id
                reverse_head_state_id = tail_state_id + 1
                reverse_tail_agent_id = head_agent_id
                reverse_tail_state_id = head_state_id - 1
                
                var_1 = m_opt.var_by_name(f"state_{reverse_head_agent_id}_{reverse_head_state_id}")
                var_2 = m_opt.var_by_name(f"state_{reverse_tail_agent_id}_{reverse_tail_state_id}")
                
                m_opt.add_constr(
                    lin_expr=var_2 - var_1 >= cost + self.EPSILON - (1 - b) * M,
                    name=f"switchable_rev_{group_id}_{edge}"
                )
        
        # Add boundary constraints
        for agent_id, state_id in enumerate(curr_states):
            var = m_opt.var_by_name(f"state_{agent_id}_{state_id}")
            m_opt.add_constr(
                lin_expr=var == 0,
                name=f"boundary_{agent_id}"
            )
        
        # Add objective function
        last_vars = []
        for agent_id, state_ids in enumerate(paths):
            last_state_id=state_ids[-1]
            var = m_opt.var_by_name(f"state_{agent_id}_{last_state_id}")
            last_vars.append(var)
        
        m_opt.verbose = 0
        m_opt.objective = minimize(xsum(last_vars))
        
        # providing init cost makes it slower...
        # m_opt += m_opt.objective<=init_cost
        
        # providing init sol also doesn't help...
        # init_sol=[]
        # for group_id, group in switchable_edge_groups.items():    
        #     b: Var = m_opt.var_by_name(
        #         name=f"switchable_{group_id}"
        #     )
        #     init_sol.append((b,0.0))
        # m_opt.start=init_sol
        
        # Print optimization problem
        self.print(m_opt.objective)
        for constr in m_opt.constrs:
            self.print(constr)

        # Run the optimization
        s_time = time.time()
        opt_status = m_opt.optimize(max_seconds=self.time_limit)
        elapse = time.time()-s_time
        
        # TODO, we should allow feasible but not optimal solution later, which can be compared with focal.
        print(opt_status)
        
        objective_value=-1
        if opt_status == mip.OptimizationStatus.OPTIMAL:
            status=0
            objective_value=m_opt.objective_value
            self.print("Optimal solution found")
        elif opt_status == mip.OptimizationStatus.FEASIBLE:
            status=1
            self.print("Feasible solution found")
        else:
            status=2
            self.print("No solution found")
        
        
        self.print(f"objective value: {objective_value}")
        
        self.print("Values of optimization variables")
        for var in m_opt.vars:
            self.print(f"{var.name}: {var.x}")
            
        self.print("--------------------------------")
        
        group_ids_to_reverse=[]
        # if no solution found, just return empty list
        if opt_status in [mip.OptimizationStatus.OPTIMAL, mip.OptimizationStatus.FEASIBLE]:
            for var in m_opt.vars:
                if var.name.startswith("switchable"):
                    group_id=int(var.name.split("_")[1])
                    if var.x==1.0:
                        group_ids_to_reverse.append(group_id)

        # TODO: we also need to return time
        return status, elapse, objective_value, group_ids_to_reverse

if __name__=="__main__":
    solver=MILPSolver()
    paths=[[0,1,2,3,4,5],[0,1,2,3]]
    curr_states=[0,0]
    non_switchable_edges=[
        (0,0,0,1),
        (0,1,0,2),
        (0,2,0,3),
        (0,3,0,4),
        (0,4,0,5),
        (1,0,1,1),
        (1,1,1,2),
        (1,2,1,3)
    ]
    switchable_edge_groups={
        0:[
            (0,4,1,1),
            (0,5,1,2),
            # (1,2,0,3),
            # (1,3,0,4)
        ]
    }
    ret=solver.solve(paths, curr_states, non_switchable_edges, switchable_edge_groups)
    print(ret)
