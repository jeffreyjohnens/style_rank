import style_rank as sr
import numpy as np
import os

if __name__ == "__main__":

  paths = ["bwv2.6.mid", "bwv3.6.mid"]

  data, domains, indices = sr.get_features(paths, features=["ChordSize"])
  sr.get_feature_csv(paths, "test_csv_output")
