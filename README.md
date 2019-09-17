# StyleRank

![alt text](https://github.com/jeffreyjohnens/style_rank/blob/master/diagram/diagram.png)

StyleRank is a method to rank MIDI files based on their similarity to an arbitrary musical style delineated by a collection of MIDI files. MIDI files are encoded using a novel set of features and an embedding is learned using Random Forests. For a detailed explanation see the original paper.

## Getting Started

### Installing

Python2 is not supported. Python>=3.6.5 is supported.

```python
pip install pybind11
pip install style_rank
```

### Basic Examples

```python
# rank midi files with respect to a style delineated corpus_paths
from style_rank import rank
to_rank_paths = ["in_style.mid", "out_of_style.mid", "somewhat_in_style.mid"]
corpus_paths = ["corpus_1.mid", "corpus_2.mid", "corpus_3.mid"]
rank(to_rank, corpus)
>>> ["in_style.mid", "somewhat_in_style.mid", "out_of_style.mid"]

# get a list of all the features
from style_rank import get_feature_names
get_feature_names()
>>> ['ChordMelodyNgram', 'ChordTranDistance', ..., 'IntervalClassDist', 'IntervalDist']

# extract features to csv's in the /path/to/csv_output folder
from style_rank import get_feature_csv
feature_names = ['IntervalClassDist', 'IntervalDist']
paths = ["corpus_1.mid", "corpus_2.mid", "corpus_3.mid"]
get_feature_csv(paths, '/path/to/csv_output', feature_names=feature_names)
```
### Features

Find the documentation for each feature here. (in progress)

## Built With

* [pybind11](https://github.com/pybind/pybind11) - c++ integration 
* [midifile](https://midifile.sapp.org/) - midi parsing

## Citing

If you want to cite StyleRank, please use the following citation.

Ens,J. and Pasquier,P. Quantifying Musical Style: Ranking Symbolic Music based on Similarity to a Style. International Symposium on Music Information Retrieval (forthcoming 2019).

## License

This project is licensed under the ISC License - see the [LICENSE.md](LICENSE.md) file for details
