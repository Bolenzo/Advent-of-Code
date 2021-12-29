///////////////////////////////////////////////////////////////////////////////
// File aoc.m.hpp
///////////////////////////////////////////////////////////////////////////////
// This is an application running Advent of Code puzzles.

#include <array>

#include "AoC_2021_01.hpp"

#include <fmt/core.h>


int main()
{
  auto depths = std::array{199, 200, 208, 210, 200, 207, 240, 269, 260, 263};

  fmt::print("This is but an example application.\n"
             "Out of {} depth measurements, there are {} increments.\n",
    depths.size(), aoc::count_increases(depths));

}

