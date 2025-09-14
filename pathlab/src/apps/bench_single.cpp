#include <cstdio>
#include <cstdlib>
#include <string>
#include <memory>
#include <chrono>
#include <limits>

#include "pathlab/core/grid_map.hpp"
#include "pathlab/io/scen_loader.hpp"
#include "pathlab/queues/ipq.hpp"
#include "pathlab/queues/heap_pq.hpp"
#include "pathlab/queues/stoc_pq.hpp"
#include "pathlab/ll/dijkstra.hpp"
#include "pathlab/queues/bucket_pq.hpp"  // <-- bucket PQ

using namespace pathlab;

// allow_diag 여부에 따라 W(최대 가중치) 결정: 10/14 스케일 가정
static std::unique_ptr<IPQ> make_pq(const std::string& name,
                                    uint32_t stoc_block,
                                    bool allow_diag) {
  if (name == "heap")   return std::make_unique<HeapPQ>();
  if (name == "stoc")   return std::make_unique<STOCPQ>(stoc_block);
  if (name == "bucket") {
    uint32_t W = allow_diag ? 14u : 10u;
    return std::make_unique<BucketPQ>(W);
  }
  // 기본은 heap
  return std::make_unique<HeapPQ>();
}

// parent를 따라가 “칸 수(=이동 횟수)” 계산
static uint32_t reconstruct_steps(const DijkstraResult& R, NodeId s, NodeId g) {
  const NodeId INVALID = std::numeric_limits<NodeId>::max();
  if (R.dist.empty()) return 0;
  if (R.dist[g] == Key::INF) return 0;   // 도달 불가
  uint32_t steps = 0;
  NodeId v = g;
  while (v != INVALID && v != s) {
    v = R.parent[v];
    ++steps;
  }
  if (v == INVALID) return 0;            // 예외: 부모 끊김
  return steps;                          // 경로 셀 개수 - 1
}

// 10/14 스케일에서 steps, dist로 직선/대각 분해
static inline void split_steps_10_14(uint32_t steps, uint32_t dist,
                                     uint32_t& straight, uint32_t& diag) {
  // steps = H + D, dist = 10*H + 14*D
  // => 4*D = dist - 10*steps
  int64_t tmp = (int64_t)dist - 10LL * (int64_t)steps;
  if (tmp < 0) { straight = steps; diag = 0; return; }
  diag = (uint32_t)(tmp / 4);
  if (diag > steps) diag = steps;
  straight = steps - diag;
}

int main(int argc, char** argv) {
  if (argc < 5) {
    std::fprintf(stderr,
      "usage: bench_single <map> <scen> <pq:heap|stoc|bucket> <cases>\n"
      "       [allow_diag=1] [stoc_block=256]\n");
    return 1;
  }
  std::string map_path  = argv[1];
  std::string scen_path = argv[2];
  std::string pq_name   = argv[3];
  int cases = std::atoi(argv[4]);
  int allow_diag = (argc > 5) ? std::atoi(argv[5]) : 1;
  uint32_t stoc_block = (argc > 6) ? (uint32_t)std::strtoul(argv[6], nullptr, 10) : 256u;

  GridMap G(map_path, allow_diag != 0);
  auto S = load_scen(scen_path);
  if (cases <= 0 || cases > (int)S.size()) cases = (int)S.size();

  auto pq = make_pq(pq_name, stoc_block, allow_diag != 0);

  uint64_t total_ms = 0;
  for (int i=0;i<cases;++i) {
    const auto& c = S[i];
    const NodeId s = node_from_xy(c.sx, c.sy, G.width());
    const NodeId g = node_from_xy(c.gx, c.gy, G.width());

    pq->reset_metrics();
    auto t0 = std::chrono::high_resolution_clock::now();
    DijkstraResult R = dijkstra_single(G, s, *pq);
    auto t1 = std::chrono::high_resolution_clock::now();
    uint64_t ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    total_ms += ms;

    const uint32_t steps = reconstruct_steps(R, s, g);
    uint32_t hv = 0, dg = 0;
    split_steps_10_14(steps, (uint32_t)R.dist[g], hv, dg);

    std::printf(
      "case=%d start=(%d,%d) goal=(%d,%d) dist=%u steps=%u (H=%u,D=%u) time=%llums | "
      "PQ push=%llu pop=%llu dec=%llu scans=%llu moves=%llu | "
      "algo relax=%llu improved=%llu settled=%llu\n",
      i, c.sx, c.sy, c.gx, c.gy,
      (unsigned)R.dist[g],
      (unsigned)steps, (unsigned)hv, (unsigned)dg,
      (unsigned long long)ms,
      (unsigned long long)R.pq.pushes,
      (unsigned long long)R.pq.pops,       // expand ~= pop
      (unsigned long long)R.pq.decreases,
      (unsigned long long)R.pq.scans,
      (unsigned long long)R.pq.moves,
      (unsigned long long)R.algo.relaxations,
      (unsigned long long)R.algo.improved,
      (unsigned long long)R.algo.settled
    );
  }

  std::printf("TOTAL %d cases: %llums (avg %.3f ms/case)\n",
              cases,
              (unsigned long long)total_ms,
              (cases>0)? (double)total_ms / (double)cases : 0.0);
  return 0;
}
