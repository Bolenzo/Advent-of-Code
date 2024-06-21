// Advent of Code 2020: day 19

// Godbolt link: https://godbolt.org/z/3oWd34

#include <algorithm>
#include <cassert>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fmt/core.h>

bool is_digit(char const c) {
  return '0' <= c && c <= '9';
}

auto fetch_int(std::string_view& s) {
  int res = 0;
  std::size_t pos = std::find_if(begin(s), end(s), is_digit) - begin(s);
  while (pos != s.size() && is_digit(s[pos])) {
    res = res*10 + (s[pos++] - '0');
  }
  s.remove_prefix(pos);
  return res;
}

std::string_view fetch_line(std::string_view& text) {
  auto const pos = text.find('\n');
  auto res = text.substr(0, pos);
  text.remove_prefix(pos == std::string_view::npos ? text.size() : pos+1);
  return res;
}

//////////////////////////////////////////////////////////////////////////

template <class T> using iMap = std::unordered_map<int, T>;

auto parse_rules(std::string_view& s) {
  iMap<std::string_view> res;
  for (auto line = fetch_line(s); !line.empty(); line = fetch_line(s)) {
    auto const i = fetch_int(line);
    line.remove_prefix(2); // colon and space
    res.emplace(i, line);
  }
  return res;
}

class Rule {
  static auto constexpr missing = '#';
  std::vector<int> v_;
  std::vector<int> alt_;
  char c_;
  int i_;
public:
  explicit Rule(int i, char const c = missing) : i_{i}, c_{c} {}
  
  void add_main(int const i) {
    v_.push_back(i);
  }
  void add_alt(int const i) {
    alt_.push_back(i);
  }

  bool matches_exactly(std::string_view s, iMap<Rule> const& rules) const {
    return matches(s, rules) && s.empty();
  }

  int matches_n(std::string_view& s, iMap<Rule> const& rules) const {
    int times = 0;
    for (;;) {
      auto cpy = s;
      auto const m = matches(cpy, rules);
      if (!m) break;
      s = cpy;
      ++times;
    }
//fmt::print("{} matched {} times\n", i_, times);
    return times;
  }

  bool matches(std::string_view& s, iMap<Rule> const& rules) const {
    if (c_ != missing) {
      if (s.empty()) return false;
      auto const f = s.front();
      s.remove_prefix(1);
      return c_ == f;
    }
    auto const matcher = [&](auto const& v, auto& str) {
      return std::all_of(begin(v), end(v), [&](int const i) {
        auto const& rule = rules.at(i);
        return rule.matches(str, rules);
      });
    };
    if (!alt_.empty()) {
      auto cpy = s;
      auto const matches_alt = matcher(alt_, cpy);
      if (matches_alt) {
        s = cpy;
        return true;
      }
    }
    return matcher(v_, s);
  }

};

auto solve_rules(std::unordered_map<int, std::string_view> const& rules) {
  iMap<Rule> res;
  for (auto const [i, s] : rules) {
    if (s.front() == '"') {
      res.emplace(i, Rule(i, s[1]));  
    }
    else {
      auto const pipe = s.find('|');
      auto f = s.substr(0, pipe-1); // ignore space
      Rule rule(i);
      while (!f.empty()) {
        rule.add_main(fetch_int(f));
      }
      auto const has_pipe = pipe != std::string_view::npos;
      if (has_pipe) {
        auto alt = s.substr(pipe);
        while (!alt.empty()) {
          rule.add_alt(fetch_int(alt));
        }
      }
      res.emplace(i, std::move(rule));
    }
  }
  return res;
}

auto parse_solve(std::string_view text)
{
  auto const rules = solve_rules(parse_rules(text));
  auto const zero = rules.at(0);
  auto res = 0;
  while (!text.empty()) { // all_of
    auto const line = fetch_line(text);
    res += zero.matches_exactly(line, rules);
  }
  return res;
}

auto hack_solve(std::string_view text)
{
  auto const rules = solve_rules(parse_rules(text));
  auto const _42 = rules.at(42);
  auto const _31 = rules.at(31);
  auto res = 0;
  while (!text.empty()) { // all_of
    auto line = fetch_line(text);
    auto const f = _42.matches_n(line, rules);
    auto const t = _31.matches_n(line, rules);
    auto matches = f > 1 && t > 0 && t < f && line.empty();
    if (matches) ++res;
  }
  return res;
}

//////////////////////////////////////////////////////////////////////

int main() {

auto const test = R"(0: 4 1 5
1: 2 3 | 3 2
2: 4 4 | 5 5
3: 4 5 | 5 4
4: "a"
5: "b"

ababbb
bababa
abbbab
aaabbb
aaaabbb
)";

auto const test2 = R"(42: 9 14 | 10 1
9: 14 27 | 1 26
10: 23 14 | 28 1
1: "a"
11: 42 31
5: 1 14 | 15 1
19: 14 1 | 14 14
12: 24 14 | 19 1
16: 15 1 | 14 14
31: 14 17 | 1 13
6: 14 14 | 1 14
2: 1 24 | 14 4
0: 8 11
13: 14 3 | 1 12
15: 1 | 14
17: 14 2 | 1 7
23: 25 1 | 22 14
28: 16 1
4: 1 1
20: 14 14 | 1 15
3: 5 14 | 16 1
27: 1 6 | 14 18
14: "b"
21: 14 1 | 1 14
25: 1 1 | 1 14
22: 14 14
8: 42
26: 14 22 | 1 20
18: 15 15
7: 14 5 | 1 21
24: 14 1

