import os
import json
import tempfile
import warnings
import collections
import numpy as np
import pandas as pd
from subprocess import call
from itertools import zip_longest, product

import style_rank as sr

import unittest
from parameterized import parameterized

curdir = os.getcwd()
os.chdir(os.path.dirname(os.path.realpath(__file__)))

midi_paths = ["bwv2.6.mid", "bwv3.6.mid"]
feature_names = sr.get_feature_names("ALL")
features_name_subsets = [list(np.random.choice(feature_names, size=(5,), replace=False)) for i in range(3)]
temp_name = next(tempfile._get_candidate_names())

params = collections.OrderedDict([
    ("paths", [midi_paths]),
    ("output_dir", [temp_name]),
    ("upper_bound", [100]),
    ("feature_names", features_name_subsets + [[]] + [feature_names]),
    ("resolution", [0,8]),
    ("include_offsets", [False,True]),
    ("rank_set", [midi_paths]),
    ("style_set", [midi_paths[::-1]]),
    ("n_estimators", [10]),
    ("max_depth", [2]),
    ("return_paths_and_labels", [False,True]),
    ("return_similarity", [False,True])
  ])

def build_param_sets(arg_list, kwarg_list, name):
  used_params = collections.OrderedDict([(k,v) for k,v in params.items() if k in arg_list or k in kwarg_list])
  param_sets = [dict(zip(used_params.keys(),x)) for x in list(product(*used_params.values()))]
  return [[name + "_" + str(p), [p[k] for k in arg_list], {k:p[k] for k in kwarg_list}] for p in param_sets]

class TestGetFeatureNames(unittest.TestCase):
  @parameterized.expand([
      ["test ALL tag", "ALL"],
      ["test ORIGINAL tag", "ORIGINAL"],
      ["test MIREX tag", "MIREX"],
      ["test NULL tag", "NULL"],
      ["test empty tag", ""],
  ])
  def test_sequence(self, name, *args):
    self.assertTrue(type(sr.get_feature_names(*args))==list)

class TestGetFeatures(unittest.TestCase):
  @parameterized.expand(build_param_sets(["paths"], ["upper_bound", "feature_names", "resolution", "include_offsets"], "get_features"))
  def test_sequence(self, name, args, kwargs):
      output,_,_ = sr.get_features(*args,**kwargs)
      if len(kwargs["feature_names"]) > 0:
        # check that the features match
        self.assertSetEqual(set(list(output.keys())),set(kwargs["feature_names"]))
      
      # check that features are all nonzero
      for k in kwargs["feature_names"]:
        self.assertTrue(np.all(output[k].sum(1)) > 0, k)

class TestGetFeatureCsv(unittest.TestCase):
  @parameterized.expand(build_param_sets(["paths", "output_dir"], ["upper_bound", "feature_names", "resolution", "include_offsets"], "get_feature_csv"))
  def test_sequence(self, name, args, kwargs):
      sr.get_feature_csv(*args,**kwargs)
      feature_names = [os.path.splitext(p)[0] for p in os.listdir(args[1])]
      if len(kwargs["feature_names"]) > 0:
        # check that all the csvs are created
        self.assertSetEqual(set(feature_names), set(kwargs["feature_names"]))
      
      # check that csvs contain the same data as get_features()
      output,_,_ = sr.get_features(args[0], **kwargs)
      for k in kwargs["feature_names"]:
        path = os.path.join(args[1],k + ".csv")
        feature = pd.read_csv(path).values[:,1:].astype(np.float32)
        self.assertTrue( np.all(feature - output[k]) < 1e-4 )
        
      call("rm -rf " + args[1], shell=True)

class TestGetSimilarityMatrix(unittest.TestCase):
  @parameterized.expand(build_param_sets(["rank_set", "style_set"], ["upper_bound", "feature_names", "resolution", "include_offsets", "n_estimators", "max_depth", "return_paths_and_labels"], "get_similarity_matrix"))
  def test_sequence(self, name, args, kwargs):
    length = len(args[0]) + len(args[1])
    with warnings.catch_warnings():
      warnings.simplefilter("ignore")
      if kwargs["return_paths_and_labels"]:
        output, paths, labels = sr.get_similarity_matrix(*args,**kwargs)
        self.assertListEqual(list(paths), list(args[0]) + list(args[1]))
        self.assertListEqual(list(labels), [0]*len(args[0]) + [1]*len(args[1]))
      else:
        output = sr.get_similarity_matrix(*args,**kwargs)
      
      self.assertTrue(output.shape == (length,length))
      self.assertTrue(np.all((output - output.T) < 1e-4)) # is symmetric

class TestRank(unittest.TestCase):
  @parameterized.expand(build_param_sets(["rank_set", "style_set"], ["upper_bound", "feature_names", "resolution", "include_offsets", "n_estimators", "max_depth", "return_similarity"], "rank"))
  def test_sequence(self, name, args, kwargs):
    with warnings.catch_warnings():
      warnings.simplefilter("ignore")
      output = sr.rank(*args,**kwargs)
      self.assertTrue(len(output) == len(args[0]), "length")

# test that it fails on corrupt input
class TestRankOnCorrupt(unittest.TestCase):
  def test(self):
    try:
      with warnings.catch_warnings():
        warnings.simplefilter("ignore", category=PendingDeprecationWarning)
        sr.rank(["not_a_mid.pdf"], midi_paths, n_estimators=50, max_depth=2)
        self.assertTrue(False, "must throw exception")
    except Exception as e:
      print(e)

class TestCorrupt(unittest.TestCase):
  def test(self):
    try:
      with warnings.catch_warnings():
        warnings.simplefilter("ignore", category=PendingDeprecationWarning)
        sr.get_similarity_matrix(
          midi_paths, midi_paths, n_estimators=10, max_depth=1)
        sr.get_features(midi_paths, resolution=3)
        sr.get_features(midi_paths, upper_bound=-50) # this will throw exception
        self.assertTrue(False, "must throw exception")
    except Exception as e:
      print(e)

if __name__ == '__main__':
  unittest.main()
  os.chdir(curdir)