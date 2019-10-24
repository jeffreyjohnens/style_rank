#ifndef STYLE_RANK_FEATURE_MAP_H
#define STYLE_RANK_FEATURE_MAP_H

#include "utils.hpp"

#include <unordered_map>
#include <string>
#include <vector>
using namespace std;

static unordered_map<string, unique_ptr<DISCRETE_DIST>(*)(Piece*)> m { 
    $FEATURE_MAP
};

static unordered_map<string, vector<string>> feature_tag_map { 
    $FEATURE_TAG_MAP
};

#endif