// Advent of code 2020, day 12
// Godbolt link: https://godbolt.org/z/jqW949

#include <algorithm>
#include <array>
#include <string_view>
#include <vector>
#include <fmt/core.h>

int to_int(std::string_view const s) {
  int res = 0;
  for (auto const c : s) {
    res = res*10 + (c - '0');
  }
  return res;
}

using Instruction = std::pair<char, int>;

Instruction parse_line(std::string_view const s) {
  auto const c = s.front();
  auto const i = to_int(s.substr(1)); 
  return {c, i};
}

auto parse(std::string_view text)
{
  std::vector<Instruction> res;
  while (!text.empty()) {
    auto const pos = text.find('\n');
    res.push_back(parse_line(text.substr(0, pos)));
    text.remove_prefix(pos+1);
  }
  return res;
}

using Position = std::pair<int, int>;

struct State {
  char direction = 'E'; // 'N', 'E', 'S', 'W'
  Position pos = {0, 0};
};

void move(Position& pos, char direction, int distance) {
  switch (direction) {
    case 'E': {
      pos.first  += distance;
      break;
    }
    case 'W': {
      pos.first  -= distance;
      break;
    }
    case 'N': {
      pos.second += distance;
      break;
    }
    case 'S': {
      pos.second -= distance;
      break;
    }
  };
}

void rotate_clockwise(State& state, int degrees) {
  auto const times = (degrees / 90) % 4 + 4;
  std::array<char, 4> dirs = {'N', 'E', 'S', 'W'};
  auto const cur_idx = std::distance(begin(dirs), std::find(begin(dirs), end(dirs), state.direction));
  auto const idx = (cur_idx + times) % 4;
  state.direction = dirs[idx];
} 

struct Waypoint {
  Position pos = {10, 1}; // relative to ship
};

void rotate_clockwise(Waypoint& w, int degrees) {
  auto const times = ((degrees / 90) % 4 + 4) % 4; // \in {0...3}
  switch (times) {
    case 0: {
      // nothing to be done
      break;
    }
    case 1: {
      w.pos = { w.pos.second, -w.pos.first };
      break;
    }
    case 2: {
      w.pos = { -w.pos.first, -w.pos.second };
      break;
    }
    case 3: {
      w.pos = { -w.pos.second, w.pos.first };
      break;
    }
  }
} 

void advance_1(State& state, Waypoint& waypoint, Instruction const& i) {
  (void) waypoint; // unused in part 1
  switch (i.first) {
    case 'N': [[fallthrough]];
    case 'E': [[fallthrough]];
    case 'S': [[fallthrough]];
    case 'W': {
      move(state.pos, i.first, i.second);
      break;
    }
    case 'R': {
      rotate_clockwise(state, i.second);
      break;
    }
    case 'L': {
      rotate_clockwise(state, -i.second);
      break;
    }
    case 'F': {
      move(state.pos, state.direction, i.second);
    }
  }
}

void advance_2(State& state, Waypoint& waypoint, Instruction const& i) {
  switch (i.first) {
    case 'N': [[fallthrough]];
    case 'E': [[fallthrough]];
    case 'S': [[fallthrough]];
    case 'W': {
      move(waypoint.pos, i.first, i.second);
      break;
    }
    case 'R': {
      rotate_clockwise(waypoint, i.second);
      break;
    }
    case 'L': {
      rotate_clockwise(waypoint, -i.second);
      break;
    }
    case 'F': {
      move(state.pos, 'E', waypoint.pos.first  * i.second);
      move(state.pos, 'N', waypoint.pos.second * i.second);
    }
  }
}

template<class F>
State execute(std::vector<std::pair<char, int>> const& instructions, F&& f) {
  State res;
  Waypoint waypoint;
  for (auto const& i : instructions) {
    f(res, waypoint, i);
  }
  return res;
}

int manhattan_distance(Position const& lhs, Position const& rhs) {
  return std::abs(lhs.first - rhs.first)
       + std::abs(lhs.second - rhs.second);
}

int main() {

auto const test = R"(F10
N3
F7
R90
F11
)";

