#ifndef STYLE_RANK_UTILS_H
#define STYLE_RANK_UTILS_H

#include <vector>
#include <numeric>
#include <sstream>
#include <iostream>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <assert.h>

// macro for suppressing std::cout and std::cerr
#define QUIET_CALL(noisy) { \
    std::cout.setstate(std::ios_base::failbit);\
    std::cerr.setstate(std::ios_base::failbit);\
    (noisy);\
    std::cout.clear();\
    std::cerr.clear();\
}

int mod(int a, int b) {
		int r = a % b;
		return r < 0 ? r + b : r;
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

// normalize a vector in place ...
void normalize(std::vector<double> &x) {
  double norm = std::accumulate(x.begin(), x.end(), 0.0);
  std::transform(x.begin(), x.end(), x.begin(), [norm](const double t){ return t/norm; });
}

class NOMINAL_TUPLE {
public:
  uint64_t value;
  NOMINAL_TUPLE(uint64_t a, uint64_t b=0, uint64_t c=0, uint64_t d=0) {
    assert(std::max({a,b,c,d}) < (1<<8));
    value = (a & 0xff) + ((b & 0xff)<<8) + ((c & 0xff)<<16) + ((d & 0xff)<<24);
  }
  NOMINAL_TUPLE(std::vector<int>::iterator begin, std::vector<int>::iterator end) {
    //assert(std::max_element(begin, end) < (1<<8));
    value = 0;
    int c = 0;
    for (auto it = begin; it != end; it++) {
      assert(*it < (1<<8));
      value |= (*it << 8*c);
      c++;
    }
  }
};

using DISCRETE_DIST = std::unordered_map<uint64_t, uint64_t>;
using VECTOR_MAP = std::map<std::string, std::vector<uint64_t>>;

template<typename TK, typename TV>
std::pair<TV,TK> flip_pair(const std::pair<TK,TV> &p) {
    return std::pair<TV,TK>(p.second, p.first);
}

template<typename TK, typename TV>
std::multimap<TV,TK> flip_map(const std::map<TK,TV> &src) {
    std::multimap<TV,TK> ret;
    std::transform(src.begin(), src.end(), std::inserter(ret, ret.begin()), 
                   flip_pair<TK,TV>);
    return ret;
}

template<typename TK, typename TV>
std::vector<TV> extract_values_in_reverse(std::multimap<TK, TV> const& src) {
    std::vector<TV> ret;
    for (auto element=src.rbegin(); element!=src.rend(); ++element) {
        ret.push_back(element->second);
    }
    return ret;
}

template<typename TK, typename TV>
std::vector<TK> extract_keys(std::multimap<TK, TV> &src) {
  std::vector<TK> ret;
  for (auto const& element : src) {
    ret.push_back(element.first);
  }
  return ret;
}

template<typename TK, typename TV>
std::vector<TK> extract_keys(std::unordered_map<TK, TV> &src) {
  std::vector<TK> ret;
  for (auto const& element : src) {
    ret.push_back(element.first);
  }
  return ret;
}

// how to make function for all containers?
template<typename TK, typename TV>
void print(std::unordered_map<TK,TV> const& src) {
    for (auto const &p : src) {
        std::cout << p.first << " : " << p.second << std::endl;
    }
    std::cout << std::endl;
}

class Collector {
public:
    std::vector<int> labels;
    std::map<std::string, std::vector<std::unique_ptr<DISCRETE_DIST>>> dists;
    std::map<std::string, std::map<uint64_t,size_t>> domains_map; // counts

    void add(std::string name, std::unique_ptr<DISCRETE_DIST> x) {
        for (const auto &kv : *x) {
            domains_map[name][kv.first]++;
        }
        dists[name].push_back(std::move(x));
    }
    void addLabel(int label) {
        labels.push_back(label);
    }
 
    std::tuple<VECTOR_MAP,VECTOR_MAP> getData(size_t upper_bound) {
        
        VECTOR_MAP ret;
        VECTOR_MAP domains;

        for (auto const &kv : dists) {
            auto rev_domains = flip_map(domains_map[kv.first]);
            auto domain = extract_values_in_reverse<size_t,uint64_t>(rev_domains);
            if (domain.size() > upper_bound) {
                domain.resize(upper_bound); // only keep top n
            }
            std::vector<uint64_t> mat;

            for (const auto &dist : kv.second) {

                // find the total of the distribution
                auto total = std::accumulate(
                    dist->begin(), dist->end(), 0, [](const size_t s, const auto &elem) { return s + elem.second; });
                
                // find the keys in the distribution
                size_t used = 0;
                for (const auto &d : domain) {
                    auto it = dist->find( d );
                    if (it != dist->end()) {
                        mat.push_back(it->second);
                        used += it->second;
                    }
                    else {
                        mat.push_back(0); // if not found in distribution
                    }
                }
                mat.push_back(total - used); // add remainder 
            }
            domains[kv.first] = domain;
            ret[kv.first] = mat;  
        }
        return std::make_pair(ret,domains);
    }
};

#endif
