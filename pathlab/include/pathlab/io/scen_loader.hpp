#pragma once
#include <string>
#include <vector>
#include "pathlab/core/types.hpp"

namespace pathlab {

struct ScenCase {
  std::string map_name;
  int map_w = 0, map_h = 0;
  int sx = 0, sy = 0;
  int gx = 0, gy = 0;
  double opt = 0.0;
};

std::vector<ScenCase> load_scen(const std::string& scen_path);
inline NodeId node_from_xy(int x, int y, int W) { return (NodeId)(y*W + x); }

} // namespace pathlab