auto const input = R"(L90
F67
R270
W1
R180
F5
E5
F59
E4
L180
F70
S2
F35
N3
E5
F58
L90
N1
F46
R90
S1
R90
E1
L180
W4
F99
N2
F84
N1
R90
N5
W4
F26
E1
F97
N1
F36
W1
F21
S4
F31
S3
F76
S5
S1
L90
S4
W4
R90
E4
F14
R90
S2
R90
S3
F21
N1
W4
S4
E1
L180
N5
F30
N3
F4
N5
F100
N2
R270
E1
S1
F79
N4
F72
W4
F50
L90
W5
S4
E2
N5
E4
S5
W5
L90
E4
L90
S4
E4
R90
N1
W5
R270
W5
N4
R180
E5
F86
L90
W3
F79
W5
F87
L180
N4
E2
S1
W3
N3
F31
W2
N1
F86
E1
L90
L90
F2
E3
F8
L90
F54
W3
S5
E3
F89
N5
R90
E3
F70
N2
R90
F55
W3
R90
F44
E2
F36
L90
E3
S2
F23
N4
F2
W5
L180
E4
N4
W3
F58
W1
R90
W1
L90
E1
F99
W4
S4
E5
N2
R180
E5
F82
N3
F99
L90
N4
E4
S5
R90
N3
F17
S5
E4
F58
E1
N1
E5
R90
F32
N1
R90
F84
E4
W4
R180
E4
R90
N1
F26
W4
R90
F96
E5
S2
F86
R90
F95
S4
F81
R90
W4
F44
N3
W3
N5
L180
L90
F71
S4
R90
E5
N4
F63
W2
F75
N3
R90
S2
E3
F75
R90
W3
F4
L90
E3
F96
L180
F53
W5
L90
F12
N2
F100
W2
R270
S1
F37
E4
S1
E1
L270
W2
S5
F10
L90
N3
F63
L90
F96
S3
W1
N4
R180
E2
F51
L90
N4
F27
W3
N5
R90
N4
L180
F4
N1
L180
F71
E5
S5
F94
L90
F98
E3
N4
E5
R90
F75
S1
F19
E2
F53
S3
L90
F29
R180
F88
R180
F3
S2
E5
F16
L90
E1
S2
E3
F28
E5
F22
L180
S2
E1
S1
F6
E2
S3
F14
R90
N4
S5
F77
L90
N3
R90
N2
L180
F99
E2
F85
S3
F81
N1
W1
F91
F31
N5
W5
R90
S1
F40
N2
E1
S3
L90
E5
R180
E2
L90
F88
R90
F45
R270
W4
F67
W4
S1
W4
F65
L90
F19
R90
F83
S1
R90
E2
R180
F78
E1
E1
L180
S1
E1
N4
W5
F98
L90
E4
L90
N2
E1
N4
E1
N5
L90
S3
F52
W5
F55
S4
R180
F56
S5
E1
R90
F97
E5
N4
L90
E1
N1
W1
N4
L270
F7
N3
L90
W3
L270
F27
E2
N5
F90
N3
R90
F79
N4
F58
L90
W5
F90
F9
E5
R90
F16
E4
F50
S1
R90
N5
E2
F86
E3
L270
W3
L90
W1
F17
N2
L180
N1
W4
R180
F10
N3
W3
L90
E2
F12
S5
L90
N3
W4
N3
F19
E5
F54
E1
F34
F2
S4
F14
R90
S4
F2
N1
E3
N2
L180
E5
F67
L180
F66
E3
S4
W3
F51
L270
N5
F51
W3
S2
E2
N2
F27
W5
F77
E4
N5
E2
F20
N5
E4
S5
F67
S2
F81
L90
F68
E4
F71
L90
F48
N3
F1
N5
R90
F76
W5
S5
F74
S1
E2
F52
R90
W1
S4
F13
F69
L180
F59
N3
F34
F84
R90
F63
W2
L90
F12
L90
W5
F25
F83
E4
N1
R90
F36
S1
W2
F41
R90
N3
W1
R180
W2
L90
N4
F87
E3
S4
F10
S3
F33
R90
E1
L180
F32
W5
S3
F23
R90
F44
L90
F45
E2
L270
F41
W1
F54
L180
F31
R90
F43
S3
F91
F88
L180
F2
W2
N5
W2
S1
L180
F12
N2
F2
N3
W2
R90
S2
E4
F66
S2
W4
F94
S5
E1
L180
N5
F2
N2
R180
E3
F3
E1
R90
S3
F28
L90
F12
L90
S2
F100
L90
F84
E2
R90
W4
F14
N1
W3
F33
W1
N5
R180
F93
W5
N2
E4
L180
W3
F2
S1
W4
L90
F8
W2
F83
E5
R180
W4
S4
R90
E4
R180
F84
E2
N3
W3
N1
L90
F76
W1
F9
E1
S1
E5
L90
S1
S5
W4
S3
F20
N2
F52
R180
F21
W4
N2
L90
F42
S3
E5
N4
F100
E5
N5
F56
L90
F90
S1
E2
N2
F42
E3
L90
W4
R180
F22
L90
R90
F48
E4
N4
E5
F10
L90
N5
F99
S4
E3
R90
N5
E3
F85
F83
W1
R180
L90
W4
R90
W1
L90
S4
L90
N3
W5
L90
R90
F68
N2
W5
N4
W3
L90
E1
W1
L180
R90
F45
E5
R90
W5
S4
F5
L180
N1
R90
S4
E3
F22
R180
W4
L180
S3
L90
N5
E5
N1
F6
S5
W1
F86
R180
S1
R90
E5
N2
L90
W4
N1
W3
R90
F1
R180
F94
L90
E5
F7
R90
F72
R90
N3
N1
L180
N4
L90
N5
E1
N1
L270
S2
R90
F8
N4
E2
F8
S5
E2
S3
L90
F67
E4
F54
E1
F100
N2
F20
)";

auto const instructions = parse(input);

auto const state = execute(instructions, advance_2);
auto const d = manhattan_distance(state.pos, {0, 0});

fmt::print(">> distance from origin is {}\n", d);

}