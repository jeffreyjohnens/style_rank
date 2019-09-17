import os
import csv
import numpy as np
import warnings
from scipy.stats import rankdata
from subprocess import call
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics.pairwise import cosine_distances
from sklearn.preprocessing import OneHotEncoder

# import c++ code
from ._style_rank import get_features_internal, get_feature_names

def validate_labels(labels):
  """ensure that labels are well formed.
  Args:
    labels (list): a list of integers on the range [0,1]
  """
  if (labels==0).sum() == 0:
    raise Exception("All to_rank_paths were corrupt")
  if (labels==1).sum() == 0:
    raise Exception("All corpus_paths were corrupt")

def validate_features(paths, labels, features):
  """ensure that the features are well formed with respect to the paths and labels.
  Args:
    paths (list): a list of midi filepaths
    labels (list): a list of integers on the range [0,1]
    features (dict): a dictionary containing one or more features
  """
  if not type(features) == dict:
    raise TypeError('Provided features are not a dictionary')
  if len(features) == 0:
    raise Exception('Provided features are empty')
  is_numpy = np.all([type(v) == np.ndarray for v in features.values()])
  is_size = np.all([len(v) == len(paths) for v in features.values()])
  if not is_numpy or not is_size:
    raise Exception('Each feature must be a matrix of size (len(paths),d)')

def validate_paths(paths, list_name=None):
  """ensure that paths are well formed.
  Args:
    paths (list): a list of midi filepaths
    list_name (str): an identifier for the paths list
  """
  valid_paths = []
  indices = []
  for i,path in enumerate(np.atleast_1d(paths)):
    if not os.path.exists(path):
      warnings.warn('{} does not exist.'.format(path))
    elif not path.endswith(".mid"):
      warnings.warn('{} if not a MIDI file (.mid).'.format(path))
    else:
      valid_paths.append(path)
      indices.append(i)
  if len(valid_paths) == 0:
    if list_name is not None:
      raise Exception('No valid filepaths provided in {}'.format(list_name))
    raise Exception('No valid filepaths provided')
  return np.array(valid_paths), np.array(indices)

def get_features(paths, upper_bound=500, feature_names=[]):
  """extract features for a list of midis
  Args:
    paths (list): a list of midi filepaths.
    upper_bound (int): the maximum cardinality of each categorical distribution.
    feature_names (list): a list of features to extract
  Returns:
    fs (dict): a dictionary of categorical distributions (np.ndarray) indexed by feature name.
    domains (dict): a dictionary of categorical domains (np.ndarray) indexed by feature name.
    path_indices (np.ndarray): an integer array indexing the filepaths from which features were sucessfully extracted.
  """
  paths, path_indices = validate_paths(paths)
  feature_names = [f for f in feature_names if f in get_feature_names()]
  (fs, domains, indices) = get_features_internal(paths, feature_names, upper_bound)
  fs = {k : np.array(v).reshape(-1,len(domains[k])+1) for k,v in fs.items()}
  domains = {k : np.array(v) for k,v in domains.items()}
  path_indices = path_indices[np.array(indices)]
  return fs, domains, path_indices

def get_feature_csv(paths, output_dir, upper_bound=500, feature_names=[]):
  """extract features for a list of midis and output to csv's
  Args:
    paths (list): a list of midi filepaths.
    output_dir (str): a directory to store the feature .csv's
    upper_bound (int): the maximum cardinality of each categorical distribution.
    feature_names (list): a list of features to extract
  """
  data, domains, indices = get_features(
    paths, upper_bound=upper_bound, feature_names=feature_names)
  call(["mkdir", "-p", output_dir])
  for k,v in data.items():
    with open(os.path.join(output_dir, k) + ".csv", "w") as f:
      w = csv.writer(f)
      w.writerow(["filepath"] + list(domains[k]) + ["remain"])
      for path, vv in zip(np.array(paths)[indices], v):
        w.writerow([path] + list(vv))

