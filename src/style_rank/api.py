import os
import csv
import json
import numpy as np
import warnings
from scipy.stats import rankdata
from subprocess import call
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics.pairwise import cosine_distances
from sklearn.preprocessing import OneHotEncoder

# import c++ code
from ._style_rank import get_features_internal, get_feature_names_internal

def get_feature_names(tag="ORIGINAL"):
	return get_feature_names_internal(tag)

# checking arguments ...
def validate_argument(x, name):
	class domain:
		def __init__(self,lb=None,ub=None,dom=None):
			if lb is not None:
				assert lb <= ub
			self.lb=lb
			self.ub=ub
			self.dom=dom
		def check(self,x):
			if self.lb is not None:
				return self.lb <= x <= self.ub
			return x in self.dom
		def __repr__(self):
			if self.lb is not None:
				return "[%d,%d]" % (self.lb, self.ub)
			return str(self.dom)

	TOTAL_UPPER_BOUND = 100000
	recommend = {
		"upper_bound" : domain(lb=100, ub=500),
		"resolution" : domain(dom=[0,4,8,16]),
		"n_estimators" : domain(lb=50,ub=500),
		"max_depth" : domain(dom=[2,3]),
	}
	valid = {
		"upper_bound" : domain(lb=1, ub=TOTAL_UPPER_BOUND),
		"resolution" : domain(lb=0,ub=TOTAL_UPPER_BOUND),
		"n_estimators" : domain(lb=1,ub=TOTAL_UPPER_BOUND),
		"max_depth" : domain(lb=1,ub=TOTAL_UPPER_BOUND)
	}
	if not recommend[name].check(x):
		args = (name, str(x), str(recommend[name]))
		warnings.warn('%s=%s is outside of recommended range %s' % args)
	if not valid[name].check(x):
		args = (name, str(x), str(valid[name]))
		raise ValueError('%s=%s is outside valid range %s' % args)

def validate_labels(labels):
	"""ensure that labels are well formed.

	Args:

		labels (list): a list of integers on the range [0,1].
	"""
	if (labels==0).sum() == 0:
		raise Exception("All rank_set were corrupt")
	if (labels==1).sum() == 0:
		raise Exception("All style_set were corrupt")

