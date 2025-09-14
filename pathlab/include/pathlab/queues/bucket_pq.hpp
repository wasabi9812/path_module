#pragma once
#include <vector>
#include <list>
#include <cstdint>
#include <optional>
#include "pathlab/core/types.hpp"
#include "pathlab/queues/ipq.hpp"

// Key의 '거리' 필드 접근자. (기본: .primary)
// Key 구조가 다르면 아래 한 줄만 바꾸세요.
//   #define PATHLAB_KEY_COST(kexpr) ((kexpr).d)        // 예: d
//   #define PATHLAB_KEY_COST(kexpr) ((kexpr).dist)     // 예: dist
//   #define PATHLAB_KEY_COST(kexpr) ((kexpr).first)    // 예: pair<Cost32,uint32_t>
#ifndef PATHLAB_KEY_COST
  #define PATHLAB_KEY_COST(kexpr) ((kexpr).primary)
#endif

namespace pathlab {

// Dial-style Bucket PQ (monotone integer keys, edge w ∈ [1..W])
class BucketPQ final : public IPQ {
public:
  explicit BucketPQ(uint32_t max_w);

  void reserve(std::size_t n) override;
  void clear() override;
  bool empty() const override;
  std::size_t size() const override;

  void push(NodeId u, Key k) override;
  void decrease(NodeId u, Key k) override;

  std::pair<NodeId, Key> top() const override;
  std::pair<NodeId, Key> pop() override;

  bool contains(NodeId u) const override;
  std::optional<Key> key_of(NodeId u) const override;

  const PQMetrics& metrics() const override { return m_; }
  void reset_metrics() override { m_ = {}; }

private:
  // state
  Cost32   cur_min_ = 0;                // current minimal cost cursor
  uint32_t W_       = 1;                // max edge weight (e.g., 14 for 10/14)
  uint32_t K_       = 2;                // K = W_ + 1 (bucket count)
  uint32_t offset_  = 0;                // (optional) cur_min_ % K_
  uint32_t count_   = 0;                // number of items

  std::vector<std::list<NodeId>> buckets_; // size K_

  // node bookkeeping
  std::vector<uint8_t>  inq_;           // in-queue flag
  std::vector<Key>      key_;           // current key per node
  std::vector<uint32_t> bidx_;          // bucket index per node

  PQMetrics m_;

  // Dial: absolute distance modulo K
  inline uint32_t bucket_index_for(Cost32 d) const {
    return static_cast<uint32_t>(d % K_);
  }
};

} // namespace pathlab
