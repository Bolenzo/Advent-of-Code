// Advent of code 2020, day 13
// Godbolt link: https://godbolt.org/z/WWzexq

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>
#include <fmt/core.h>

#include <numeric>

bool is_digit(char const c) {
  return '0' <= c && c <= '9';
}

using Int = std::int64_t;

Int to_int(std::string_view const s) {
  Int res = 0;
  for (auto const c : s) {
    res = res*10 + (c - '0');
  }
  return res;
}

std::pair<Int, std::vector<std::pair<Int, Int>>> parse(std::string_view const text)
{
  std::vector<std::pair<Int, Int>> buses;
  auto it = std::find(begin(text), end(text), '\n');
  auto const t = to_int({begin(text), it});
  int xs = 0;
  while (it != end(text)) {
    auto const f = std::find_if(it, end(text), is_digit);
    if (f == end(text)) break;
    xs += std::count(it, f, 'x');
    auto const l = std::find_if_not(f, end(text), is_digit);
    buses.emplace_back(to_int({f, l}),xs++);
    it = l;
  }
  return {t, buses};
}

std::pair<Int, Int> bus_and_earliest_time(Int const t0, std::vector<std::pair<Int, Int>> const& buses) {
  Int bus;
  Int time = INT64_MAX;
  for (auto const [b, _] : buses) {
    auto t = 0;
    for (; t < t0; t += b);
    if (t < time) {
      bus = b;
      time = t;
    }
  }
  return {bus, time};
}

std::tuple<Int, Int, Int> ext_euclid(Int x, Int y) {
  Int x0 = 1, x1 = 0, y0 = 0, y1 = 1;
  while (y > 0) {
    auto const q = x / y;
    x  = std::exchange(y, x % y);
    x0 = std::exchange(x1, x0 - q * x1);
    y0 = std::exchange(y1, y0 - q * y1);
  }
  return {x, x0, y0}; // gcd and coefficients
}

Int inv_mod(Int const a, Int const m) {
  auto const [g, x, _] = ext_euclid(a, m);
  assert( g == 1 );
  return x % m;
}

Int align_buses(std::vector<std::pair<Int, Int>> const& buses) {
  auto const prod = std::accumulate(begin(buses), end(buses), Int(1), [](Int res, auto const& bus) {
    return res * bus.first;
  });
  Int res = 0;
  for (auto const [b, d] : buses) {
    auto const a = (b - d) % b;
    auto const c = prod / b;
    res += a * c * inv_mod(c, b);
  }
  while (res < 0) res += prod;
  return res % prod;
}

int main() {

auto const test = R"(939
7,13,x,x,59,x,31,19
)";

auto const test2 = R"(0
17,x,13,19
)";

auto const input = R"(1015292
19,x,x,x,x,x,x,x,x,41,x,x,x,x,x,x,x,x,x,743,x,x,x,x,x,x,x,x,x,x,x,x,13,17,x,x,x,x,x,x,x,x,x,x,x,x,x,x,29,x,643,x,x,x,x,x,37,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,23
)";

auto const [t, buses] = parse(input);

fmt::print("t: {}\nbuses: ", t);
for (auto [b, _] : buses) {
  fmt::print("{} ", b);
}

fmt::print("\ndelays: ");
for (auto [_, d] : buses) {
  fmt::print("{} ", d);
}

auto const [b, e] = bus_and_earliest_time(t, buses);
fmt::print("\nbus {} arrives at time {}\n", b, e);

auto const res1 = (e - t) * b;
fmt::print("solution to part 1 is {}\n", res1);

auto const res2 = align_buses(buses);
fmt::print("solution to part 2 is {}\n", res2);

}