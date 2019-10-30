import os
import json
import collections
import numpy as np
from itertools import zip_longest, product

import style_rank as sr

import unittest
from parameterized import parameterized

curdir = os.getcwd()
os.chdir(os.path.dirname(os.path.realpath(__file__)))

midi_paths = ["bwv2.6.mid", "bwv3.6.mid"]
feature_names = sr.get_feature_names("ALL")
features_name_subsets = [list(np.random.choice(feature_names, size=(5,), replace=False)) for i in range(3)]

params = collections.OrderedDict([
    ("paths", [midi_paths]),
    ("upper_bound", [1,500]),
    ("feature_names", features_name_subsets + [[]]),
    ("resolution", [0,8]),
    ("include_offsets", [False,True]),
  ])

def build_param_sets(arg_list, kwarg_list):
  used_params = collections.OrderedDict([(k,v) for k,v in params.items() if k in arg_list or k in kwarg_list])
  param_sets = [dict(zip(used_params.keys(),x)) for x in list(product(*used_params.values()))]
  return [[str(p), [p[k] for k in arg_list], {k:p[k] for k in kwarg_list}] for p in param_sets]

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
  @parameterized.expand(build_param_sets(["paths"], ["upper_bound", "feature_names", "resolution", "include_offsets"]))
  def test_sequence(self, name, args, kwargs):
    output = sr.get_features(*args,**kwargs)
    if len(kwargs["feature_names"]) > 0:
      self.assertSetEqual(set(list(output[0].keys())), set(kwargs["feature_names"]))

if __name__ == '__main__':
  unittest.main()
  os.chdir(curdir)