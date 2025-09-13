#include "pathlab/queues/heap_pq.hpp"
#include <cassert>
#include <utility>

namespace pathlab {

HeapPQ::HeapPQ(std::size_t n_hint) {
  if (n_hint) { heap_.reserve(n_hint); pos_.assign(n_hint, -1); }
}

void HeapPQ::reserve(std::size_t n) {
  heap_.reserve(n);
  if (pos_.size() < n) pos_.resize(n, -1);
}

void HeapPQ::clear() {
  heap_.clear();
  pos_.assign(pos_.size(), -1);
  m_ = {};
}

bool HeapPQ::empty() const { return heap_.empty(); }
std::size_t HeapPQ::size() const { return heap_.size(); }

bool HeapPQ::less_(int a, int b) const {
  const auto& A = heap_[a].k; const auto& B = heap_[b].k;
  if (A.primary != B.primary) return A.primary < B.primary;
  return A.tie < B.tie;
}

void HeapPQ::swap_(int a, int b) {
  std::swap(heap_[a], heap_[b]);
  pos_[heap_[a].u] = a; pos_[heap_[b].u] = b;
  m_.moves += 2;
}

void HeapPQ::sift_up_(int i) {
  while (i > 0) {
    int p = (i - 1) >> 1;
    if (!less_(i, p)) break;
    swap_(i, p); i = p;
  }
}

void HeapPQ::sift_down_(int i) {
  int n = (int)heap_.size();
  while (true) {
    int l = (i<<1)+1, r = l+1, m = i;
    if (l < n && less_(l, m)) m = l;
    if (r < n && less_(r, m)) m = r;
    if (m == i) break;
    swap_(i, m); i = m;
  }
}

void HeapPQ::ensure_pos_size_(std::size_t n){
  if (pos_.size() <= n) pos_.resize(n+1, -1);
}

void HeapPQ::push(NodeId u, Key k) {
  ensure_pos_size_(u);
  if (pos_[u] != -1) { decrease(u, k); return; }
  int idx = (int)heap_.size();
  heap_.push_back({u,k});
  pos_[u] = idx;
  sift_up_(idx);
  m_.pushes++;
}

void HeapPQ::decrease(NodeId u, Key k) {
  ensure_pos_size_(u);
  int idx = pos_[u];
  if (idx == -1) { push(u, k); return; }
  if (k.primary == heap_[idx].k.primary && k.tie >= heap_[idx].k.tie) return;
  heap_[idx].k = k;
  sift_up_(idx);
  m_.decreases++;
}

std::pair<NodeId, Key> HeapPQ::top() const {
  assert(!heap_.empty());
  return { heap_.front().u, heap_.front().k };
}

std::pair<NodeId, Key> HeapPQ::pop() {
  assert(!heap_.empty());
  auto out = heap_.front();
  int n = (int)heap_.size();
  pos_[out.u] = -1;
  if (n == 1) {
    heap_.pop_back();
  } else {
    heap_[0] = heap_.back();
    pos_[heap_[0].u] = 0;
    heap_.pop_back();
    sift_down_(0);
  }
  m_.pops++;
  return { out.u, out.k };
}

bool HeapPQ::contains(NodeId u) const {
  return (u < pos_.size()) && (pos_[u] != -1);
}

std::optional<Key> HeapPQ::key_of(NodeId u) const {
  if (!contains(u)) return std::nullopt;
  return heap_[pos_[u]].k;
}

} // namespace pathlab
