#include "pathlab/queues/bucket_pq.hpp"
#include <cassert>

namespace pathlab {

BucketPQ::BucketPQ(uint32_t max_w) : W_(max_w ? max_w : 1) {
  K_ = W_ + 1;
  buckets_.assign(K_, {});
}

void BucketPQ::reserve(std::size_t n) {
  // reserve node-related arrays (size = n)
  inq_.assign(n, 0);
  key_.assign(n, Key{Key::INF, 0});
  bidx_.assign(n, 0);
}

void BucketPQ::clear() {
  for (auto& b : buckets_) b.clear();
  if (!inq_.empty())  std::fill(inq_.begin(), inq_.end(), 0);
  cur_min_ = 0;
  offset_  = 0;
  count_   = 0;
  m_ = {};
}

bool BucketPQ::empty() const { return count_ == 0; }
std::size_t BucketPQ::size() const { return count_; }

void BucketPQ::push(NodeId v, Key k) {
  if (v >= inq_.size()) reserve(v + 1);     // grow node arrays if needed
  if (inq_[v]) { decrease(v, k); return; }  // already in queue -> treat as decrease

  key_[v] = k;
  const uint32_t bi = bucket_index_for(PATHLAB_KEY_COST(k)); // cost % K
  buckets_[bi].push_back(v);
  bidx_[v] = bi;
  inq_[v]  = 1;
  count_  += 1;
  m_.pushes++;
}

bool BucketPQ::contains(NodeId v) const {
  return (v < inq_.size()) && inq_[v];
}

void BucketPQ::decrease(NodeId v, Key k) {
  assert(contains(v));
  // unlink from old bucket
  auto& ob = buckets_[bidx_[v]];
  for (auto it = ob.begin(); it != ob.end(); ++it) {
    if (*it == v) { ob.erase(it); break; }
  }

  key_[v] = k;
  const uint32_t bi = bucket_index_for(PATHLAB_KEY_COST(k));
  buckets_[bi].push_back(v);
  bidx_[v] = bi;

  m_.decreases++;
  m_.moves++; // relink counted as a move
}

std::pair<NodeId, Key> BucketPQ::top() const {
  // Dial: only look at current bucket (cur_min_ % K_); do not mutate metrics.
  uint32_t idx = static_cast<uint32_t>(cur_min_ % K_);
  if (buckets_[idx].empty()) return { static_cast<NodeId>(0), Key{Key::INF, 0} };
  NodeId v = buckets_[idx].front();
  return { v, key_[v] };
}

std::pair<NodeId, Key> BucketPQ::pop() {
  // Dial: advance cur_min_ until its bucket has an element
  uint32_t idx = static_cast<uint32_t>(cur_min_ % K_);
  while (buckets_[idx].empty()) {
    cur_min_ += 1;            // increase distance one by one
    m_.scans++;               // count skipped empty buckets
    idx = static_cast<uint32_t>(cur_min_ % K_);
  }

  NodeId v = buckets_[idx].front();
  buckets_[idx].pop_front();
  inq_[v] = 0;
  count_ -= 1;
  m_.pops++;

  // Note: we do NOT set cur_min_ to the popped key; Dial increases lazily above.
  const Key kv = key_[v];
  offset_  = static_cast<uint32_t>(cur_min_ % K_); // optional

  return { v, kv };
}

std::optional<Key> BucketPQ::key_of(NodeId v) const {
  if (!contains(v)) return std::nullopt;
  return key_[v];
}

} // namespace pathlab
