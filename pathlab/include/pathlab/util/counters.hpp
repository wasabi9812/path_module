#pragma once
#include <cstdint>

namespace pathlab {

struct PQMetrics {
  uint64_t pushes = 0;
  uint64_t pops = 0;
  uint64_t decreases = 0;
  uint64_t moves = 0;
  uint64_t scans = 0;
};

} // namespace pathlab
