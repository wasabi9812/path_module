#pragma once
#include <vector>
#include <optional>
#include "pathlab/queues/ipq.hpp"

namespace pathlab {

class HeapPQ final : public IPQ {
public:
  explicit HeapPQ(std::size_t n_hint = 0);

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
  struct Entry { NodeId u; Key k; };
  std::vector<Entry> heap_;
  std::vector<int32_t> pos_;
  PQMetrics m_;

  bool less_(int a, int b) const;
  void swap_(int a, int b);
  void sift_up_(int i);
  void sift_down_(int i);
  void ensure_pos_size_(std::size_t n);
};

} // namespace pathlab
