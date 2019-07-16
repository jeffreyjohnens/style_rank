import style_rank as sr
import numpy as np
import os

print(dir(sr))

if __name__ == "__main__":

  paths = ["/Users/Jeff/sfu/phd/style_rank/bwv2.6.mid", "/Users/Jeff/sfu/phd/style_rank/bwv3.6.mid", "garbage"]

  data, domains, indices = sr.get_features(paths, features=["ChordSize"])
  sr.get_feature_csv(paths, "test_csv_output")
