cmake_minimum_required(VERSION 3.16)
project(pathlab LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PATHLAB_INC ${CMAKE_CURRENT_SOURCE_DIR}/pathlab/include)

add_library(pathlab_core
  pathlab/src/core/grid_map.cpp
  pathlab/src/io/scen_loader.cpp
  pathlab/src/queues/heap_pq.cpp
  pathlab/src/queues/stoc_pq.cpp
  pathlab/src/ll/dijkstra.cpp
)
target_include_directories(pathlab_core PUBLIC ${PATHLAB_INC})

add_executable(bench_single pathlab/src/apps/bench_single.cpp)
target_include_directories(bench_single PRIVATE ${PATHLAB_INC})
target_link_libraries(bench_single PRIVATE pathlab_core)
