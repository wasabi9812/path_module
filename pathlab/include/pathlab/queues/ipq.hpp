#pragma once
#include <cstddef>
#include <utility>
#include <optional>
#include "pathlab/core/types.hpp"
#include "pathlab/util/counters.hpp"

namespace pathlab {

class IPQ {
public:
  virtual ~IPQ() = default;

  virtual void reserve(std::size_t n) = 0;
  virtual void clear() = 0;
  virtual bool empty() const = 0;
  virtual std::size_t size() const = 0;

  virtual void push(NodeId u, Key k) = 0;
  virtual void decrease(NodeId u, Key k) = 0;

  virtual std::pair<NodeId, Key> top() const = 0;
  virtual std::pair<NodeId, Key> pop() = 0;

  virtual bool contains(NodeId u) const = 0;
  virtual std::optional<Key> key_of(NodeId u) const = 0;

  virtual const PQMetrics& metrics() const = 0;
  virtual void reset_metrics() = 0;
};

} // namespace pathlab
