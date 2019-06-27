from ortools.sat.python import cp_model
import networkx as nx
import numpy as np
from tqdm import tqdm

def find_shortest_path(pc):
  nodes = [n for n in range(12) if (pc >> n) & 1]
  if len(nodes) <= 1:
    return 0
  if len(nodes) == 2:
    return len(ALL_SHORTEST_PATHS[nodes[0]][nodes[1]]) - 1

  model = cp_model.CpModel()
  N = len(nodes)

  W = np.array([model.NewIntVar(0,12,"w") for i in range(N-1)])
  V = np.array([[model.NewBoolVar("v") for j in range(N)] for i in range(N)])

  for row in V:
    model.Add(row.sum() == 1)

  for col in V.T:
    model.Add(col.sum() == 1)

  for i in range(N-1):
    for j in range(N):
      for k in range(N):
        w = (len(ALL_SHORTEST_PATHS[nodes[j]][nodes[k]]) - 1)
        both = model.NewBoolVar("tmp")
        model.AddMinEquality(both, [V[i,j], V[i+1,k]])
        model.Add(W[i] == w).OnlyEnforceIf(both)

  s = model.NewIntVar(0,12,"sum")
  model.Add(W.sum() == s)
  model.Minimize(s)

  solver = cp_model.CpSolver()
  solver.Solve(model)
  path_length = solver.Value(s)
  assert path_length >= (len(nodes) - 1)
  return path_length

def get_pcd(x, n=12):
	power = (2**np.arange(n))
	r = np.array([np.roll((x & power).astype(np.bool), j) for j in range(n)])
	return np.min((r * power[None,:]).sum(1))

if __name__ == "__main__":

  # construct the tonnetz
  G = nx.Graph()
  for i in range(12):
    G.add_edge(i % 12, (i + 7) % 12)
    G.add_edge(i % 12, (i + 3) % 12)
    G.add_edge(i % 12, (i + 4) % 12)

  # find all shortest paths between node pairs
  ALL_SHORTEST_PATHS = {k:v for k,v in nx.all_pairs_shortest_path(G)}

  pc = np.arange(4096)
  pcd = np.array([get_pcd(_) for _ in pc])
  uniq, inv = np.unique(pcd, return_inverse=True)

  path_lengths = []
  for i in tqdm(uniq):
    path_lengths.append( find_shortest_path(i) )

  path_lengths = np.array([path_lengths[i] for i in inv])
  np.savez("tonnetz_path_lengths.npz", data=path_lengths)



