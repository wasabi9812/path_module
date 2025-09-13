#include "pathlab/queues/stoc_pq.hpp"
#include <cassert>

namespace pathlab {

STOCPQ::STOCPQ(uint32_t block_size, Cost32 bound)
  : B_(block_size ? block_size : 256),
    bound_(bound ? bound : Key::INF) {}

void STOCPQ::reserve(std::size_t n) {
  ensure_best_size_(n);
}

void STOCPQ::clear() {
  batch_blocks_.clear();
  sorted_blocks_.clear();
  active_.clear();
  active_pos_ = 0;
  best_.assign(best_.size(), std::nullopt);
  live_ = 0;
  m_ = {};
}

bool STOCPQ::empty() const {
  return live_ == 0 && active_pos_ >= active_.size()
         && batch_blocks_.empty() && sorted_blocks_.empty();
}

std::size_t STOCPQ::size() const { return live_; }

void STOCPQ::ensure_best_size_(std::size_t n) {
  if (best_.size() <= n) best_.resize(n + 1);
}

void STOCPQ::append_unsorted_(Item it) {
  if (sorted_blocks_.empty() || sorted_blocks_.back().size() >= B_) {
    sorted_blocks_.emplace_back();
    sorted_blocks_.back().reserve(B_);
    m_.moves++; // 새 블록 할당으로 1회 이동 취급
  }
  sorted_blocks_.back().push_back(it);
  m_.moves++; // append 1회
}

void STOCPQ::prepend_batch_(std::vector<Item> blk) {
  if (blk.empty()) return;
  batch_blocks_.emplace_front(std::move(blk));
  m_.moves++; // prepend 1회
}

void STOCPQ::push(NodeId u, Key k) {
  if (k.primary >= bound_) return;
  ensure_best_size_(u);

  // 이미 들어있다면 decrease로 위임
  if (best_[u].has_value()) { decrease(u, k); return; }

  best_[u] = k;
  append_unsorted_({u, k});
  live_++;
  m_.pushes++;
}

void STOCPQ::decrease(NodeId u, Key k) {
  if (k.primary >= bound_) return;
  ensure_best_size_(u);
  auto& b = best_[u];

  if (!b.has_value() || KeyLess{}(k, *b)) {
    b = k; // 최신 키로 갱신
    append_unsorted_({u, k}); // 지연: 구키는 나중에 폐기
    live_++;
    m_.decreases++;
  }
  // 더 크거나 같으면 무시
}

// active 블록 준비: batch 앞 → 없으면 sorted 뒤에서 꺼내 정렬
bool STOCPQ::ensure_active_() {
  if (active_pos_ < active_.size()) return true;

  active_.clear();
  active_pos_ = 0;

  if (!batch_blocks_.empty()) {
    active_ = std::move(batch_blocks_.front());
    batch_blocks_.pop_front();
    m_.moves++; // 이동 1
  } else if (!sorted_blocks_.empty()) {
    active_ = std::move(sorted_blocks_.back());
    sorted_blocks_.pop_back();
    m_.moves++; // 이동 1
  } else {
    return false; // 진짜 비었음
  }

  // 정렬: 비교 1회당 scans++
  std::sort(active_.begin(), active_.end(),
            [this](const Item& a, const Item& b){
              m_.scans++;
              if (a.second.primary != b.second.primary) return a.second.primary < b.second.primary;
              return a.second.tie < b.second.tie;
            });
  // moves: 대략 n-1 만큼(안정적/보수적 근사)
  if (active_.size() > 1) m_.moves += (active_.size() - 1);
  return true;
}

// active_pos_부터 stale(현재 best와 불일치) 폐기
bool STOCPQ::skip_stale_forward_() {
  while (active_pos_ < active_.size()) {
    const auto& [u, k] = active_[active_pos_];
    if (u < best_.size() && best_[u].has_value()
        && !KeyLess{}(*best_[u], k) && !KeyLess{}(k, *best_[u])) {
      // k == best[u] → 유효
      return true;
    }
    // stale → 폐기
    active_pos_++;
    if (live_ > 0) live_--;
    m_.moves++; // discard 1회
  }
  return false;
}

std::pair<NodeId, Key> STOCPQ::peek_impl_() {
  while (true) {
    if (!ensure_active_()) break;
    if (skip_stale_forward_()) {
      const auto& [u, k] = active_[active_pos_];
      return {u, k};
    }
    // active 소진 → 다음 블록
  }
  return {0u, Key{Key::INF, 0}};
}

std::pair<NodeId, Key> STOCPQ::pop_impl_() {
  while (true) {
    if (!ensure_active_()) break;
    if (skip_stale_forward_()) {
      auto [u, k] = active_[active_pos_];
      // consume 1개
      active_pos_++;
      if (live_ > 0) live_--;
      // 이 시점에서 u는 PQ에서 제거되므로 best를 비워 contains=false가 됨
      best_[u].reset();
      m_.pops++;
      m_.moves++; // consume 1회
      return {u, k};
    }
  }
  return {0u, Key{Key::INF, 0}};
}

std::pair<NodeId, Key> STOCPQ::top() const {
  // const 인터페이스 요구로 내부 비-const 헬퍼 호출
  return const_cast<STOCPQ*>(this)->peek_impl_();
}

std::pair<NodeId, Key> STOCPQ::pop() {
  return pop_impl_();
}

bool STOCPQ::contains(NodeId u) const {
  return (u < best_.size()) && best_[u].has_value();
}

std::optional<Key> STOCPQ::key_of(NodeId u) const {
  if (!contains(u)) return std::nullopt;
  return best_[u];
}

} // namespace pathlab
