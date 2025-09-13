#pragma once
#include <string>
#include <vector>
#include "pathlab/core/types.hpp"
#include "pathlab/core/graph_iface.hpp"

namespace pathlab {

// MovingAI .map -> 4/8-이웃 그래프 (정수 코스트: 직선10, 대각14)
class GridMap final : public IGraph {
public:
  explicit GridMap(const std::string& map_path, bool allow_diag = true);

  std::size_t num_nodes() const override { return (std::size_t)W_ * (std::size_t)H_; }

  // ★ 여기: IGraph::EdgeCB 로 명시
  void for_each_edge(NodeId u, IGraph::EdgeCB cb, void* ctx) const override;

  int width()  const { return W_; }
  int height() const { return H_; }
  bool passable(int x, int y) const {
    if (x < 0 || y < 0 || x >= W_ || y >= H_) return false;
    return free_[y*W_ + x];
  }
  static inline NodeId id(int x, int y, int W) { return (NodeId)(y*W + x); }

private:
  int W_ = 0, H_ = 0;
  bool diag_ = true;
  std::vector<uint8_t> free_;
};

} // namespace pathlab
