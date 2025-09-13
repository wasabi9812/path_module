#pragma once
#include <cstddef>
#include "pathlab/core/types.hpp"

namespace pathlab {

class IGraph {
public:
  using EdgeCB = void(*)(NodeId v, Cost32 w, void* ctx);
  virtual ~IGraph() = default;
  virtual std::size_t num_nodes() const = 0;
  virtual void for_each_edge(NodeId u, EdgeCB cb, void* ctx) const = 0;
};

} // namespace pathlab
