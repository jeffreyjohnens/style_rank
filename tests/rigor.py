import numpy as np
import style_rank as sr

import os
curdir = os.getcwd()
os.chdir(os.path.dirname(os.path.realpath(__file__)))

def get_midi_paths(folder):
  return [os.path.join(folder,p) for p in os.listdir(folder) if p.endswith(".mid")]

#a = get_midi_paths("/Users/jeff/Code/MIDI_DATA/CLASSICAL_ARCHIVES/HECTOR BERLIOZ")
#b = get_midi_paths("/Users/jeff/Code/MIDI_DATA/CLASSICAL_ARCHIVES/DIONISIO AGUADO")
a = get_midi_paths("/Users/Jeff/sfuvault/MIDI_DATA/CLASSICAL_ARCHIVES/AGUADO")
b = get_midi_paths("/Users/Jeff/sfuvault/MIDI_DATA/CLASSICAL_ARCHIVES/BERLIOZ")

style_set = b[:5]
rank_set = a + b[5:]

midi_paths = ["bwv2.6.mid", "bwv3.6.mid"]
feature_names = sr.get_feature_names("ALL")

features_name_subset = np.random.choice(feature_names, size=(5,), replace=False)
features, domains, index = sr.get_features(
  midi_paths, feature_names=features_name_subset)

assert set(list(features.keys())) == set(list(features_name_subset))

import json
sr.rank(rank_set, style_set, feature_names=sr.get_feature_names("ORIGINAL"), json_path="test.json")
with open("test.json", "r") as f:
  print(json.load(f))

os.chdir(curdir)