# this is the main interface for all the functions ...
import os
import csv
import numpy as np
import warnings
from subprocess import call
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics.pairwise import cosine_distances
from sklearn.preprocessing import OneHotEncoder

# import c++ code
from .style_rank import get_features_internal, get_feature_names

def filter_valid_paths(paths):
  valid_paths = []
  for path in paths:
    if not os.path.exists(path):
      warnings.warn('{} does not exist.'.format(path))
    elif not path.endswith(".mid"):
      warnings.warn('{} if not a MIDI file (.mid).'.format(path))
    else:
      valid_paths.append(path)
  if len(valid_paths) == 0:
    raise Exception('No valid filepaths provided')
  return valid_paths

def get_features(paths, upper_bound=500, features=[]):
  paths = filter_valid_paths(paths) # filter paths
  features = [f for f in features if f in get_feature_names()] # filter names
  (data, domains, indices) = get_features_internal(paths, features, upper_bound)
  data = {k : np.array(v).reshape(-1,len(domains[k])+1) for k,v in data.items()}
  domains = {k : np.array(v) for k,v in domains.items()}
  return data, domains, np.array(indices)

def get_feature_csv(paths, output_dir, upper_bound=500, features=[]):
  data, domains, indices = get_features(
    paths, upper_bound=upper_bound, features=features)
  call(["mkdir", "-p", output_dir])
  for k,v in data.items():
    with open(os.path.join(output_dir, k) + ".csv", "w") as f:
      w = csv.writer(f)
      w.writerow(["filepath"] + list(domains[k]) + ["remain"])
      for path, vv in zip(np.array(paths)[indices], v):
        w.writerow([path] + list(vv))

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

