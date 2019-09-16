# StyleRank

StyleRank is a method to rank MIDI files based on their similarity to an arbitrary musical style delineated by a collection of MIDI files. MIDI files are encoded using a novel set of features and an embedding is learned using Random Forests.

## Getting Started

### Installing

```python
pip install style_rank
```

### Basic Examples

```python
from style_rank import rank
to_rank_paths = ["in_style.mid", "out_of_style.mid", "somewhat_in_style.mid"]
corpus_paths = ["corpus_1.mid", "corpus_2.mid", "corpus_3.mid"]
rank(to_rank, corpus)
>>> ["in_style.mid", "somewhat_in_style.mid", "out_of_style.mid"]
```

## Built With

* [pybind11](https://github.com/pybind/pybind11) - c++ integration 
* [midifile](https://midifile.sapp.org/) - midi parsing

## Citing

If you want to cite StyleRank, please use the following citation.

Ens,J. and Pasquier,P. Quantifying Musical Style: Ranking Symbolic Music based on Similarity to a Style. International Symposium on Music Information Retrieval (forthcoming 2019).

## License

This project is licensed under the ISC License - see the [LICENSE.md](LICENSE.md) file for details
