# this is the main interface for all the functions ...
import numpy as np
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics.pairwise import cosine_distances
from sklearn.preprocessing import OneHotEncoder

# import c++ code
from .style_rank import get_features_internal

def get_features(paths, upper_bound=500):
  (data, domains, indices) = get_features_internal(paths, upper_bound)
  data = {k : np.array(v).reshape(-1,len(domains[k])+1) for k,v in data.items()}
  domains = {k : np.array(v) for k,v in domains.items()}
  return data, domains, np.array(indices)

def get_distance_matrix(paths, labels, data=None, upper_bound=500):
  assert len(paths) == len(labels)
  if data is None:
    data, _, indices = get_features(paths, upper_bound=upper_bound)
    labels = labels[indices]
  else:
    indices = np.arange(len(paths))
  dist_mat = []
  for k,v in data.items():
    clf = RandomForestClassifier(n_estimators=500, max_depth=5, bootstrap=True, criterion='entropy', class_weight='balanced')
    clf.fit(v, labels)
    leaves = clf.apply(v)
    embedded = np.array(
      OneHotEncoder(categories='auto').fit_transform(leaves).todense())
    dist_mat.append( cosine_distances(embedded) )
  return np.mean(np.array(dist_mat),axis=0), indices

