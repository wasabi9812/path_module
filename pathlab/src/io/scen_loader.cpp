#include "pathlab/io/scen_loader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace pathlab {

std::vector<ScenCase> load_scen(const std::string& scen_path) {
  std::ifstream ifs(scen_path);
  if (!ifs) throw std::runtime_error("cannot open scen: " + scen_path);

  std::vector<ScenCase> out;
  std::string line; bool first = true;

  while (std::getline(ifs, line)) {
    if (line.empty()) continue;

    // 첫 줄이 "version X" 면 스킵
    if (first && line.rfind("version", 0) == 0) { first = false; continue; }
    first = false;

    std::istringstream iss(line);
    int bucket;
    ScenCase c;
    // MovingAI .scen: bucket map_name map_w map_h sx sy gx gy [opt]
    if (!(iss >> bucket >> c.map_name >> c.map_w >> c.map_h
              >> c.sx >> c.sy >> c.gx >> c.gy)) {
      continue; // parse 실패 라인 스킵
    }
    // opt(있을 수도/없을 수도)
    if (!(iss >> c.opt)) c.opt = 0.0;

    out.push_back(c);
  }
  return out;
}

} // namespace pathlab
