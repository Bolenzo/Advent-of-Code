// Advent of Code 2020: day 11

// Godbolt link: https://godbolt.org/z/rrWbb3

#include <algorithm>
#include <cassert>
#include <string_view>
#include <utility>
#include <vector>
#include <fmt/core.h>

auto constexpr occupied = '#';
auto constexpr empty = 'L';
auto constexpr floor = '.';

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

void print(Matrix<char> const& m, std::string_view const at_the_end = "\n") {
  for (std::size_t i = 0; i < m.rows(); ++i) {
    for (std::size_t j = 0; j < m.cols(); ++j) {
      fmt::print("{}", m.at(i, j));
    }
    fmt::print("\n");
  }
  fmt::print("{}", at_the_end);
}

Matrix<char> parse(std::string_view s) {
  auto const n = std::min(s.size(), s.find('\n'));
  Matrix<char> res(0, n, floor);
  for (auto it = begin(s); it != end(s); it += n + 1) {
    res.add_row(it, std::next(it, n));
  }
  return res;
}

template<class T>
std::size_t count_neighbor(Matrix<T> const& m, std::size_t i, std::size_t j, T const& value) {
  auto const R = m.rows()-1;
  auto const C = m.cols()-1;
  auto res = 0;
  if (i > 0 && j > 0) res += m.at(i-1, j-1) == value;
  if (i > 0         ) res += m.at(i-1, j  ) == value;
  if (i > 0 && j < C) res += m.at(i-1, j+1) == value;
  if (         j > 0) res += m.at(i  , j-1) == value;
  if (         j < C) res += m.at(i  , j+1) == value;
  if (i < R && j > 0) res += m.at(i+1, j-1) == value;
  if (i < R         ) res += m.at(i+1, j  ) == value;
  if (i < R && j < C) res += m.at(i+1, j+1) == value;
  //fmt::print("({},{}) has {} neighbors {}\n", i, j, res, value);
  return res;
}

std::size_t evolve_1(Matrix<char>& m) {
  auto res = 0;
  auto update = m;
  for (std::size_t i = 0; i < m.rows(); ++i) {
    for (std::size_t j = 0; j < m.cols(); ++j) {
      switch (m.at(i, j)) {
        case empty: {
          if (count_neighbor(m, i, j, occupied) == 0) {
            assert( update.at(i, j) == empty );
            update.set(i, j, occupied);
            ++res;
          }
          break;
        }
        case occupied: {
          if (count_neighbor(m, i, j, occupied) >= 4) {
            assert( update.at(i, j) == occupied );
            update.set(i, j, empty);
            ++res;
          }
          break;
        }
        case floor: break;
      }
    }
  }
  m = std::move(update);
  return res;
}

template<class T>
bool contains_before_direction(Matrix<T> const& m, T const& value, T const& ignore,
                               int i, int j, int h_step, int v_step) {
  auto const is_valid = [&](auto const& p) {
    auto const i = p.first;
    auto const j = p.second;
    return 0 <= i && i < m.rows()
        && 0 <= j && j < m.cols();
  };
  auto const advance = [&](int& i, int& j) -> std::pair<int, int> {
    i += h_step;
    j += v_step;
    return {i, j};
  };
  for (; is_valid(advance(i, j));) {
    auto const v = m.at(i, j);
    if (v == value) return true;
    if (v != ignore) break;
  }
  return false;
}

template<class T>
std::size_t count_neighbor_ignore(Matrix<T> const& m, std::size_t i, std::size_t j,
                                  T const& value, T const& ignore) {
  auto res = 0;
  res += contains_before_direction(m, value, ignore, i, j, +1, +0);
  res += contains_before_direction(m, value, ignore, i, j, +1, +1);
  res += contains_before_direction(m, value, ignore, i, j, +0, +1);
  res += contains_before_direction(m, value, ignore, i, j, -1, +1);
  res += contains_before_direction(m, value, ignore, i, j, -1, +0);
  res += contains_before_direction(m, value, ignore, i, j, -1, -1);
  res += contains_before_direction(m, value, ignore, i, j, +0, -1);
  res += contains_before_direction(m, value, ignore, i, j, +1, -1);
  //fmt::print("({},{}) has {} neighbors {}, ignoring {}\n", i, j, res, value, ignore);
  return res;
}

