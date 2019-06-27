import style_rank as sr
import numpy as np
import os

if __name__ == "__main__":

  root = "/Users/Jeff/Code/ISMIR_2019/CODE_APPENDIX/bach_midi"
  paths = [os.path.join(root,p) for p in os.listdir(root) if p.endswith(".mid")]
  data, domains, indices = sr.get_features(paths)

  d = sr.get_distance_matrix(paths, np.random.randint(2,size=(len(paths),)))

  print(d.shape)
