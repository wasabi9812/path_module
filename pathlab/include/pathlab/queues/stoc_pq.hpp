#pragma once
#include <vector>
#include <deque>
#include <optional>
#include <utility>
#include <cstdint>
#include <algorithm>
#include "pathlab/queues/ipq.hpp"

namespace pathlab {

// Batch-based partially-sorted PQ adapted to IPQ:
// - 내부에는 "미정렬 블록" 두 종류 보관: batch(큐, 먼저 소진) / sorted(스택, 나중 소진)
// - 필요할 때만 블록 하나를 꺼내 정렬해 active 블록으로 만들고, 그 안에서 1개씩 top/pop
// - decrease-key는 "지연" 처리: best[u]만 최신으로 유지, stale 엔트리는 pop시 건너뜀
// - metrics: scans=비교횟수(정렬/비교), moves=삽입/삭제/폐기 등의 재배치

class STOCPQ final : public IPQ {
public:
  explicit STOCPQ(uint32_t block_size = 256, Cost32 bound = Key::INF);

  void reserve(std::size_t n) override;
  void clear() override;
  bool empty() const override;
  std::size_t size() const override;

  void push(NodeId u, Key k) override;      // insert
  void decrease(NodeId u, Key k) override;  // lazy decrease

  std::pair<NodeId, Key> top() const override; // 내부에서 active 준비, stale skip
  std::pair<NodeId, Key> pop() override;       // 위와 동일 + 실제 consume

  bool contains(NodeId u) const override;
  std::optional<Key> key_of(NodeId u) const override;

  const PQMetrics& metrics() const override { return m_; }
  void reset_metrics() override { m_ = {}; }

private:
  using Item = std::pair<NodeId, Key>; // (vertex, key)

  // 미정렬 블록 저장소
  std::deque<std::vector<Item>> batch_blocks_; // 먼저 소진
  std::vector<std::vector<Item>> sorted_blocks_; // 나중 소진

  // 현재 정렬되어 소비 중인 블록
  std::vector<Item> active_;
  std::size_t active_pos_ = 0;

  // per-node best key (지연 감소 처리)
  std::vector<std::optional<Key>> best_;

  uint32_t B_;         // 블록 크기
  Cost32   bound_;     // 키 상한(>=bound 무시)
  std::size_t live_ = 0;   // 유효 엔트리 추정치(지연으로 과대 가능, stale 폐기 시 감소)
  mutable PQMetrics m_;    // metrics

  // 내부 유틸
  void ensure_best_size_(std::size_t n);
  void append_unsorted_(Item it);               // sorted_blocks 뒤에 채우기
  void prepend_batch_(std::vector<Item> blk);   // batch_blocks 앞에 넣기

  bool ensure_active_();                        // active 없으면 블록 하나 꺼내 정렬
  bool skip_stale_forward_();                   // active_pos_부터 stale 폐기
  std::pair<NodeId, Key> peek_impl_();          // 비-const top 구현
  std::pair<NodeId, Key> pop_impl_();           // 비-const pop 구현
};

} // namespace pathlab