std::size_t evolve_2(Matrix<char>& m) {
  auto res = 0;
  auto update = m;
  for (std::size_t i = 0; i < m.rows(); ++i) {
    for (std::size_t j = 0; j < m.cols(); ++j) {
      switch (m.at(i, j)) {
        case empty: {
          if (count_neighbor_ignore(m, i, j, occupied, floor) == 0) {
            assert( update.at(i, j) == empty );
            update.set(i, j, occupied);
            ++res;
          }
          break;
        }
        case occupied: {
          if (count_neighbor_ignore(m, i, j, occupied, floor) >= 5) {
            assert( update.at(i, j) == occupied );
            update.set(i, j, empty);
            ++res;
          }
          break;
        }
        case floor: break;
      }
    }
  }
  m = std::move(update);
  return res;
}

template<class Evolution>
void evolve_until_stable(Matrix<char>& m, Evolution&& e) {
  for (; e(m) > 0; ) /*print(m)*/;
}

//////////////////////////////////////////////////////////////////////

int main() {

auto const test = R"(L.LL.LL.LL
LLLLLLL.LL
L.L.L..L..
LLLL.LL.LL
L.LL.LL.LL
L.LLLLL.LL
..L.L.....
LLLLLLLLLL
L.LLLLLL.L
L.LLLLL.LL
)";

