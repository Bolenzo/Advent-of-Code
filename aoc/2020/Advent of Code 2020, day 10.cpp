// Advent of Code 2020: day 10

// Godbolt link: https://godbolt.org/z/3dGfzh

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>
#include <fmt/core.h>

std::array<int, 3> count_sorted_diffs(std::vector<int> const& v) {
  auto cpy = v;
  cpy.push_back(0); // charging outlet
  std::sort(begin(cpy), end(cpy));
  std::array<int, 3> res{};
  for (auto i = 1; i < cpy.size(); ++i) { // adjacent_diff with special inserter
    auto const d = cpy[i] - cpy[i-1];
    //fmt::print("{} to {} is {}. ", cpy[i-1], cpy[i], d);
    ++res[d - 1];
    //fmt::print("res is [{}, {}, {}]\n", res[0], res[1], res[2]);
  }
  ++res.back(); // device
  return res;
}

class Node {
  int value_;
  std::vector<Node*> next_;
public:
  explicit Node(int const value) noexcept : value_{value} {}
  
  int value() const noexcept {
    return value_;
  }

  void add_next(Node* const node) {
    next_.push_back(node);
  }

  auto const& range_next() const noexcept {
    return next_;
  }
};

auto get_nodes(std::vector<int> const& v) {
  auto cpy = v;
  cpy.push_back(0);
  std::sort(begin(cpy), end(cpy));
  auto const last_value = cpy.back()+3;
  cpy.push_back(last_value); // additions to vector are wasteful, but dry code
  std::unordered_map<int, std::unique_ptr<Node>> res;
  std::for_each(rbegin(cpy), rend(cpy), [&](int const i){
    auto p = std::make_unique<Node>(i);
    for (auto const& [value, node] : res) {
      if (value - p->value() <= 3) {
        p->add_next(node.get());
      }
    }
    res.emplace(i, std::move(p));
  });
  return std::make_pair(std::move(res), last_value);
}

std::size_t count_paths_impl(Node const& root, std::unordered_map<int, std::size_t>& cache) {
  if (cache.contains(root.value())) {
    return cache.at(root.value());
  }
  std::size_t res = 0;
  for (auto const& node : root.range_next()) {
    res += count_paths_impl(*node, cache);
  }
  cache.emplace(root.value(), res);
  return res;
}

std::size_t count_paths(std::vector<int> const& v) {
  auto const [nodes, target] = get_nodes(v);
  std::unordered_map<int, std::size_t> cache_;
  cache_.emplace(target, 1);
  return count_paths_impl(*nodes.at(0), cache_);
}


#if defined(SLOW)

bool is_valid(std::vector<int> const& v /* sorted */) {
  if (v.size() < 2) return false;
  for (auto i = 1; i < v.size(); ++i) { // all_of | adjacent_diff
    if (v[i] - v[i-1] > 3) return false;
  }
  return true;
}

std::size_t count_valid_impl(std::vector<int>& v /* sorted */, std::size_t const first) {
  if (!is_valid(v)) return 0;
  std::size_t res = 1;
  //fmt::print("A valid combination is [");
  //for (auto const i : v) {
  //  fmt::print("{}, ", i);
  //}
  //fmt::print("]\n");
  for (std::size_t i = first; i+1 < v.size(); ++i) {
    auto const x = v[i];
    v.erase(std::next(begin(v), i)); // wannabe RAII
    res += count_valid_impl(v, i);
    v.insert(std::next(begin(v), i), x); // RAII destructor
  }
  return res;  
}

std::size_t count_valid(std::vector<int> const& v) {
  auto cpy = v;
  cpy.push_back(0); // charging outlet
  std::sort(begin(cpy), end(cpy));
  cpy.push_back(cpy.back()+3);
  return count_valid_impl(cpy, 1);
}

#endif // SLOW

//////////////////////////////////////////////////////////////////////

int main() {

auto const test = std::vector<int>
{ 16
, 10
, 15
, 5
, 1
, 11
, 7
, 19
, 6
, 12
, 4
};

auto const test2 = std::vector<int>
{ 28
, 33
, 18
, 42
, 31
, 14
, 46
, 20
, 48
, 47
, 24
, 23
, 49
, 45
, 19
, 38
, 39
, 11
, 1
, 32
, 25
, 35
, 8
, 17
, 7
, 9
, 4
, 2
, 34
, 10
, 3
};

auto const input = std::vector<int>
{ 99
, 104
, 120
, 108
, 67
, 136
, 80
, 44
, 129
, 113
, 158
, 157
, 89
, 60
, 138
, 63
, 35
, 57
, 61
, 153
, 116
, 54
, 7
, 22
, 133
, 130
, 5
, 72
, 2
, 28
, 131
, 123
, 55
, 145
, 151
, 42
, 98
, 34
, 140
, 146
, 100
, 79
, 117
, 154
, 9
, 83
, 132
, 45
, 43
, 107
, 91
, 163
, 86
, 115
, 39
, 76
, 36
, 82
, 162
, 6
, 27
, 101
, 150
, 30
, 110
, 139
, 109
, 1
, 64
, 56
, 161
, 92
, 62
, 69
, 144
, 21
, 147
, 12
, 114
, 18
, 137
, 75
, 164
, 33
, 152
, 23
, 68
, 51
, 8
, 95
, 90
, 48
, 29
, 26
, 165
, 81
, 13
, 126
, 14
, 143
, 15
};

auto const diffs = count_sorted_diffs(input);
fmt::print("There are {} 1-diffs and {} 3-diffs, product is {}\n",
  diffs.front(), diffs.back(), diffs.front() * diffs.back());

auto const combinations = count_paths(input);
fmt::print("There are {} valid combinations\n", combinations);

}