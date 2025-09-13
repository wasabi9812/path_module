#include "pathlab/core/grid_map.hpp"
#include <fstream>
#include <stdexcept>

namespace pathlab {

static bool is_free_char(char c) {
  return (c == '.' || c == 'G' || c == 'S');
}

GridMap::GridMap(const std::string& map_path, bool allow_diag) : diag_(allow_diag) {
  std::ifstream ifs(map_path);
  if (!ifs) throw std::runtime_error("cannot open map: " + map_path);

  std::string tag, typestr;
  ifs >> tag >> typestr; // type octile
  ifs >> tag >> H_;      // height
  ifs >> tag >> W_;      // width
  ifs >> tag;            // map
  if (W_ <= 0 || H_ <= 0) throw std::runtime_error("invalid size");

  free_.assign((std::size_t)W_*H_, 0);

  std::string line; std::getline(ifs, line);
  for (int y = 0; y < H_; ++y) {
    std::getline(ifs, line);
    if ((int)line.size() < W_) throw std::runtime_error("map row too short");
    for (int x = 0; x < W_; ++x) free_[y*W_ + x] = is_free_char(line[x]) ? 1 : 0;
  }
}

// ★ 여기: IGraph::EdgeCB 로 명시
void GridMap::for_each_edge(NodeId u, IGraph::EdgeCB cb, void* ctx) const {
  int x = (int)(u % (NodeId)W_);
  int y = (int)(u / (NodeId)W_);
  if (!passable(x,y)) return;

  static const int dx8[8] = { 1,-1, 0, 0, 1, 1,-1,-1 };
  static const int dy8[8] = { 0, 0, 1,-1, 1,-1, 1,-1 };
  static const int w8[8]  = {10,10,10,10,14,14,14,14};

  const int N = diag_ ? 8 : 4;
  for (int i=0;i<N;++i){
    int nx = x + dx8[i], ny = y + dy8[i];
    if (!passable(nx,ny)) continue;
    NodeId v = id(nx,ny,W_);
    cb(v, (Cost32)w8[i], ctx);
  }
}

} // namespace pathlab