abbbbbabbbaaaababbaabbbbabababbbabbbbbbabaaaa
bbabbbbaabaabba
babbbbaabbbbbabbbbbbaabaaabaaa
aaabbbbbbaaaabaababaabababbabaaabbababababaaa
bbbbbbbaaaabbbbaaabbabaaa
bbbababbbbaaaaaaaabbababaaababaabab
ababaaaaaabaaab
ababaaaaabbbaba
baabbaaaabbaaaababbaababb
abbbbabbbbaaaababbbbbbaaaababb
aaaaabbaabaaaaababaa
aaaabbaaaabbaaa
aaaabbaabbaaaaaaabbbabbbaaabbaabaaa
babaaabbbaaabaababbaabababaaab
aabbbbbaabbbaaaaaabbbbbababaaaaabbaaabba
)";

auto const input = R"(27: 116 44 | 127 69
19: 60 116 | 55 127
91: 127 13 | 116 127
11: 42 31
30: 80 116 | 100 127
53: 78 116 | 100 127
47: 116 129 | 127 76
21: 57 127 | 100 116
26: 86 116 | 28 127
86: 127 122 | 116 29
74: 127 39 | 116 105
41: 118 116 | 132 127
13: 116 | 127
115: 82 116 | 72 127
100: 116 116 | 116 127
5: 78 127 | 91 116
31: 101 127 | 121 116
134: 116 117
73: 123 116 | 62 127
69: 116 124 | 127 77
28: 71 116 | 66 127
14: 131 116 | 128 127
111: 127 59 | 116 117
6: 127 53 | 116 79
104: 26 116 | 65 127
61: 78 127 | 56 116
67: 127 30 | 116 130
32: 81 127 | 27 116
2: 116 117 | 127 72
128: 57 116 | 91 127
81: 127 96 | 116 51
38: 127 59 | 116 54
36: 22 127 | 107 116
35: 116 61 | 127 24
7: 116 100
18: 16 116 | 82 127
116: "a"
54: 116 116 | 127 116
84: 20 116 | 93 127
92: 38 127 | 111 116
64: 127 57 | 116 82
105: 116 43 | 127 110
44: 127 106 | 116 88
93: 33 116 | 56 127
82: 116 13 | 127 116
75: 2 127 | 9 116
65: 127 97 | 116 36
4: 5 127 | 98 116
1: 114 127 | 58 116
122: 54 127 | 59 116
70: 116 67 | 127 75
117: 127 127
68: 48 127 | 40 116
97: 127 95 | 116 113
132: 127 33 | 116 54
51: 23 127 | 30 116
119: 127 37 | 116 103
63: 57 127 | 59 116
34: 57 13
48: 116 73 | 127 47
23: 82 127 | 59 116
66: 127 82 | 116 78
95: 127 100 | 116 82
123: 117 116 | 82 127
24: 116 33
120: 49 127 | 99 116
103: 127 90 | 116 83
102: 72 13
58: 46 116 | 45 127
106: 56 127 | 117 116
85: 126 127 | 133 116
3: 74 116 | 85 127
20: 33 127 | 56 116
127: "b"
45: 116 102 | 127 18
126: 127 41 | 116 112
94: 116 87 | 127 21
114: 35 116 | 19 127
0: 8 11
99: 127 34 | 116 76
80: 116 127 | 127 116
50: 15 116 | 7 127
55: 127 91 | 116 59
90: 92 127 | 94 116
25: 57 116 | 54 127
124: 116 57 | 127 91
77: 78 116 | 78 127
56: 127 127 | 116 116
79: 116 59 | 127 57
12: 127 64 | 116 115
130: 17 127 | 33 116
8: 42
60: 116 72 | 127 59
113: 117 127 | 117 116
131: 16 127 | 56 116
46: 134 116 | 62 127
83: 6 127 | 50 116
33: 116 116
88: 116 57 | 127 54
118: 116 100 | 127 54
125: 100 116
121: 32 116 | 104 127
29: 127 17 | 116 57
101: 127 1 | 116 68
22: 116 117 | 127 100
43: 127 125 | 116 22
10: 29 127 | 63 116
112: 98 116 | 107 127
87: 80 127 | 57 116
129: 54 116 | 17 127
98: 116 80 | 127 117
57: 116 116 | 127 13
107: 127 72 | 116 59
40: 116 14 | 127 4
39: 12 116 | 84 127
133: 116 108 | 127 10
71: 127 56 | 116 54
76: 116 78 | 127 33
15: 127 59 | 116 78
42: 119 116 | 3 127
9: 127 72 | 116 100
72: 127 127 | 127 116
17: 13 13
16: 116 116 | 13 127
49: 53 127 | 25 116
62: 127 80 | 116 100
52: 72 116 | 33 127
37: 70 127 | 120 116
110: 89 127 | 87 116
96: 116 52 | 127 55
78: 116 127
108: 127 109 | 116 71
109: 116 59 | 127 16
59: 116 127 | 127 127
89: 57 127 | 33 116