def rf_embed(feature, labels, n_estimators=100, max_depth=3):
  """construct an embedding using a random forest
  Args:
    feature (np.ndarray): a matrix of shape (len(labels),D) with D>0.
    labels (list): a list of integers on the range [0,1]
    n_estimators (int): the number of trees in the random forest
    max_depth (int): the maximum depth of each tree
  Returns:
    dist_mat (np.ndarray): a matrix containg all pairwise distances for a single categorical distribution (feature).
  """
  clf = RandomForestClassifier(n_estimators=n_estimators, max_depth=max_depth, bootstrap=True, criterion='entropy', class_weight='balanced')
  clf.fit(feature, labels)
  leaves = clf.apply(feature)
  embedded = np.array(
    OneHotEncoder(categories='auto').fit_transform(leaves).todense())
  return cosine_distances(embedded)

def get_distance_matrix(to_rank_paths, corpus_paths, features=None, upper_bound=500, n_estimators=100, max_depth=3, return_paths_and_labels=False):
  """construct a distance matrix
  Args:
    to_rank_paths (list/np.ndarray): a list/array of midis to be ranked.
    corpus_paths (list/np.ndarray): a list/array of midis to define the style.
    features (dict): a dictionary of categorical distributions (np.ndarray) indexed by feature name.
    upper_bound (int): the maximum cardinality of each categorical distribution.
    n_estimators (int): the number of trees in the random forest.
    max_depth (int): the maximum depth of each tree.
    return_paths_and_labels (int): a boolean flag indicating whether these items should be returned or not
  Returns:
    dist_mat (np.ndarray): a matrix containg all pairwise distances.
    paths (np.ndarray) : an array of midi filepaths corresponding to each row/col in the distance matrix.
    labels (np.ndarray): an array of labels corresponding to each row/col in the distance matrix.
  """
  # create paths and labels
  to_rank_paths,_ = validate_paths(to_rank_paths, list_name="to_rank_paths")
  corpus_paths,_ = validate_paths(corpus_paths, list_name="corpus_paths")
  paths = np.hstack([to_rank_paths, corpus_paths])
  labels = np.array([0] * len(to_rank_paths) + [1] * len(corpus_paths))

  # extract features
  if features is None:
    features, _, indices = get_features(paths, upper_bound=upper_bound)
    labels = labels[indices]
  else:
    validate_features(paths, labels, features)
    indices = np.arange(len(paths))
  
  # ensure corpus_paths and to_rank_paths were parsed
  validate_labels(labels)

  # create embedding via trained random forests
  dist_mat = []
  for _,feature in features.items():
    dist_mat.append( 
      rf_embed(feature, labels, n_estimators=n_estimators, max_depth=max_depth))
  dist_mat = np.mean(np.array(dist_mat),axis=0)

  if return_paths_and_labels:
    return dist_mat, paths[indices], labels
  return dist_mat

def rank(to_rank_paths, corpus_paths, features=None, upper_bound=500, n_estimators=100, max_depth=3):
  """construct a distance matrix
  Args:
    to_rank_paths (list/np.ndarray): a list/array of midis to be ranked.
    corpus_paths (list/np.ndarray): a list/array of midis to define the style.
    features (dict): a dictionary of categorical distributions (np.ndarray) indexed by feature name.
    upper_bound (int): the maximum cardinality of each categorical distribution.
    n_estimators (int): the number of trees in the random forest.
    max_depth (int): the maximum depth of each tree.
  Returns:
    paths (np.ndarray): an array containing the to_rank_paths sorted from most to least stylistically similar to the corpus.
  """
  dmat,paths,labels = get_distance_matrix(to_rank_paths, corpus_paths, upper_bound=upper_bound, n_estimators=n_estimators, max_depth=max_depth, return_paths_and_labels=True, features=features)
  dists = dmat[labels==0][:,labels==1].sum(1)
  return paths[np.argsort(dists)]