#ifndef STYLE_RANK_FEATURE_MAP_H
#define STYLE_RANK_FEATURE_MAP_H

#include "utils.hpp"

#include <unordered_map>
#include <string>

static std::unordered_map<std::string, std::unique_ptr<DISCRETE_DIST>(*)(Piece*)> m { 
    $FEATURE_MAP
};

#endif