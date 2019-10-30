#include <iostream>
#include <vector>
#include <numeric>

template<typename T>
class zip_iterator {
  using it_type = typename std::vector<T>::iterator;
  it_type it1;
  it_type it2;
  public:
    zip_iterator(it_type iterator1, it_type iterator2)
      : it1{iterator1}, it2{iterator2} {}
    zip_iterator& operator++() {
      ++it1;
      ++it2;
      return *this;
    }
    bool operator!=(const zip_iterator& o) const {
      return it1 != o.it1 && it2 != o.it2;
    }
    bool operator==(const zip_iterator& o) const {
      return !operator!=(o);
    }
    std::pair<T,T> operator*() const {
      return {*it1, *it2};
    }
};

template<typename T>
class zipper {
  using vec_type = typename std::vector<T>;
  vec_type &vec;
  public:
    zipper(vec_type &v) : vec{v} {}
    zip_iterator<T> begin() const {
      return {std::begin(vec), std::begin(vec)+1};
    }
    zip_iterator<T> end() const {
      return {std::end(vec), std::end(vec)};
    }
};


/*
#include <vector>
#include <iostream>

int main() {
  std::vector<double> a {1.0, 2.0, 3.0};
  std::vector<double> b {1.0, 4.0};

  for (const auto &x : Zip::Zip(a,b)) {
    std::cout << std::get<0>(x) << " " << std::get<1>(x) << std::endl;
  }
}
*/