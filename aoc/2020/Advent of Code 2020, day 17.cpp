// Advent of Code 2020: day 17

// Godbolt link: https://godbolt.org/z/sfbTj3

// NB: kind-of spaghetti code, very improvable...

#include <algorithm>
#include <cassert>
#include <string_view>
#include <utility>
#include <vector>
#include <fmt/core.h>

auto constexpr active = '#';
auto constexpr inactive = '.';

std::string_view fetch_line(std::string_view& text) {
  auto const pos = text.find('\n');
  auto res = text.substr(0, pos);
  text.remove_prefix(pos+1);
  return res;
}

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
 
  void add_row(T const& value) {
    auto const idx = data_.size();
    data_.resize(data_.size() + cols_);
    std::fill_n(std::next(begin(data_), idx), cols_, value);
  }

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

  void set(std::size_t row, std::size_t col, T value) noexcept {
    data_[idx(row, col)] = std::move(value);
  }

  std::size_t count(T const& value) const noexcept {
    return std::count(begin(data_), end(data_), value);
  }

private:
  std::size_t idx(std::size_t row, std::size_t col) const noexcept {
    return row*cols_ + col;
  }
  auto iterator(std::size_t row, std::size_t col) const noexcept {
    return std::next(begin(data_), idx(row, col));
  }

};

void print(Matrix<bool> const& m, std::pair<char, char> const repr = {active, inactive}, std::string_view const at_the_end = "\n") {
  for (std::size_t i = 0; i < m.rows(); ++i) {
    for (std::size_t j = 0; j < m.cols(); ++j) {
      fmt::print("{}", m.at(i, j) ? repr.first : repr.second);
    }
    fmt::print("\n");
  }
  fmt::print("{}", at_the_end);
}

template<class T>
class Cube {
  std::vector<Matrix<T>> slices_;
public:
  Cube(std::size_t r, std::size_t c, std::size_t s, T value) {
    slices_.reserve(s);
    for (std::size_t i = 0; i < s; ++i) { // s-times
      slices_.push_back(Matrix<T>(r, c, value));
    }
  }

  std::size_t slices() const noexcept {
    return slices_.size();
  }
  std::size_t rows() const noexcept {
    return slices_.front().rows();
  }
  std::size_t cols() const noexcept {
    return slices_.front().cols();
  }

  T at(std::size_t row, std::size_t col, std::size_t s) const noexcept {
    return slices_[s].at(row, col);
  }

  void set(std::size_t row, std::size_t col, std::size_t s, T value) noexcept {
    slices_[s].set(row, col, value);
  }

  std::size_t count(T const& value) const noexcept {
    std::size_t res = 0;
    for (auto const& m : slices_) { // accumulate
      res += m.count(value);
    }
    return res;
  }

  void add(Matrix<T> slice) {
    slices_.push_back(std::move(slice));
  }

  Matrix<T> const& slice(std::size_t i) const {
    return slices_[i];
  }

};

Cube<bool> parse_reserve_for(std::string_view s, int const t) {
  auto line = fetch_line(s);
  std::vector<bool> buffer(2*t + line.size(), false);
  Matrix<bool> slice(t, buffer.size(), false);
  auto const add_line = [&](auto& m, auto const line) {
    for (std::size_t i=0; i < line.size(); ++i) { // transform
      buffer[i+t] = line[i] == active;
    }
    m.add_row(begin(buffer), end(buffer));
  };
  add_line(slice, line);
  while(!s.empty()) {
    add_line(slice, fetch_line(s));
  }
  std::fill(begin(buffer), end(buffer), false);
  for (std::size_t i=0; i < t; ++i) { // t times
    slice.add_row(begin(buffer), end(buffer));
  }
  auto const r = slice.rows();
  auto const c = slice.cols();
  Cube<bool> res(r, c, t, false);
  res.add(std::move(slice));
  for (std::size_t i=0; i < t; ++i) { // t times
    res.add(Matrix<bool>(r, c, false));
  }
  return res;
}

template<class T>
std::size_t count_neighbor(Cube<T> const& cube, std::size_t r, std::size_t c, std::size_t s, T const& value) {
  auto const R = cube.rows()-1;
  auto const C = cube.cols()-1;
  auto const S = cube.slices()-1;
  auto res = 0;
  for (auto di : {-1, 0, 1}) {
    for (auto dj : {-1, 0, 1}) {
      for (auto dk : {-1, 0, 1}) {
        auto const i = r + di;
        auto const j = c + dj;
        auto const k = s + dk;
        if (0 <= i && i <= R
         && 0 <= j && j <= C
         && 0 <= k && k <= S
         && cube.at(i, j, k) == value) ++res;  
      }   
    }    
  }
  if (cube.at(r, c, s) == value) --res; // undo {di, dj, dk} == 0
  //fmt::print("({},{}) has {} neighbors {}\n", i, j, res, value);
  return res;
}