babababababababaabbbbbabaaabbabbabbabbaa
abbbbbaababbbabbbbbaaaaaaaababbbaabbabab
bbaaaaababababbaaaaabbababbabaabaaaaaaabaaaaaaaa
aabaabbaaaaaabaaaaaaabaabaabbbbb
bbaababbaaabaabaababbaaaabaababbaabbbaababbaaabababaabab
bbabbabbabaabbaabbbbaabbbbaabbaa
baaaabbbabaaababbababbbbababbbbbaaaabaabaabbbbaa
aaaabbabbaaaabbbaabaaabbaaabbbbababaabab
bbbaabaabbaabaaaaaababbbbaaaabbaaabaaabbbbbbbbbbbababbab
aabaabbaaabaaaabbbbaababbaabababaabbbbab
bbaabbbbbaaabbbbbabbbbaaaabaabbaabbaabbabbababbb
aaabaabaabaaababbabbabbb
abbababbabbababbaaaaaaab
ababbaaaabbabbabaaaabbaaaabaaabbbaabbbbaabbbabbbbbbaabbabbabbaaaababaabaabbabbaa
abbababaabbbabbbaaababbbabaaababbaababaabaaaaaaaaabababbabababab
baabbbbabaababbabaababbbababbbaa
aaabaabbbbbbaaaabaaaaabaaaaaabaaaababbaaaaaabaab
babababbabaaabbabbbbbbbabaaaabbaabababaaaabbbaab
aabaabbaabbabaababaaaaaabbbababaaabbbbab
aabaaabbbaabaabbabaaabbabbababaabbbbabbb
abababbaaaabbababaaaaabaaaaaabbaabbbaabb
bbaaabbbbaababaabbbaabbbabbbababbbbabbaa
babaaaaabbbbbbabbabaaaba
baaababbaabbbabbaaaababbababbabbbbbbbbbbaabbbaaaaaababbbbbaabbbb
bbbbaaabbaaaabaabbbbbbbabaaababaabbbbaaa
bbabbabbbbabaabbaaaaabab
baaababbbbbbaababaabaabbbbbbaaaaaaaaababaabbbaabaabaaaba
baababaaaababaaaaaaabbbaabbbbbaabaaababaaabbabaaaaabbaaa
abbbbbbbaaabaababaaabbaa
bbbabaaaabbabbbaaaaabaaabbbaabaabbbaaabbbabaaaba
abaabbaabaaabbabaaaaaaaa
aaabaabbaabaaaabaaaabbaabbabbaab
bbaabaaaabbaabaaababaabb
bbabaaababbaaabbbbabbabbabaaaaabbaaaabbbabbababbabbbbaaa
abbaaabbababbabbbaaaabbababaabab
aabababababbbaabaaabababababaaaaaaabbaaabbabaaba
abaaaaabbbbbbaabbaaaabababaaaabb
bbbbabbabababbbbaaaaaaba
aaaaaabbbaabbbababbabbbabaaabbbbabbabbaa
bbbababbbababbbbbbbbbabaabbaabaababbaaaabbababbb
abbabbabaaaabaaabaaabbbaabbaabababbbaabaaaaaaaabbaaababa
abbbabbbabaababbaababaabbabababbaaaaabaababaabaabaabbabbbabbbbbb
bbbaabbbbbbababbaaabbabaabbbbababbaababbabbabaaababaabbb
bbaaaaababbaaaaaabababaaaababbbabaabababbabbbaaa
aabbbabbbbabaabbabbaabba
ababbababbbbaaaabbabbaab
abbaaaababbbbbabbaaabaaabbabbabbbabababbabaabbba
bababaabbbaabaabbaaabbaa
bbbaaaabbababaabbbababab
babaabbbbaaababaababaaabbbaabbab
bbbabaaabaaaababbaabaaabbababbbbbaabbaab
abbbbbbbaababaabaabababb
abbbabaabababababbaaaaabaaabbabb
baaabbabbbbaababbbaabbba
bbbbaabababaabbabaabbbaaaababbab
abbaaaaaaabaaabbbaaabbababbaaaaa
aaaabbabaababbababbbbaababbbbaaa
abbbbbbbbbbaaabbbabbabbb
abaaababbababbbbbabaaaabbabbaabbaaaababb
bbaabbbabbbabbbbabaabbbb
babbaababbbbbbabaababaaabbaabaababaaaaba
bbbaaaabbbbbaaabbaabbaba
ababbaaabbbbbaabbbaabbbbbaabbbbb
baabbbabbaababbbbbbabbbb
abbaabaaaaaabbabbaaabbabababbaaaababbbbbaaaababb
bbbbbaabbbbaaabbbbabaaba
bababbaaaababaaabbbabbba
bbbbaabbaababaabababbbaa
bbbaaaaaabaabbabaabaabaa
bbbaaabbbbabbabbaababbbb
aabababababbaabaabbaaaaaabbabbabaabbabab
babbbabbaaababbbbbabaabbabaaaabb
aababaaabbbbbaaabaabbabb
bbbbbaaabaaabbababbaaaaaabaabaaa
bbaaaaaababaaabbaabbaaaa
baababbbaaaabbaaabaabbba
aababaaaabababaaaaaabbabababbbbb
bbaababaabbababbbbbbbaaaabbbaabbabbbaaaa
bababbbbaababaaabaabbbbb
bbaaaaaababaaaababababaaabaabbba
bbbaaaaaabbaabaaabbaabaaabbabaaababbbabaaabbaaababaaabbb
aabbbbbbbbbaaaaaaabbabbb
bbabaababbbaabaabaaaabbaabbabbbbbabaabbabbbababaaababbababbabbbabbabbbbabbabbbababbabbaababbaaaa
abaaababbabbbaabbabaabbaabaaabbaabbbaaabbaabbaabbbaaaabb
aabaabbbbbbaababbabbaababbbbbbaa
baabaaabbbbbbaaaabbbbbabaabbbaba
bbbbbbbabbaababbabababab
aaabaaabbbbbbababaaababbbbbbbbabaaababababbbbaaaabaababaaabaabababbaabba
abababaabbaabbbbaabbaaaa
bababbbbbbbbbaabbaaabbaa
abbbbbbaaabbbaaaaabbabab
baaaababbbbaaabbaaaaaabbababaabb
abbbbbabbbabaabbbabbaabb
aababbbabbbbaabbaababbaa
bbbbaabbbbaaaabaaabbabaa
abbaaaabbbaaabaabbbababbbbbbbbbabbbaababbaababbabababbabbbbabbbabbaaabba
aaabbabaababaaaaabababbaabbababbbbbbbababbbbbabb
abbbbbababbbaaabaabbaaba
abbbbbbaaabbaaabaabbbbbaaabbaababaaaaaaa
ababaaaaaaababbbaaabaaaa
babbbbaaabaaababbbbbbbaa
abaaaaabbabaaaabbbbbbbaa
bbbaaaabbbbbabbababbbbbb
baabbaaaabaabbaabbbbbbaaabaaabababbbbabaabbbabbaaaababbaaaababbaaabbaaababbbbaba
bababbbabaaababbbabbbbaaaaaabbbaabbabaab
bbbbaabbbababbbbabbbabbbaabbbbba
aaabbbbaabbbaaabaababbaa
aaabbabababbbabbbbbbabbb
ababaaabbbababaabbbbbbbbbabaaabbabbbbbbbababaabb
abbbaaababbbbabaabbaaaaabbbabbaaaaabbabb
baabbbababbabaaabbbbbaaaaaabaabaabaaabbababbaaab
aabaabbabababbbbbbbababbbbabbbabbbaaabba
baaababbbbbaabaabaabbaba
abbbbabaabbbaaabbaabbbabbbbbaaabbbababab
babaaabbabaabbababaaabaa
abaaabbabaaaaaababaaabababaaaaabaabababb
abbbabbbaaababbbababaaaabbabababbbaabbab
aaaaabaaabbbbbbbbaaabbbaabbbbbbabbaabbabaaabbaab
bbbbbbbabbbaaabbaaabaabaababbabaabaaabaaabaaabbb
abbbbbbaabbabbbabbbbabbabbbbabbaaaaababb
bbbbbaaababbbaabaabbbbbbbbbbbbbbbababaaa
abbbbbbbbbbbbaabaabbaaba
abbaaaababbbbbaaabbabbbaaabaabbabbbbbbbababaabaa
abaaabababbbbabaabbaabbaaabababaaabbbabbabbababaabbbaaaaabaabbaaaaaaabba
bbaaabbbaabaaabbbbbaaaaababbabab
baaabaaabaababbabbabaaabaaabbabb
bbbbabbaabababaaabbbaaba
aabbbabbbaabbbbaaaabaaab
abaaabbabbaabaabaababababbabaaabbbbbbabbabbbbbbbbbbbbbbbababbbaabbabbaababbaaaba
bbbaabbbabbbaaabbbbbbabaabaaaaba
abbbbabababbbaabaaaababa
abbabbbaabbbbbbbaaabaabbbbbbaabaaabbabaabaabbbbb
babbbabbbbbaaabbabbaaaaaabbaaaabbbbbbbbb
bbbbaaaaabbabababbaaabbaabaaabbaababababbbbabbbabbaaaabaabbaabaaabbababb
abbabbbabaabbbabaaabbaab
bbaababbbaababaaaabaabab
bbbbababaaabababbbabbbbbbabababaabbaabaababbaaab
baaaababaaaaabbaaaabbbbbbbaaaaabbabbababaaaabbbb
baaaaaabbbaaaaabaaabbaabbabbbaaababbabba
abbbbabababbaabaababbbab
aaabaabaaaabaaabbbbababbaabbaaaabbbabbaa
aaaaaabbbbaaabaaaaaaabaabbbabababaabbaaa
abbbbbbbabbbbabaaabbabaa
bbaaaaabbaababbabbbaabba
abbbbababaaababbbbbbabab
abbababbababbbabaabbaabbaaaaaaaaaaabaaaa
bababbbbbbaaaaaaabbbabba
baaabbabbbbbabbabaabbaababbbaaaa
baabaaababbbaaabbaabaaabaababbbbababaaba
aaaabbaabbbaabbbbaabaabbbaabbaaa
baaabaaaabbbbbabaaabaababaabababbaaababa
abbabbabbaaaaaabbaaaabaababaaaababbbaabb
bbaaaaaaaababbbaaabaaaabbaaaabaababaabaa
baaaaaabaaaabbbaaabbaaba
aababababbbbbaaababababbbbaaaaaabbaaabba
bbbaaabbbabababbbababbaaaaabbaaa
baababaabababaabbbaaaabb
bbaababbaababbbaaababbbb
aaaaaabbaabbbbbbbbaaaaabbaaaababbbbababbbbaaaababababaaaabbbbaab
baaabbabababbabababaabbaabbaabaaabaababbbbabaaabbabbbaaaaaababbabbabbbabaabbbaab
abbbaaabbababbbbabbbabba
aababaababbbbbbbabbababbbbbbbaba
abbbabbbbabaaabbbbaabbbbababbaaaaabababaabbaababbbbbbbaababbaabbbbbbbbaaabababbb
bbbaaaaaaababbabbbbbabaabaabbbabaabbbbbbbababbbb
baababbabaaabaaababaaabbbaabaabbbbbaaabbbabbabaaababaaba
aaabaaababbaaaaaaaabababbabaaaba
abaaababaabaaaabaaaabbbb
bbbaabaababbaababaaaaabaaaabaababbababab
bbbbbabaabbaaaaabaaaaabaabaabaab
baaaabaaabbabaabbbbbbbbb
bbaaaababbbbbbabbaababbaabababbabbbbaaaaabbbbbaababaabaabbbaaabaaaaababbababbbaaaaababba
bbbababbababbabbbabbbaabaabaaabbbababaabaabbaaaa
bbbbaabaaaaabbabbababbab
baabaaabbabaaaaababaaaba
abbbabaabaabbbbaabaababa
abbbbbaabbbbbbabbbbbaabbbaaababbabbabaaaaabababb
bbbbaaabaaababbbaaaaabab
bbbaaaaabbbbaabababbbbba
abbabbbaabbbabbbbaaaabbababbaaababbbaaaa
aaabbaaababbbabaaaababaa
ababaabbabaaaaaaaaaaababbbaaabbbbbbababababbabbaaaaabaabaaabbbab
baababaabbbaabaabababbaabaabbbbabbbbbabb
abbbabbbaabababaabbbaabb
baaaabbaaababaabaabaaabbabbabaabbaaabbbababbbaaa
baababaababbbbaabbaaaaaabbbbaaababaabbaaabaaaaabaaaaabbb
abbabbbbbaaabbbbabbbaaaa
abbbbabababaaaaaabbaaaaababbbbbabbbbabab
bbbaaabbabaaaaaaaaaaaabbababaaaabbaaaaabaabbaaaa
bbbbaabbbababbaaabababbabaaaabaabaabaaaa
baaababbabbaaaababbabbaa
abaaaaabbbabaaabaabaaaabaaabababbbaaababaaababaabbaaabab
bbbaaaabbababbbbababbbba
baaababbbababaababbbbabb
aaabbbbbabbabaababbaaaabbaabbaba
baabaabbaaababababbbbbaababababbaabaabbaaabaaababbabbbaabaabbabbaabbabba
baaaabbbbabaaaaabbabaaabaabaaaabbbaaaababbaaaabaabaabaaaabaabaaaabbbabbaaabbaaab
bababbbbbbbbaabbabbababbbabaaaaabbbabaab
baaabbababaabbababaabaab
abbaaabbbbbbaabaabbbbababababbbbababaaba
bbbbaabaabbaabbaabaabbbb
baababbbbbbbbababaaaabbbaaaaabbb
aaaabbabbabbaababbabaaababbaabab
aababbbabaabaabbbbbaabaabbabbbaa
abaabbaabbaabbbbaabbaabb
bbbaaaabaaabbbbabaaabaaabbababbbaabbbaab
abaabbaaaaabaabbbbbbbababbbbbababbbaabbbbbaabbba
baababbbbbbababbaaaaabaababbbbba
abbabaaaaababaabababbaaabbaaabbabaaabbaa
abaaabbaabababaaaabaaabbbababbbbbaabbbbbaabbabbaababbbab
bbaabbbbbbabbbbaabbaabaababbbaababbababbbbabbabaabbaabbbaabbaaaa
baabababababaaaaaabbabba
babababbbbaaabaaabaabbbb
abbabaaaaaaaabaaabaaabbababbbbaabababaabaaaaaaaaaabbbbbaaaaabababbababba
bbabbabbabaaabbaaabbabab
aabbbbbbbbaababbbaaabaaaaabaaaba
bbaababbabbaaaabababaaba
bbabaabbbaabbbbbabaaabaabbbbbbaa
bbbabaaabbbaaaababababbaabaaababbbbaaabbbbbabbaabbabaababaaababa
abababaaaababbbababbabab
bbbbbabaabbababbbbabbabbaabbaaaa
babbaabababaaaabbbaabbaa
bbbaabbbbababababbabbaab
bbbaabaaabaaaaaaababbbbbbbaaaabababbabababbaaabbababaabbabbaabaabbbabbaa
bbbbabbabababbaaaaaabaab
babbbaabbababbabaaababbbaaababbaaabbaabb
abaabbababaababbabbbaaabbbbbabbb
bbbabaaabaaaabbababbbabbabbbabbbabbbbbbabbabbaaa
bbabaabbaaabaabaabbabbabaabaaabbaabbbbaa
bbababaabaaaaaabbabbbbaabbaabaabaabbbababbaabbba
abbaaabbbaaabbbbbabbbabbaaabbbaa
bbbabaaaabaaabbaabbabbaa
abbbbbbaabbabaaaababbbab
abbbabbbaaaabaaaabbbbbaababaaaababbabbbaaabbbbba
abbabaabbaaabaaaaabaabbbaaabaabaabbbaaaa
bbbaaaaaabbaaaaabaabbbaaabaaaababaaaaabb
baabbbbabababbbabababaabbaaabbabaabbbbaa
aabaabbbabbabbabbabbabba
babbbabbbbbaabbbaababbbabbabaaababbaabab
bbbaabaaaaaabaaaabbbabaababaaaaababaabaaaabbaaabbabbabbb
aabaabbbbaabababaaaaabbb
baababaabaaabbabbaaabbaa
abababbabaabbbabbbaaaabaabbaabbabbbbabbb
abbbabbbabbababbbababaaa
bbabbbbaabababbaabbabaabababbaaaaabbabaabbbbabab
aaabbabababbbabbaabbbbab
bbaabababbaaabbbaabbbbab
baaabbaababbbbbaaabaaabaabaabbbababbaaab
bababbbbbbbbaabbbbbbabaa
bbaaaabaaaaabaaaabbababaaabbbaab
aaabbaabaaaabbabbbaaabababbbbababbbaababbbaaaabbabaabbab
ababbaabaabbbbbbbabaaaabaaaaaaab
baaabbbaaaabbabaababbbab
bbbbaababaabaaababbbabbbbaaabbbbaaaaaabbbabaabaaaabbabbb
bababbaaaaaabaaabbbaababaaaaabbaabbaaaaaaabaaaaaabaabbbbbabbbbababbbaabb
aaabbababaaabbbbabbaaaaaaaaabbbaaaabbbaababaabbb
aabaaaabaaaabbbabababababbbbaabaaabaaaaa
aaaabbaaabbabbbaabbabaaababababbabbabaaaabbaabbb
aaaabbbbbbababbaabbbaabbbabbbbaaabbaaabbbababbaabbbababbabbbababbaababbbaababbaaaaababbb
babababbbbabaabbbbaaaabb
abaabbabaaabaababbababaabbabaaababbbbaaa
aaaaabaabaaaabaaaaabbaaa
baaabbbaabbaaabbaababaabbbaaaaaaabbbaabb
babaaaabbaabbbabbbbabaab
baaababbbbbbbaaaaaaaabaaababbbabaaaaabab
bbbaabaabaabbbabbbaabbba
aaaaabbbabaabbbaabaaabbaabbbbbabaaaabaabaabbabbbbbbabbbbbbabbbbaaaaabbaa
abaaaaabbabbaababbaababbaaaaabaaababbaaaaaaabbbbbaabbaaa
babaaaabbbaababbbaabbaaa
babbbbaaabaabbaababaaaabbbababaabbbbbbaababaabaa
baaaaaababababaabbbabbba
bbaaabaabbaaabaaaabaabbaabbaabbaaabbaaaa
aaabbbbbaaabaababbbbbaababbbaaababbabbbabbbbbbbbaaaababbabbbaaba
bbaabaabbbbbbaaabababbab
baaabbbbbbbbbabbbaaaaabb
aabbbabbbaaaabbbaabaaabbaaabaabaabaabbbbbaabaaba
abbaaaaabbbbbaaaaaaabbbababbabba
bababbbaababbabaababaaab
bababbaaabbbbbabbbabbaab
bbabaabbaaabaababbbbabbaaaaabbbaaaabbbbabbabbaab
bbabaabbbaaaabbaabbbbbaababbbbaaaabbbbba
babaaaabbababababbaaaabababbaaab
bbbbaabaaabababaaabbbbbababbbababaaaaaabbbaabbabaaabbbaa
baaabbbbabbababaabababbaabbaabababaabaaa
bbabaaabaababababaaaababbbbbbbababbaababaabbbaab
bbbbbabbaababaaabbbbbababbbababa
bbbabaaaaabbbbbbbaabbaab
bbbbabbaabbbbbababaaabbaaaabbbbbbbbbbabababbaabb
abbabbabbbbbabbabaaaaaaa
bbaabaabbabbbbaabbbabbaababbbababbababbbbbbbabbb
abababbbaaaababbbabbaabb
bbabaabbaaaaabaaabbabaabaabaaaba
bbabbbbaaaabababbbabbbbb
bbaaaaabaaabbabaabbaabaaaabbbabaaabaaaaa
baaaaaabaabaaabbbaabababaaabaaababaaabbbbabaabaabaabbaba
bbbababbbaababbabaaaabaabbbaabaaabababbbababbbbb
bbaababbbabbbabbaababbbababababbbbbaaaba
baaabababbabbabbabbbbabbbabaaabaaabbaabababbbbbbbaabbbaa
ababaaaabbbbbabaabaabbabababbabbaabbabba
bbabbabbbabababaaabbaaba
baaabbbaabababbbbaabbabbaabbaabbbababbbaabbaaabbbabaaabbaaabbbab
aabababaabaaabbaaaaababb
babaabbaaaabbbbbaaababaa
aaabaaababbbbabbabbabaababababab
abaaaaaabbabbabbababbaabbabaabaa
aaaaabaaabababaabbababba
babbaabbaaaabababbabbbbbaaaababa
bbaabbbbbababaababaabbbb
bbaaaaaabbbbaabbbbbaaabbaaabbaabaababbbb
bbabbbbaaababbaaabbabbaaabbbaabaaababbab
bababababaaaabaaabababbb
baaabbbaaabaaaaaaabbbbba
baabbbbabbbaabaaaaaabbababbaaaaabbaaabbb
babaabbabbbbbabaabaaaaaabbbabbaaabaabbba
babababbbabbbbaaabbaaaababaaaaabababbaaaaaabbbabababbbabababbbbbbbbbabbb
aabaabbbaaabbbbaabbabbabbabbababbaaaaabb
baaabaaabaaabbabaaaabbabbbaabbaaabaabaaa
aaabaababbbbbbabaabbbbbbaaaabaab
aabbabbbbbbbbbbbababbbabaaabbbaa
bbbaabbbababbababbbbaaabbababbbabbababba
bbabbbbabaaabaaababababbaabbaaaa
abbabbabbaaababbbbbaaaba
baaabbbabbbbabbabbbbabbb
abbbabaabbaabaaabbbaaaababbababbabbbaaaaaabbaabb
bbbbbaaabababbbabbbaaaba
baaaababbbaababbaababbaa
baabbbbabbaabaaaaabbaaba
abbbabaabbbaababbbabaaabaaaabbaabbabaaba
bbababaabbbabaaabababbbbabaaabbb
ababbaaaabbbbbabbbabaaba
bbbbaabbbbbaaaabbbaabbbaaaabbabb
bbbbbbababbabbbbabbaaaababbbbbbbbbabbbbb
abbaababbbaabbaaaaabbbababaababaabaaabaaaaaababb
baaabaaabbbbabbababababaaabaaabbabbaaabbbaabbabb
baabbbaaaaaaabbaabaaaaababbabbabbbaaabab
ababababbabbaabbbabbabbbbabaaaaaaaaabbbb
baababbabbbaabbbabaaabbaabbbabba
bbaabbbbbbaababbbabababbbbbaaabbabbaaaaabbbbbbbbbbbbabaaabbbaaaa
baaaaabaaabaaaabaaabbbbaababbaabbbbbbabaaababbaa
abaabbaaabaababbbbaabbaa
baaabaaabbaaaaaababaabab
bbbbaaabbabababbbbaaabba
aaaaaabbababbabbabbabababbabaabbaabbabbbbbbaaababbabbaba
bbbbbabababaaaabaabbaaaa
baaaabaaaaabbbbbbababbbbbbbaabbbbaaabaabababbbaa
bbaaaabaabbbbbaabbaaabaaaababaaabaaaabaababbaaab
bbaabbbbbbaaaabaaaabbbbbbbaaabaaababbaaaabbaaaabaabaaabaabbbabbaababbbab
bbbbaababaaabaaaaabbbbba
aaabbbbaabababaabbaababaaaabababaaaaaaaa
bbaabaabaaaabbbabababbaaabbbbbaabbabaabaaaabbbab
aaabaaabbbaabaaabaaabbababaabbabbbabbbaa
abbabaababbabbbabbbaaabbbbbababa
aaabbbbbaabaabbababbbbba
baaaabaabbaababbababbbba
bbbbbbabbabbbaaaababababaababbbaaababbbabaabbbbaabaabbaaabbbbaaa
abbabaabbaaabaaabaabbaaabaabbaaababbbbbbbbbbaabaabaabbab
abbbbbbbbbbababaaaabaaaaabaababaababbbbababbaaaa
ababbabababbbaababbabbbaabbabbbbbbabbaaa
baaaabaababaabbabbaaaaaabbbaaaaaabaaabababaaaabbbabbaaaa
abbbbbaababbbaababbaaabbbbaaabbbbbababab
aabaabbaabbbbbababaabaaaabbaaabaabbaaababbbbabaaabbababbabbbbabaabbbaaaaaaabaabb
babbaababababababbaabbba
abbabaabaababbbababaabaa
bbaabaaabaabababbaabababbaaababbbbbaaaabbabbbbbbababbbbaababbbbbbabbbaba
aaabaaabbbaaabbbbbaaabaabaaaababbbabbbabababababbbbabbaa
bbbbbbaaabbbabaababaabaaababbbabaaabbbbababaaaabbbbbbbbabaaababbbaaabaabbbbbabbaaaabaaba
aaabbbbabbbaabaaabbaabaabaaababaaabbbaab
abbaaabbbbbbaabbbaababaababbbbba
bbaaaaabbbbabaaabaaabaab
bbbaaaaababababbbaaabbbbbaababbabaabaaaaababbbbabbababba
ababbaaabbbbbbbaabaaaaaababaabaaabaaabaa
baababbbbaaababbaabbbbba
babbabbbabbabbaabababbbbaaaabbbbbbbbaaabbabbaabaaababbab
bbabbabbabbaaaabbaabbbaaaaabaaababbbbbabbabbabba
bbbbaabababababbbabaabab
abbbbabbababbaabaabbabab
aaabaabbbbbabaaaaababbab
baaaaabaabaaabbabbaabbbbbaaabbaa
bbbbaabaababbababbbabbab
aababaaabbbbbbbabbbababbbabbaabbabaaabbb
aabbbaaaaaaaabaabaaaaaabaabbbbbbaabababb
aaaaaabbbaabaabbaabbabba
aaaaabbaaaabaabbabaababa
aaabababbbbaaabbabbaaabbbbbaaaaababaaaaaababbababbbabaab
abaababbabaabbabbbbaababbbaabbba
aababaaabbbbbabaabbabbbbabbabbbbaaabbbbbabbababbbbaaabababaaaaba
aaabaaababbababbaaaabbbabbbbabaaaabbbaba
bbababbbaaabbaababaabbbbaaaabbbabaaaabbababbabbbabbaaaaaabbaaabababaabba
aaaabbaabaaaaaabbababbbbaababbaa
abaababbaaabbbbabbaabaabbbbbaabaababbbabbabbaaab
aababbbbaabbaaaabbaabbabababbabaaabaaaaabaaaabbbabababab
babbbabbbbbaaabbaabbabbb
bbaaaaaaaabbbabbaabaabbabbbbaaaabbababba
bababbbbbbbaabbbbbbabbab
bbbaaabbabbbbbaaababbbbb
abbbbbbbbbbbbaabaabbabbabbbbbabbaabbabbaabaabaababbbabbbbbbbbbaabbbabaaaabbbaaba
aabbbbbbaaaaaabbbbbbaaababbbbbabbbaabaaabbbbbbbb
bbaabababaababbbaaabbbab
bbbbbabbbbbbaabababbabbb
bbabbabbabababbaaaababbbbaabababbaabbabb
aabbbbbbbaaabbabaaabbbbbbaabbaaaaaaabbbb
bababbbaababbaabbaaababbabbabbbbbababbbaaaabbbbbabbabbaa
aabbbaaaaaaabaaaaababaaababbbbabaabaabab
abbaabaaaaabaaabbbaaabbbaaabbbbbbbbbbbaaaabbaabaababbbbb
bbbabbbababababaabbabbbaaaaaabaaaaaaabaabbbbaabaaabbabaabaababbbaaaaabaaabbbbbbbbaaabbaabaaaabaa
baabaaabbaabbbaaaaabbbaa
babaaaaaabbabbababbbabba
bbbbaabbbbabbbbaaaabbbbb
aabaaabbbabbbabbbbabaabbaabbbabbbbaaabaabababaaabbabbaaaabbaabbaaabababb
bababababbbbaaaabaabbbbaaababbaaaaabbaab
bababaabbaaaababaaaaaaba
abbababbbabbaabababbbbaaaabababaaaabababbababaaa
abbaabaaabbabaabbbaaabbbbaaabbbb
abaaababbbaababbabbbabbbaaababaa
bababbbbabbbbababbbabbab
baaabbbababbbabbaabbbbba
aaaabbbabbaaabaaaabbbaaabbababba
bbbaaaabbbabbbbaabbbabba
abaaaaabbbbaaaaabbabaaabaabbabbababbbbbb
aaaaabbaaaabababbbbaaaba
ababaaaaabaabbabbbbaababbbbbbbaababbaaaa
babbbabbabaaaaaabababababaabaababbbabbaa
aaaaaabbaaabaababbaaabba
baabbbaabbababaabbbbaaaabbaabaabbabaabbbaaaababa
abbabaaabaaaabbbaaaaabbb
aababbbaabbbbbabbbababaabbbaabababbbabaaabbbbaab
abbabaaababbabaaabbabbabbaabbbab
bbabbbbaabbbbbabbbaabaabaaabaababbbabaababaaabbbabbaabba
abaabbbabaabbabababaaabbbaabbabaaaababaababbbaabaababbab
abababbabbbababbabbbbabbbabaabab
aaaaabbabaaabaaaabaaabaa
aaabbbbabbabbabbababaaaaaaaaabaabaabbbbb
ababbababbbbabbabbbaaaaabbabbaba
abbbbbbababbbbaaabbabaabaabababb
bbaaabaaaaaabaababaabbabababbaaabbaababbaaabaababbaababbaabababbbaaababaaabbaaabbbbbbaaa
aaaabaaabaababbbabbbbbbaabaabaaa
bbaabaabaababaaaabbbabaabbaabbab
abbbabaaaaaaaabbaababbbaabaaaaababaaaaaabbaabbaa
abbabbababbbabbbabbbabbbaaaabaaabbaabbaaabbbabbabbbbbbbb
bababaabaababbbaabbababaababaaab
bbaaabaaababbaaabbaabbba
aabaabbaabaaaaaabbababbb
ababbbbbbbbbbabbbabbaabbbbbaabababababbabbaabbabaaaabbbbabbbbaab
abaabbaaabbababaabbbaabb
ababaaaaabbabbbababaaaba
baabaabbabaabbaabaababaaabbbbbbbaababaaabaaabbaaabaabbbb
baaabbabaaabbbbbaababaabbabaaabaaaababaa
bbaaabbbbbbbaaabaaaaaaba
aabaaaababbaaaabbbbbbabbaabbaaba
abaabbababbaaaabbababbbaaaabbabb
bbaaababbaabbbababaaaaabbababaabaaaabbaaababbaaaaaaaababaaabbaab
aababbbabababbbaabbbabba
)";

auto const res1 = parse_solve(input);
fmt::print("res to part 1 is {}\n", res1);

auto const res2 = hack_solve(input);
fmt::print("res to part 2 is {}\n", res2);

}