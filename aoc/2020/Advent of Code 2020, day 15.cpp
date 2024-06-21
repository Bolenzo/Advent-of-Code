// Advent of Code 2020: day 15

// Godbolt link: https://godbolt.org/z/v3b83z

#include <unordered_map>
#include <utility>
#include <vector>
#include <fmt/core.h>

auto setup(std::vector<int> const& starting) {
  std::unordered_map<int, std::pair<std::size_t /*2nd-to-last*/, std::size_t /*last*/>> res;
  for (std::size_t i=0; i<starting.size(); ++i) { // enumerate
    res.emplace(starting[i], std::pair{i, i});
  }
  return res;
}

int spoken_number(std::size_t const n, std::vector<int> const& starting) {
  auto memo = setup(starting);
  auto last = starting.back();
  for (auto i = starting.size(); i < n; ++i) {
    auto const [p, l] = memo[last];
    last = l - p; // conveniently 0 if 'l == p', that is spoken only once
    //fmt::print("{}. {}\n", i, last);
    if (memo.contains(last)) {
        memo[last] = {memo[last].second, i};
    }
    else {
        memo[last] = {i, i};
    }
  }
  return last;
}

//////////////////////////////////////////////////////////////////////

int main() {

auto const test = {0, 3, 6};

auto const input = {1, 20, 11, 6, 12, 0};

auto const n = 2020;
auto const res = spoken_number(n, input);
fmt::print("(1) the {}-th spoken number is {}\n", n, res);

}