def validate_features(paths, labels, features):
	"""ensure that the features are well formed with respect to the paths and labels.

	Args:

		paths (list): a list of midi filepaths.
		labels (list): a list of integers on the range [0,1].
		features (dict): a dictionary containing one or more features.
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

def get_features(paths, upper_bound=500, feature_names=[], resolution=0, include_offsets=False):
	"""extract features for a list of midis

	Args:
		paths (list): a list of midi filepaths.
		upper_bound (int): the maximum cardinality of each categorical distribution.
		feature_names (list): a list of features to extract
		resolution (int): the number of divisions per beat for the quantization of time-based values. If resolution=0, no quantization will take place.
		include_offsets (int): a boolean flag indicating if offsets will be considered for chord segment boundaries.

	Returns:
		fs (dict): a dictionary of categorical distributions (np.ndarray) indexed by feature name.
		domains (dict): a dictionary of categorical domains (np.ndarray) indexed by feature name.
		path_indices (np.ndarray): an integer array indexing the filepaths from which features were sucessfully extracted.
	"""
	validate_argument(upper_bound, "upper_bound")
	validate_argument(resolution, "resolution")

	paths, path_indices = validate_paths(paths)
	feature_names = [f for f in feature_names if f in get_feature_names("ALL")]
	(fs, domains, indices) = get_features_internal(paths, feature_names, upper_bound, resolution, include_offsets)
	fs = {k : np.array(v).reshape(-1,len(domains[k])+1) for k,v in fs.items()}
	domains = {k : np.array(v) for k,v in domains.items()}
	path_indices = path_indices[np.array(indices)]
	return fs, domains, path_indices

def get_feature_csv(paths, output_dir, upper_bound=500, feature_names=[], resolution=0, include_offsets=False):
	"""extract features for a list of midis and output to csv's

	Args:
		paths (list): a list of midi filepaths.
		output_dir (str): a directory to store the feature .csv's
		upper_bound (int): the maximum cardinality of each categorical distribution.
		feature_names (list): a list of features to extract.
		resolution (int): the number of divisions per beat for the quantization of time-based values. If resolution=0, no quantization will take place.
		include_offsets (int): a boolean flag indicating if offsets will be considered for chord segment boundaries.
	"""
	data, domains, indices = get_features(
		paths, upper_bound=upper_bound, feature_names=feature_names, resolution=resolution, include_offsets=include_offsets)
	call(["mkdir", "-p", output_dir])
	for k,v in data.items():
		with open(os.path.join(output_dir, k) + ".csv", "w") as f:
			w = csv.writer(f)
			w.writerow(["filepath"] + list(domains[k]) + ["remain"])
			for path, vv in zip(np.array(paths)[indices], v):
				w.writerow([path] + list(vv))

def rf_embed(feature, labels, n_estimators=100, max_depth=3):
	"""construct an embedding using a random forest.

	Args:
		feature (np.ndarray): a matrix of shape (len(labels),D) with D>0.
		labels (list): a list of integers on the range [0,1]
		n_estimators (int): the number of trees in the random forest
		max_depth (int): the maximum depth of each tree

	Returns:
		np.ndarray: a matrix containg all pairwise similarities for a single categorical distribution (feature).

	"""
	clf = RandomForestClassifier(n_estimators=n_estimators, max_depth=max_depth, bootstrap=True, criterion='entropy', class_weight='balanced')
	clf.fit(feature, labels)
	leaves = clf.apply(feature)
	embedded = np.array(
		OneHotEncoder(categories='auto').fit_transform(leaves).todense())
	return 1. - cosine_distances(embedded)

def get_similarity_matrix(rank_set, style_set, raw_features=None, upper_bound=500, n_estimators=100, max_depth=3, return_paths_and_labels=False, resolution=0, include_offsets=False, feature_names=[]):
	"""construct a similarity matrix

	Args:
		rank_set (list/np.ndarray): a list/array of midis to be ranked.
		style_set (list/np.ndarray): a list/array of midis to define the style.
		raw_features (dict): a dictionary of categorical distributions (np.ndarray) indexed by feature name.
		upper_bound (int): the maximum cardinality of each categorical distribution.
		n_estimators (int): the number of trees in the random forest.
		max_depth (int): the maximum depth of each tree.
		return_paths_and_labels (int): a boolean flag indicating whether these items should be returned or not
		resolution (int): the number of divisions per beat for the quantization of time-based values. If resolution=0, no quantization will take place.
		include_offsets (int): a boolean flag indicating if offsets will be considered for chord segment boundaries.
		feature_names (list): a list of features to extract. if feature_names=[] all features will be used.

	Returns:
		sim_mat (np.ndarray): a matrix containg all pairwise similarities.
		paths (np.ndarray) : an array of midi filepaths corresponding to each row/col in the similarity matrix.
		labels (np.ndarray): an array of labels corresponding to each row/col in the similarity matrix.
	"""
	validate_argument(n_estimators, "n_estimators")
	validate_argument(max_depth, "max_depth")

	# create paths and labels
	rank_set,_ = validate_paths(rank_set, list_name="rank_set")
	style_set,_ = validate_paths(style_set, list_name="style_set")
	paths = np.hstack([rank_set, style_set])
	labels = np.array([0] * len(rank_set) + [1] * len(style_set))

	# extract features
	if raw_features is None:
		features, _, indices = get_features(paths, upper_bound=upper_bound, resolution=resolution, include_offsets=include_offsets, feature_names=feature_names)
		labels = labels[indices]
	else:
		validate_features(paths, labels, raw_features)
		indices = np.arange(len(paths))
		features = raw_features
	
	# ensure style_set and rank_set were parsed
	validate_labels(labels)

	# create embedding via trained random forests
	sim_mat = np.zeros((len(labels), len(labels)))
	for _, feature in features.items():
		sim_mat += rf_embed(feature, labels, n_estimators=n_estimators, max_depth=max_depth)
	sim_mat /= len(features)

	if return_paths_and_labels:
		return sim_mat, paths[indices], labels
	return sim_mat

def rank(rank_set, style_set, raw_features=None, upper_bound=500, n_estimators=100, max_depth=3, return_similarity=False, resolution=0, include_offsets=False, feature_names=[], json_path=None):
	"""construct a similarity matrix

	Args:
		rank_set (list/np.ndarray): a list/array of midis to be ranked.
		style_set (list/np.ndarray): a list/array of midis to define the style.
		features (dict): a dictionary of categorical distributions (np.ndarray) indexed by feature name.
		upper_bound (int): the maximum cardinality of each categorical distribution.
		n_estimators (int): the number of trees in the random forest.
		max_depth (int): the maximum depth of each tree.
		return_similarity (bool) : return the cosine similarity to the corpus for each ranked MIDI.
		resolution (int): the number of divisions per beat for the quantization of time-based values. If resolution=0, no quantization will take place.
		include_offsets (int): a boolean flag indicating if offsets will be considered for chord segment boundaries.
		feature_names (list): a list of features to extract. if feature_names=[] all features will be used.
		json_path (str): if not None, the ranks will be written to a .json file.

	Returns:
		paths (np.ndarray): an array containing the rank_set sorted from most to least stylistically similar to the corpus.
	"""
	sim_mat,paths,labels = get_similarity_matrix(rank_set, style_set, upper_bound=upper_bound, n_estimators=n_estimators, max_depth=max_depth, return_paths_and_labels=True, raw_features=raw_features, resolution=resolution, include_offsets=include_offsets, feature_names=feature_names)
	sims = sim_mat[labels==0][:,labels==1].sum(1)
	order = np.argsort(sims)[::-1]
	output = list(zip(paths[order], sims[order]))
	if json_path is not None:
		with open(json_path, "w") as f:
			f.write(json.dumps(
				{str(p):float(d) for p,d in sorted(output,key=lambda x:x[1])}, 
				indent=4))
	if return_similarity:
		return output
	return paths[order]
