// Advent of Code 2020: day 3

// Godbolt link: https://godbolt.org/z/TzzMKj

#include <algorithm>
#include <cassert>
#include <vector>
#include <fmt/core.h>

template<class T>
class Matrix
{
  std::vector<T> data_;
  std::size_t cols_;
public:
  Matrix(std::size_t rows, std::size_t cols, T const& value)
  : data_(rows*cols, value)
  , cols_{cols}
  { }

  template<class It>
  void add_row(It first, It last) {
    assert(std::distance(first, last) == cols_);
    auto const idx = data_.size();
    data_.resize(data_.size() + cols_);
    std::copy(first, last, std::next(begin(data_), idx));
  }

  std::size_t rows() const noexcept { return data_.size() / cols_; }
  std::size_t cols() const noexcept { return cols_; }

  T at(std::size_t row, std::size_t col) const noexcept {
    return data_[row*cols_ + col];
  }

  std::size_t count(T const& value) const noexcept {
    return std::count(begin(data_), end(data_), value);
  }
};

std::size_t get_cols(char const* s) {
  std::size_t res = 0;
  while (*s != '\0' && *s++ != '\n') {
    ++res;
  }
  return res;
}

Matrix<bool> parse_trees(char const* s) {
  auto constexpr tree = '#';
  auto constexpr clear = '.';
  auto const cols = get_cols(s);
  Matrix<bool> res(0, cols, false);
  std::vector<bool> buffer_(cols, false);
  while (*s != '\0') {
    if (*s == '\n') ++s;
    std::transform(s, s + cols, begin(buffer_), [&](char const c) {
      return c == tree;
    });
    res.add_row(begin(buffer_), end(buffer_));
    s += cols;
  }
  return res;
}

std::size_t slope_trees(Matrix<bool> const& m, std::size_t h, std::size_t v) {
  std::size_t res = 0;
  for (std::size_t i = 0, j = 0; i < m.rows(); i += v, j = (j + h) % m.cols()) {
    res += m.at(i, j);
  }
  return res;
}