auto const input = R"(LLLLL.LLLLLLLL..LLLLLLLLLLLLLL.LLLL..LL..LLLLLLLL.LLLL.LLLLLLLLLLLL.LLLLLL.LLLLLL.LLLLLLLL
LLLLL.LLLLLLLL.LLLLLL.LLLLLLLL.LLLLLLLLL.LLL.LLLL.LLLLLLLLLLLLLLLLLLLLLLLL.LLLLLL.LLLLLLLL
LLLLLLLLLLLLLLLLLLLLL.LLLLLLLL.LLLLLLL.L.LLLLL.LLLLLLL.LLLLLLLLLLLLLLLLLLLLLLLLLL.L.LLLLLL
LL.LLLLLLLLL.L.LLLLLL.LLLLLLLL.LLLL.LLLL.LLLLLLLL.LL.L.LLLL.LLLL.LLLLLLLLLLLLLLLLLLLLLLLLL
LLLLL.LLLLLLLL.LLLLLL.LLLLLLLLLLLLLLLL.L.LLLLLLLL.LLLL.LLLLLLLLL..LLLL.LLLLLL.LLL.LLLL.LLL
....L...............L...L...L.....L..L.LLLL..L.L...L...L.L.L..L.....LL..LL........L.LLLL..
LLLLL.LLLLLLLLLLLLLLL.LLLLLLLL.LLLLLLLLL.LLLLL.LL.LLLL.LLLL.LL.L.LLL.LLLLLLLLLLLL.LLLLLLLL
LLLLLLLLLLLLLL.L.LLLL.LLLLLLLL.LLLLLLLLLLLLLLLLLL.L.LLLLLLLLLLLL.LLLLLLLLLLLLLLLLLLLLLLLLL
LLLLL.LLLLLLLL.LLLLLLLLLLLLLLL.LLLL.LLLLLLLLLLL.LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL.LLLLLLLL
LLLLLLLLLLLLLL.LLLLLL.LLL.LLLLLLLLL.L.LLLLLLLLLLL.LLLL.LLLLLLLLL.LLLLLLLLL..LLLLL.LLLLLLLL
LLLLLLLLLLLLLL.LLLLLL.LLLLLLLL.LLLL.LLLL.LL.LLLLL.LLLL.LLLLLLLLL.LLLLLLLLL.LLLLLLLLLL.LLLL
LLLLL.LLLLLLLLLLLLLLL.LLLLLLLL.LLLL.LLLL.LLLLLLLLLLLLL.LLLLLLLLL.LLLLLLLLLLLLLLLL.LLLLLLLL
L..L..L.LLL...L.L.....L....L.LL..........L.L....LLL...L...L.LLL.L...L...L......L..L..L....
LL.L.L..LLLL.LLLLLLLLLLLLLLLLL.LLLL..LLL.LLLLLLLL.LLLL.LLLLLLLLLL.LLLLLL.L.LLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLL.LLLL.LLL..LLLLLLLLLLLLLLLL.LLLLLL.LLL.LLL.L.LLLLLL.LLLLLLLL
LLLLLLLLL.LLLLLLLLLLLLLL.LLLL..LLLLLLLLL.LLLLLLLL.LLLLLLLL.LLLLL.LLLLLLLLLLLLLLLLLLLLLLLLL
LLLLL.LLLLLLLLLLLLLLL.LLLLLLLL.LL.LLLLLL.LLLLLLLL.LLLL.LLLLLLLLL.LLLLLLLLL.LLLLLL.LL.LLLLL
LLL.L.LLLLLLLL.LLLLLL.LLLLLLLLLLLLL.LLLL.LLLLLLLL.LLLL.LL.LLLL.L.LLLL.LLL.LLLLLLL.LLLLLLLL
LLLLL.LLLLLLLL.LLLLLL.LLLLLLLL.LLLL.LLLL.LLLLLLLLLLLLL.LLLLLLLLLLLLLLLLLLLLLLLLLL.LLLLLLLL
.LLLL.LLLLLLLLLLLLLLL.LLLLLLLL.LLLL.LLLL.LLLLL.LLLLLLLLLLLLLLLLL.LLLLLLLLL.LLLLLL.LLLLLL.L
LL....LL.L.L....L...........L........L...L.L.L.L.L.L.L...LLLL....L......L..LL.L...L...L..L
LLLLL.LLLLLLLL.LLLLLL.L.LLLLLL.LLLLLLLLLLLLLLLLLLLL.LL.LLLLLLLLLLLLLLLLLLL.LLLLLL.LLLLLLLL
LL.LL.LLLLLLLL.LLLLLL...LLLLLL.LLLL.LLLL.LLLLLLLL.LL.L.LLLLLLLLLLLLLLLLLLL.LLLLL..LLLLLLLL
LLLLL.LLL.LLLL.LL.LLLLLLLLLLLL.LLLLLLLLL.LLLLLLLL.LLLL.LLLLLLLLLLLLLLLLLLL..LLL.LLL.LLLLLL
LLLL.LLLLLLLLL.LLLLLL.LLLLLLLL.LLLLLLLLL.LLLLLLLLLLLLLLLLLLLLLLL.LLLLLLLLL.LLLL.L.LLLLLLLL
LLLLLLLLLLLLLL.LLLLLL.LLLLLLLLLLLLL.LLLLLLLLLLLLL.LLLL.LLLLLLLLL.LLLLLLLLLLLLL.LL.L.LLLLLL
.LLLLL..L.LL....L..LL.LLL.L.....LLL.LL..........L..L..LL..L..L.L..L.L.L..L.L...........L.L
LLLLL.LLLLLLLL.LLLLLLLL.LLLLLL.LLLLLLLLL.LLLLLLLLLLLLL.LLLLLLLLL.LLLLLLLLL.LLLLLL.LLLL.LLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLL.LLL..LLLLLLLLLLLLLLLLLL.LLLLLL.LLLLLLLLLLLL.LLLLLLLL.LLLLLL
LLL.L.LLLLLLLL.LLLLLLLLLLLLL.L.LLLL.LLLLLLLLLLLLL.LLLL.L.LLLLLLLLLLLLLLLLL.LL.LLL.LLLLLLLL
LLLLLLLLLLLLLLLLLLLLL.LLLLLLLLLLLLL.LLLL.LLLLLLLL.LLLL..LLLLLLLL.LLLLLLLLL.LLLLLL.LLLLL.LL
LLLLLLLLL.LLLL.LLLLLL.LLLLLLLL.LLLLLLLLLLLLLLLLLL.LLLL.L.LLLLLLL.LLLLLLLLLL.LLLLL.LLLLLLLL
LLLLL.LLLLLLLLLLLLLLL.LLLLLLLLLLLLLLLLLL.LLLLLLLL.LLLL.LLLLLLLLLLLLL.LLLLL.LLLLLL.LLLL.LLL
LLLLL.LLLLLLLL.LLLLLL.LLLLLLLLLLLLL.LLLL.LLLLLLLL.LLLL.LLLL.LLLLLLL.LLLLLL.LLLLLL.LLLLLLLL
LLL.LLLLL....L..L..L....L.L..L....L..LL..L.........L......L..L....LL...L.L...LLLLL.....L.L
.LLLL.LLLLLLLLLLLLLL.LLLLLLLLL.LLLLLLLLL.LLLLLLLL.LL.L.LLL.LLLLL.LLLLLLLLLLLLLLLLLLLLLLLLL
LLL.LLLLLLLLL.LLLLLLLLLLLLLLLL.LLLL.LLLL.LLLLLLLL.LLLL.LLLLLLLLLLLLLLLLLLL.LLLLLL.LLLLLLLL
LLLLLLLLLLLLLL.LLLLLL.LLLLLLLLLLLLLLLLL..LLLLLLLL.LLLL.LLLLLLL.L.LLLLLLLLL.LLLL.L.LLL.LLLL
LLLLL.LLLLLLLL.LLLLLL.LLLLLLLLLLLLLLLLLLLLLLLLLLL.LLLL.LLLLLLL.L.LLLLLLLL.LLLLLLL..LLLLLLL
....L...LL....L..L..L.L......LLL........L.LL..L.L...L...L..L....LLL..LL.LLL.LLL.L..L....L.
LLLLLLLLLLLLLL.LLLLLL.LLLLLLL..LLLL.LLLL..LLLLLLLLLL.LLLLLLLLLLL..LLLLLL.LLLLLLLL.LLLLLLLL
LLLLLLLLL.LLLLLLLLLLL.LLLLLLLLLLLLL.LLLL.LLLLLLLL.LLLL.LLLL.LLLL.LL.LLLL.L.LLL.LL.LLLL.LLL
LLLLL.LL.LLLLL.LLLL.L.LLLLLLLL.LLLL.L.LLLLLLLLLLL.LLLL.LLLLLLLLL.LLLLLLLLL.LLLLLL.LLLLLLLL
LLLLL.LLLLLLLLLLLLLLL.LLLLLLLL.LLL.LLLL..LLLLLLLL.LLLL.LLLLLLLLLLLLLLLLLLL.LLLLLL.LLLLLLLL
L.LLL.LLLLLLLL.LL.LLLLLLLLLLLL.LLLL.LLLL.LL.L.LLL.LLLL.LLLLLLLL..LLLLLLLLL.LLLLLL.LLLLLLLL
...LLLL..L...LLLL..LL...L.......L..LLL....L..........LL.L...LL.L.....L.....LL..L......LLLL
L.LLL.LLLLLLLLLLLLLLL.LLLLLLLL.LLLLLL.LL.LLLLLLLLLLLLLLLLLLLL.LLLLL.LLLL.LLLLLLLL.LLLLLLLL
LLLLL.LLLLLLLL.LLLLLL.LLLLLLLLLLLLL.LLLL..LLLLLLL.L.LL.LLLLLLL.LLLLLLLLLLL.LLLLLLLLLLLLLLL
LLLLL.LLLLLLL..LLLLLLLLLLLLLLL.LLLLLLLLL.LLLLLLLL.LLLL.L.L...LLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLL.LLLLL..LLLLLLLLLLLLLLLLLLLL.LLLL.LLLLLLLLLLLLLLLLLLLLLLL.LLLLLLLLLLLLLLLLLLLLLLLLL
....LLL.L.......LL.....LL..L...LL.LLL......L..L..LL..LL.LL.L..L.LL.....L.L.L.L.....L..L...
LLL.L.LLLLLLLL.LL.LLL.LLLLLLLL.LLLL.LLLLLLLLLLLLL.LLLLLLLL.LLLLL.LLLLLLLLLLLLLLLLLLLLLLLLL
LLLLL.LLLLLLLL.LLLLLLLLLLLLLLL.LLLL.LLLLLLLLLLLLL.LLL..LLLLLLLLLLLLLLLLLLL.LLLLLL.LLLLLLLL
LLLLL.LL.LLLLL.LLLLLL.LLLLLLLLLL.LL.LLLL.LLLLLLLLLLLLLLLLLLLLLLL.LLLLLLLLL.LLLLLL.LLLLLLLL
LLLLL.LLLLLLLL.LLLLLL.LLLL.LLLLLLLLLLLLLLLLLLLLLL.LLLL.LLLLLLLLL.LLLLLLLLL.LLLLLL.LLLLLLLL
LLLLL.LLLLLLLLLLLLLLL.LLLLLLLL.LLLLLLLLL.LLLLLLL..LLLL.LLLLLLL.LLLLLLLLLLLLLLLLLLLLLLLL.LL
LLLLL.LLLLLLLL.LLL.LLLLLLLLLLL.LLLLLLLLLLLLLLLLLL.LLLL.LLLLLLLLL.LLLLLLLLL.LLLLLL.LLLLLLLL
LLLLLLLLLLLLLL.LLLLLL.LL.LLLLL.LLLLLLLLL.LLLLLLLL.LLLL.LLLLLLLL..LLLLLLLL..LL.LLL.LLLLLLLL
LLL..L.L.L..........LL....L...L.L.LLL.L.L..L.....L...L.LLL............L.L.L..L..........L.
LLLLL.LLLLL..L.LLLLLL.LLLLLLLL.LLLL.LLLL.LLLLLLLLLLLLL.LLLLLLLLL.LL.LLLLLLLLLLLLLLLLLLLLLL
L.LLL.LLLLLLLL.LLLLLL.LLLLLLLL.LLLLLLL.L.LLLLLLLLLLLLL.LLLLLLLLL.LLLLLLLLLLLLLL.L.LLLLLLLL
LLLL..LLLLLLLLLLLLLLLLLLLLL.LL.LLLL.LLLL.LLLLLLLL.LLLLLLLLLLLLLLLL.LLLL.LL.LLLLLLLLLLLLLLL
LLLL..LLLLLLLL.LLLLLL.LLLLLLLL.LLL.L.LLL.LLLLLLLLLLLLL.LLLLLLLLLLLLL.LLLLL.LLLLLL.LLLLLLLL
LLLLLLLLLLLLLLLLLLLLL.LLLLLLLL.LLLLLLLLL.LLLLLLLL.LLLLLLLLLLLLLLLLLL.LLLLLLLLLLLL.LLLL.LLL
.LLLL.LLL.LLLLLLLLLLL.LLLL.LLL..LLLLLLLL.LLLLLLLLLLLLL.LL.LLLLLLLLLLLLLLLLLLLLL.L.LLLLLLLL
LL.LL.LLLLLLLL.LLLLLLLLLLLLLLL.LLLL.LLLLLLLLLLLLLLLLLL.LLLLLLLLL.LLLLLLLLLL.LLLLL.LL.LLLLL
LLLLL.LLLLLLLL.LLL.LL..LLLLLLL.LLLL.LLLL.LLLLLLLL.LLLLLLLLLLLLLL.LLLLLLLLL.LLLLLLLL.LLLLLL
...L.L.........L............L...LLL.L.L.L....L..LL.LL...........L.LL.L.....LL.L.L..L..L...
LLLLL.LLLLLLLL.LLLLLL.LLLLLLLLLLLLL.LL.L.LLLLLLLL.LLLLLLLLL.L.LLLLLLLLLLLL.LLLLLL.LLLLLLLL
LLLLLLLLLLLL..LLLLLLLLLL.LLLLL.LLLL.LLLL.LLLLLLLL.LLLL.LL.LLLLLLLLLLLLLLLL.LLLLLL.LLL.LLLL
LLLLLLLLLLLLLL.LLLLLLLLLLLLLLL.L.LL.LLLL.LLLLLLLLLLLLLLLLLLLLLLL..LLLLLLLL.LLLLLLLLLLLLLLL
LLLLL.LLLLLLLLLLLLLLL.LLLLLLLL.LLLL.LLLL.LL..LLLL.LLLLLLLLLLLLLL.LLLLLLLLL.LLLLLL.L.LLLL.L
LLLLLLLLLL.LLL.LLLLLL.L.LLLLLL.LLLL.LLLL.LLL.LLLL.LLLLLLLLLLLLLL.LLLLLLLLL.LLLLLLLLLLLLLLL
LLLLL.LLLLLLLL.LLLLLL.LLLLLLLL.LLLLLLLLLLLLLLLLLL.LLLL.LLLLLLLLL.LLLLLL.LL.LLLLLL.LLLLLLLL
LLLLL.LLLL.LLL.LLLLLLLLLLLLLLL.LLLL.LLLL.LLLLLLLLLLLLL.LLLLLLLLL.L.LLLLLLL.LLLLLL.LLLLLLLL
L.LLLLLLLLLLLL.LLLLLL.LLLLLL.L.LLLLLLLLL.LL.LLLLL.LLLL.LLLLL.LLLLLLL.LLLLLLLLLLLLL.LLLLLLL
.......L.LL.LLL.L.L.....LL.LL.L....L...LL...L......L..L.L..L...L.....L.LL.L.L.LL.L.L......
LLLLL.LLLLLLLL.LLLLLL.LLLLLLLL.LLLL.LLLLLLLLLLLLL.LLLL.LLLLLLLLL.LLLLLLLLL.LLLLLL.LLL.LLLL
LLLLL.LLLLLLLLLLLLLLL.LLLLLLLL.LLLL.LLLLLLLLLLLLL.LLLL.LLL.LLLLL.L.LLLLLLL..LL.L.LLLLLLLLL
LLLLL.L.LLLLLL.LLLLLLLLLLLL.LLLLLLL.LLLLLLLLLLLLL.LLLLLLLLLL.LLL.LLLLLLLLL.LLLLLLLLLLL.LLL
LLLLLLLLLLLLLL.LLLLLL.LLLLLLLL.LLL...LLL.LL.LLLLL.LLLL.LLLLLLLLL..LLLLLLLL.LLLLLL.LLLLLLLL
LLLLLLLLLLLLLL.LLLLLL.LLL.LLLLLLLLLLLLLL.LLLLLLLL.LLLL.LLLL..LLL.LLLLLLLLL.LLLLLL.LLLLLLLL
LLLLL.LLLLLLLL.LLLLL..LLL..LLL.LLLLLLLLL.LLLLLLLL.LLLLLLLLLLLLLLLLLLLLLLLL.LLLLLL.LLLLLLLL
LLLLL.LLLLLLLL.LLLLLL.LL.LLLLLLLLLLLLLLL.LLLLLLLL..LLLLLLLLLLLLL.LLLLLLLLLLLLLLLL.LLLLLLLL
...LL.L..L............LL.LL.....L.L.L..LLLL.L....L.L...L..LL.....LL.LL.L.L..LL..L..LL.L.L.
LLLLLLLLLLLLLL.LLLLLL.LLLLLLLL.LLLL.LLLL.L.LLLLLLLLLLLLLLLLLLLLL.LLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLL.LLLLLLLLLLLLL.LL.L.LLLLLLLL.LLLL.LLLLLLLLLLLLLLLLLLL.LLLLLLLLLLLLLLL
LLLLLLL.LL.LLL.LLLLLLLLLLLLLLL.LLLL..LLL.LLLLLLLLLLLLLLLLLLLLLLL..LLLLLLLLLLL.LLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLL.L.LLLLLLLL.LLLLLLLLLLLLLLL.LLLLLLL.LLLLLLLLL.LLLLLLLLL.LLLLLLLLLLLLLLL
LLLLL..LLLLLLL.LLLLLL.LLLLLLLLLLLLL.LLLL.LLLLLLLLLLLLLLLLLLLLLLLLLLLL.LL.L..LLLLL.LLLLLLLL
LLLLLLLLLLLLLLL.LLLLL.LLLLLLLL..LLL.LLLL.LLLLLLLLLLLLLLLLLLLLLLL.LLLLLLLLL.LLLLLLLLLLLLLLL
LLL.L.LLLLLLLL.LLLLLLLLLLLLLLLLLLLLLLLLL.L.L.LLL.LLLLL.LLLLLLLLL.LLLLLLLLL.LLLLLL.LLLLLLLL
LLLLL.LLLLLLLL.LLLLLLLLLLLLLLL.LLLL.LLLL.LLLLLLLLLLLLL...LLLLLLL.LLLLLLLLL.LLLLLL.LLLLLLLL
.LLLL.LLLLLLLLLLLLLL..LLLLLLL.LLLLL.LLLL.LLLLLLLLLL.LL.LLLLLLLLLLLLLLLLLLL.LLLL...LLLLLLLL
LLLLLLLLLLLLLLLLLLLL..LLLLLLLL.LLLLLLLLL.LLLLLLLLLLLLLLLLLLLLLLL.LLLLLLLLL..LLLLL..LLLLLLL
LLLLLLLLLLLLLL.LLLLLL.LLLLLLLLLLLLLLLLLL.LLLLL.LL.LL.L.LLLLLLLLL.LLLLLLLLL.LLLLLLLLLLLLLLL
LL.LL.LLLLLLLL.LLLLLL.LLLLLLLL.LLLL.L.LL.LLLLLLLL.LLLL.LLLLLLL...LLLLLLLLL.LLLLLL.LL.LLLLL
LLLLL.LLLLLLLL.LLLLLL.LLLLL.LLLLLLL.LLLLLLLL.LLLLLLLLL.LLLLLLLLL.LLLLLL..L.LLLLLL.LLLLLLLL
LLLLL.LLLLLLLL.LLLLLLLLLL.LLLLLLLLL.LLLL.LLLLLLLL.LLL..LLLLLLLLL.LLLLLLLLL.LLLLLL.LLLLLLLL
)";

auto const m = parse(input);
fmt::print("m is {} by {} ({} cells)\n",
  m.rows(), m.cols(), m.rows()*m.cols());
fmt::print("initially it has {} empty, {} occupied, {} floor\n\n",
  m.count(empty), m.count(occupied), m.count(floor));
//print(m);

auto m1 = m;
evolve_until_stable(m1, evolve_1);
//print(m1);
fmt::print("stable #1 has {} empty, {} occupied, {} floor\n\n",
  m1.count(empty), m1.count(occupied), m1.count(floor));

auto m2 = m;
evolve_until_stable(m2, evolve_2);
//print(m2);
fmt::print("stable #2 has {} empty, {} occupied, {} floor\n\n",
  m2.count(empty), m2.count(occupied), m2.count(floor));

}