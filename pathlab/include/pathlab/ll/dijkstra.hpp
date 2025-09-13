#pragma once
#include <vector>
#include "pathlab/core/types.hpp"
#include "pathlab/core/graph_iface.hpp"
#include "pathlab/queues/ipq.hpp"

namespace pathlab {

struct DijkstraMetrics {
  uint64_t relaxations = 0;
  uint64_t improved    = 0;
  uint64_t settled     = 0;
};

struct DijkstraResult {
  std::vector<Cost32> dist;
  std::vector<NodeId> parent;
  DijkstraMetrics algo;
  PQMetrics pq;
};

DijkstraResult dijkstra_single(const IGraph& G, NodeId s, IPQ& Q);

} // namespace pathlab
