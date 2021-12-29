find_package(fmt)

add_subdirectory(2021)

add_executable(aoc aoc.m.cpp)

target_link_libraries(
  aoc
  PRIVATE project_options
          project_warnings
          aoc2021
          fmt::fmt)