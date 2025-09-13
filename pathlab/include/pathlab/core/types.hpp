#pragma once
#include <cstdint>
#include <limits>

namespace pathlab {

using NodeId = uint32_t;
using Cost32 = uint32_t;

struct Key {
  Cost32  primary;      // Dijkstra: g
  uint32_t tie;         // secondary tie-breaker
  static constexpr Cost32 INF = std::numeric_limits<Cost32>::max();
};

struct KeyLess {
  bool operator()(const Key& a, const Key& b) const {
    if (a.primary != b.primary) return a.primary < b.primary;
    return a.tie < b.tie;
  }
};

} // namespace pathlab