int main() {

auto const test = R"(..##.......
#...#...#..
.#....#..#.
..#.#...#.#
.#...##..#.
..#.##.....
.#.#.#....#
.#........#
#.##...#...
#...##....#
.#..#...#.#)";

auto const input = R"(......#...........#...#........
.#.....#...##.......#.....##...
......#.#....#.................
..............#.#.......#......
.....#.#...##...#.#..#..#..#..#
.......##...#..#...........#...
.......#.##.#...#.#.........#..
..#...##............##......#.#
.......#.......##......##.##.#.
...#...#........#....#........#
#............###.#......#.....#
..#........#....#..#..........#
..#..##....#......#..#......#..
........#......#......#..#..#..
..#...#....#..##.......#.#.....
.....#.#......#..#....#.##.#..#
......###.....#..#..........#..
.#................#.#..........
.........#..#...#......##......
##...#....#...#.#...#.##..#....
...##...#....#.........###.....
.#.#....#.........##...........
....#.#..#..#...........#......
..#..#.#....#....#...#.........
..........##.....#.##..........
..#.#....#..##......#.#.....##.
..#...#.##......#..........#...
......#....#..#.....#.....#...#
#.#...##.#.##.........#..#.....
...#.#.#.........#.....#.#.#...
..#.........#...............#..
#..##.....#.........#....#.....
...#....##..##...........##..#.
......##.................#.#...
##.......#....#.#.#.....#......
....#.#...#.................##.
#...#.........##.....#.........
#....#.###..#.....##.#....#....
#..#....#...#....#.#.#.........
.......#...........#....#.....#
#...#.............#........#...
.......#.....#...#..#.........#
.##.....##.....##.......#......
....##...##.......#..#.#.....#.
.##.........#......#........##.
.......#...#...###.#..#........
..#..###......##..##...........
.#..#......##..#.#.........#...
...#.......#........#...#.#....
...#....#..#....#.....##.......
............#......#..........#
.#.......#......#.#....#..#.#..
##.........#.#.#..........#....
....##.....#...................
.......#..#........#...........
....##.#..#......###.......#...
....#....#...#.#......#...#...#
.......#.....##..#....#...#....
#...#........#.........#..##...
...........##.........#.#...#..
....................#....#.##..
.#..#..#.........#....#..#..##.
......................#........
..###....#.......#.....###.##..
......#......#.......#.....#..#
.....#...#.##...#......#....#..
.....#.....##.............#....
....#......##..#....#.......#..
.##....#..##......###....#..#..
...###.#.............##...#.#..
.....#.....#.....#...#..#.#....
..#.#.....###......#.......#...
..........#.##......#.........#
..##..#.......................#
........#......#............#..
#..#..#..#.#......#..#....#....
...##......#.............#....#
...........#..#..##.......#....
.....#.........#.#..#..........
##...#.......#.#....#..#..#....
#.#.#...........#.##.#.#..###..
#..#...........#.........##....
............#.#..............#.
.#....#....##.#...........#..#.
....#...#..#...#....#....#.....
....#....#...#..#......#.......
.#.#.........#.......#.##......
.#..##...#........#...........#
##...#..#...#...#.....#...#....
....###.#..#.......##.#..#...#.
...##.......####...##.#........
#....#....#.#............#..#..
#.#.#...#...................##.
##......#...........#..........
#..#..#....#.#...#......#......
.##...#.....#...#........#.....
..#............#..............#
###........#..#....#...#......#
###..##......#.##...........#..
........#......#..#.....#......
...#..........#..#...........#.
....#..#..#....#........#....#.
.#.................#####..##..#
.....#...##..#..........#.##...
..#..............#...####......
.....#.##..................#.#.
...#.#..#..#........#..........
...........#....#.#..#.........
.....##.......#......#..#.#.#..
...#.............##...#........
...............#.......##.##.##
.....#........#........#.#..#..
...#..#.........#...##...###...
...#.#.............###.#.....#.
.#..........#......###.#.#.....
....##..##.............###.....
..#..#.#...##...#.......##.....
..........###........#.....#.#.
#.#....#..#..#......#...#...#..
.........#......##.......#.#..#
...#.....#.........##..#..#....
.....##.#..##.##..##...........
...#.#.##....#..#..#......#..#.
#....#....#.............#...##.
#......#..#.####.#.##.#....##..
##.#.#....##..................#
.....##......#.......##.......#
..#......#.#..#...##......##...
..#....##....#.........#..##...
.###.....#....##...........#...
.........#......#.#........#...
...#...#..#.#....######.#..#...
###......#.#.#.........##.#....
.....#...#.........#...#.......
....#.............#.#.........#
..##...#...#.......#......#....
.....#...#.#...#...#..#........
.#......#......................
...###..#..#....#...##.#.......
.#.#.....##...#...#.....#...##.
.....###..###....##............
.....##....#..#.....#.##.......
#........#.........#...#..#....
...#.#.........#..#.......#.#..
....#.#....##.....#..........#.
.#..#....#..#.#..#..#.........#
#...#....#..............#......
.........#.....#.##...##...###.
.....#....##............#..#...
.....#.#...........#..#....#...
.#..........#...#......#.....#.
.#...........#.....#..#........
..............#......##...#..#.
...#.........#..#....#..##...##
..##...#..................#....
#.....#.................#......
...#......#..#..........#.#....
......#..#.....#.....##...#..#.
......#........#..........#....
...##.##....#..##.#..........#.
..........#..#.#.##............
..##........................#..
.....#.#.#......#....#....##...
#....#.........#........#......
.##.......#...#...#........##..
....##......#....#.#..........#
..#.......#..............#.....
.....#......#.#...#..#.#.#....#
.....#..#........#.##.##.......
##........#..........#.........
.....#..##....#.#......###..##.
#.#...##.........#.#.....#..#..
#....#.#...#........#.....#..#.
........................#......
....###......#............#...#
...#..##......#..##.........#..
.............#...#......#..#..#
....#......#....#...........#..
..#.#.####.#.....##........#..#
#..#...#..#..#.......#.#..#....
..#..#..#....#.#.........##..#.
.......#......#.#............#.
...#.............#.#.....#.....
...#.#.........##...#.#.......#
........#...#...........##...#.
..........#....#......#....##..
..........#...........#........
...#..#...#..........#......#..
......#......#....#.....#..#.#.
........##.................#..#
.#........#.#...........#......
#...#........#.#.#.....#.#.#...
.........#........#..#..#....#.
##........#..........#....#..#.
.#.##...........#..#.#..##....#
.......#.#....#..#......#......
..#.....#........##..#......###
..#...#..................#....#
......#...#..#.##.......#......
........#...#.#................
.........#............#........
..#.....##....#.#..##..........
#.....#..........#....#........
....#.#...#...##....#.....##...
..#.#.......#.............#...#
...##..............#......#....
#......#...#................##.
.#.#...#.#..#..................
...##.......#...........#.#.#..
#......#.#.#........#.##...####
.......#..#.#.........#.#.##..#
..............#....#.........#.
...........#.#..#....##......#.
#.............#...##..#.......#
.........#............#...#.##.
.......#.........#.#.....#..#..
........................#.#.##.
#......#.#......#.........#....
...#.......#.......#.....#.....
#..#....#................#...#.
........#.#..##......#.........
#..#...##....##....##.........#
.......#...#...###.............
#.#..#........#.#.#............
#.....#........##.........#.#..
.#..........#....#.#....###....
.#.....#...#.#........#..#.##..
...#.##......#..#.............#
..##..#.#...................#..
.....#....#...#.#...#...#......
.....#..#.#....#.#.............
#.#....#.#.##..###..........#..
........#.#.............#..#...
.........#.......#.............
.##.#............##...#........
......#................#.......
...............#..#...........#
...#.......#...#.##.....#....#.
##..##..#..........#...........
.##.#.......#...#..#...#...#...
....#..#...........#....#.##...
.#........#........#....#......
.......#...#.##.#..#.#..#......
.#..#......#....#...##....#.#..
......#...##.#.....##.###.....#
.#....#..#......#...#.#.....#..
#............#....##...##.##...
#...#.#....#...#.......##...##.
#...........#.##..#....#.....#.
...#..#...#.........#.......#..
.#....#.....#............#.#..#
.#.....#.#...#.#....##......###
..#..#.#.#...#..#.............#
...#...#..#....#........#...##.
.......#.....#...##...........#
#.##.................#...##...#
..............##........#.....#
............#...#..#.......#.#.
#.#.....#.........#...#......#.
#.###..#......#..#..#...#.....#
.....#.......#.................
........#..#......#.#...#......
#.......#..#........#...#..#...
..#...#.......##.............#.
#.......#.......##...#.........
.........#....#.#..##.....#...#
..#.....#.#.......#....#.......
...#.......#.....#..##.#..#....
....#.......#.#.#..............
.#..#......#........#.#..##..##
....#...#.##.#...#....##...#...
#..##..#.....#.......#.........
....#..#..#.#............#.....
#.......##...##..##............
...............................
....#.......#.##...#.....#.#...
...#........#....#.#..#..#.....
##.......#.....##.#.#....#....#
#.............#...........#.##.
#...........#.#..........#.....
#..#....#....#.#.........#.#...
......#.#.#..#.#.#.............
...#.....#........##....#......
..#...#...#.#.......#......#...
.##........#...#..#..........#.
..#...........#..##.....##.....
............#..#.#...#.....#...
..........#....##.......#......
....#....#.................#..#
....#...............#.........#
..#.#...#......#..........##...
.....#...........#.........#..#
.......#.....##.....#.#........
.#.#..........#....#...........
.#..##....#........#....#......
....#.#..#.......#..#.........#
..#....#.....#......#..#.......
......#........#.......#...#.#.
.......#.......#....#.....##...
....##........#..#...#.#..#...#
.#......#...........##....#....
##....##......#.......#.......#
.##....#.##......#.......##..#.
...#..#.#.#.......#..#.###.....
..........##....#..#.##........
...#........###.#..#........#..
.....#....#..##....#.....#....#
#..........#..........#.#....#.
..#....#.....#..............#..
#..................#......#.##.
.#...#.#.....#.........##......
...#...........#.....#......#..
......#.....#.#..##......##....
...#....###..#.....#..#..##..##
......#.......##..#..#.........
#..#.#....#.#..#..........##.#.
..#..#..##..#.#.#.#.....#......
..#.#...#..#.....###.#.........
##.#.#......#........#.####....
.............#..#..#....#......
...##..........#.......#.#....#
..#.....................#......
..#..#...##...#.##........#....)";

auto const m = parse_trees(input);

fmt::print("m is {} x {} with {} trees\n", m.rows(), m.cols(), m.count(true));

auto const r11 = slope_trees(m, 1, 1);
fmt::print("trees encountered by moving ({}, {}): {}\n", 1, 1, r11);
auto const r31 = slope_trees(m, 3, 1);
fmt::print("trees encountered by moving ({}, {}): {}\n", 3, 1, r31);
auto const r51 = slope_trees(m, 5, 1);
fmt::print("trees encountered by moving ({}, {}): {}\n", 5, 1, r51);
auto const r71 = slope_trees(m, 7, 1);
fmt::print("trees encountered by moving ({}, {}): {}\n", 7, 1, r71);
auto const r12 = slope_trees(m, 1, 2);
fmt::print("trees encountered by moving ({}, {}): {}\n", 1, 2, r12);

fmt::print("product is {}\n", r11*r31*r51*r71*r12);

}