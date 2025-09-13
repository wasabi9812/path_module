#include "pathlab/ll/dijkstra.hpp"
#include <limits>

namespace pathlab {

DijkstraResult dijkstra_single(const IGraph& G, NodeId s, IPQ& Q) {
  const std::size_t N = G.num_nodes();
  std::vector<Cost32> dist(N, Key::INF);
  std::vector<NodeId> parent(N, std::numeric_limits<NodeId>::max());
  DijkstraMetrics am{};
  Q.clear(); Q.reserve(N);

  uint32_t tie = 0;
  dist[s] = 0;
  Q.push(s, Key{0u, tie++});

  struct Ctx {
    NodeId u;
    std::vector<Cost32>* D;
    std::vector<NodeId>* P;
    IPQ* Q;
    uint32_t* tie;
    DijkstraMetrics* am;
  } ctx{0, &dist, &parent, &Q, &tie, &am};

  while (!Q.empty()) {
    auto [u, ku] = Q.pop();
    am.settled++;
    ctx.u = u;

    auto cb = [](NodeId v, Cost32 w, void* p){
      auto& C = *static_cast<Ctx*>(p);
      auto& D = *C.D; auto& P = *C.P; auto& Q = *C.Q; auto& am = *C.am;
      am.relaxations++;
      const uint64_t cand = (uint64_t)D[C.u] + w;
      if (cand < D[v]) {
        D[v] = (Cost32)cand; P[v] = C.u; am.improved++;
        Key nk{D[v], (*C.tie)++};
        if (Q.contains(v)) Q.decrease(v, nk);
        else               Q.push(v, nk);
      }
    };
    G.for_each_edge(u, cb, &ctx);
  }

  return { std::move(dist), std::move(parent), am, Q.metrics() };
}

} // namespace pathlab
