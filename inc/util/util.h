#pragma once
#include <boost/functional/hash.hpp>
#include <utility>

struct PairIntHash {
  size_t operator() (const std::pair<int, int>& p) const {
    size_t seed = 0;
    boost::hash_combine(seed, p.first);
    boost::hash_combine(seed, p.second);
    return seed;
  }
};