std::size_t evolve(Cube<bool>& c) {
  auto constexpr Active = true;
  auto res = 0;
  auto update = c;
  for (std::size_t i = 0; i < c.rows(); ++i) {
    for (std::size_t j = 0; j < c.cols(); ++j) {
      for (std::size_t k = 0; k < c.slices(); ++k) {
        auto const neigh = count_neighbor(c, i, j, k, Active);
        if (c.at(i, j, k) == Active) {
          if (neigh < 2 || 3 < neigh) {
            update.set(i, j, k, !Active);
            ++res;
          }
        }
        else {
          if (neigh == 3) {
            update.set(i, j, k, Active);
            ++res;
          }
        }
      }
    }
  }
  c = std::move(update);
  return res;
}

template<class T>
void evolve(T& obj, std::size_t n) {
  for (std::size_t i = 0; i < n; ++i) { // n times
    evolve(obj);
  }
}

template<class T>
class HyperCube {
  std::vector<Cube<T>> cubes_;
public:
  HyperCube(std::size_t r, std::size_t c, std::size_t s, std::size_t h, T value) {
    cubes_.reserve(h);
    for (std::size_t i = 0; i < h; ++i) { // h-times
      cubes_.push_back(Cube<T>(r, c, s, value));
    }
  }

  std::size_t high() const noexcept {
    return cubes_.size();
  }
  std::size_t slices() const noexcept {
    return cubes_.front().slices();
  }
  std::size_t rows() const noexcept {
    return cubes_.front().rows();
  }
  std::size_t cols() const noexcept {
    return cubes_.front().cols();
  }

  T at(std::size_t row, std::size_t col, std::size_t s, std::size_t h) const noexcept {
    return cubes_[h].at(row, col, s);
  }

  void set(std::size_t row, std::size_t col, std::size_t s, std::size_t h, T value) noexcept {
    cubes_[h].set(row, col, s, value);
  }

  std::size_t count(T const& value) const noexcept {
    std::size_t res = 0;
    for (auto const& m : cubes_) { // accumulate
      res += m.count(value);
    }
    return res;
  }

  void add(Cube<T> cube) {
    cubes_.push_back(std::move(cube));
  }

  Cube<T> const& cube(std::size_t i) const {
    return cubes_[i];
  }

};

template<class T>
std::size_t count_neighbor(HyperCube<T> const& cube,
                           std::size_t r, std::size_t c, std::size_t s, std::size_t h, T const& value) {
  auto const R = cube.rows()-1;
  auto const C = cube.cols()-1;
  auto const S = cube.slices()-1;
  auto const H = cube.high()-1;
  auto res = 0;
  for (auto di : {-1, 0, 1}) {
    for (auto dj : {-1, 0, 1}) {
      for (auto dk : {-1, 0, 1}) {
        for (auto dl : {-1, 0, 1}) {
          auto const i = r + di;
          auto const j = c + dj;
          auto const k = s + dk;
          auto const l = h + dl;
          if (0 <= i && i <= R
           && 0 <= j && j <= C
           && 0 <= k && k <= S
           && 0 <= l && l <= H
           && cube.at(i, j, k, l) == value) ++res;  
        }
      }   
    }    
  }
  if (cube.at(r, c, s, h) == value) --res; // undo {di, dj, dk, dh} == 0
  return res;
}

std::size_t evolve(HyperCube<bool>& c) {
  auto constexpr Active = true;
  auto res = 0;
  auto update = c;
  for (std::size_t i = 0; i < c.rows(); ++i) {
    for (std::size_t j = 0; j < c.cols(); ++j) {
      for (std::size_t k = 0; k < c.slices(); ++k) {
        for (std::size_t l = 0; l < c.high(); ++l) {
          auto const neigh = count_neighbor(c, i, j, k, l, Active);
          if (c.at(i, j, k, l) == Active) {
            if (neigh < 2 || 3 < neigh) {
              update.set(i, j, k, l, !Active);
              ++res;
            }
          }
          else {
            if (neigh == 3) {
              update.set(i, j, k, l, Active);
              ++res;
            }
          }
        }
      }
    }
  }
  c = std::move(update);
  return res;
}

//////////////////////////////////////////////////////////////////////

int main() {

auto const test = R"(.#.
..#
###
)";

auto const input = R"(##.#####
#.##..#.
.##...##
###.#...
.#######
##....##
###.###.
.#.#.#..
)";

auto const t = 6;

auto const source = parse_reserve_for(input, t);
/*
for (std::size_t i=0; i < source.slices(); ++i) {
  fmt::print("{}. ---------------------------------------\n", i);
  print(source.slice(i));
}
*/
auto cube = source;
evolve(cube, t);
/*
for (std::size_t i=0; i < cube.slices(); ++i) {
  fmt::print("{}. ---------------------------------------\n", i);
  print(cube.slice(i));
}
*/
auto const res1 = cube.count(true);
fmt::print("After {} evolutions {} are active\n", t, res1);

auto hypercube = HyperCube{source.rows(), source.cols(), source.slices(), t, false};
hypercube.add(source);
for (std::size_t i=0; i<t; ++i) { // t times
  hypercube.add(Cube<bool>(source.rows(), source.cols(), source.slices(), false));  
}
evolve(hypercube, t);
auto const res2 = hypercube.count(true);
fmt::print("After {} evolutions {} are active\n", t, res2);

}