#include "utils.hpp"
#include "parse.hpp"
#include "features.hpp"
#include "feature_map.hpp"

#include <tuple>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

std::tuple<VECTOR_MAP,VECTOR_MAP,std::vector<int>> get_features_internal(std::vector<std::string> &paths, int upper_bound) {
  Collector c;
  std::vector<int> indices;
  for (int i=0; i<paths.size(); i++) {
    Piece *p = new Piece(paths[i]);
    if ((p) && (p->chords.size() > 10)) {
      for (const auto &feature : m) {
        c.add(feature.first, feature.second(p));
      }
      indices.push_back(i);
    }
  }
  return std::tuple_cat(c.getData(upper_bound), std::tie(indices));
}

PYBIND11_PLUGIN(style_rank) {
  py::module m("style_rank", R"doc(
        Python module
        -----------------------
        .. currentmodule:: style_rank
        .. autosummary::
           :toctree: _generate

           add
           subtract
    )doc");

  m.def("get_features_internal", &get_features_internal);
  return m.ptr();
}