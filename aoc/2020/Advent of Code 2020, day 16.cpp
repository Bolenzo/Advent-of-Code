// Advent of Code 2020: day 16

// Godbolt link: https://godbolt.org/z/63cxqh

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <vector>
#include <fmt/core.h>

bool is_digit(char const c) {
  return '0' <= c && c <= '9';
}

template<class It>
int to_int(It first, It const last) {
  int res = 0;
  while (first != last) {
    res = res*10 + (*first++ - '0');
  }
  return res;
}

std::string_view fetch_line(std::string_view& text) {
  auto const pos = text.find('\n');
  auto res = text.substr(0, pos);
  text.remove_prefix(pos+1);
  return res;
}

void skip_to_digit(std::string_view& text) {
  auto const it = std::find_if(begin(text), end(text), is_digit);
  auto const pos = std::distance(begin(text), it);
  text.remove_prefix(pos);
}

int fetch_int_nosign(std::string_view& text) {
  skip_to_digit(text);
  auto const last = std::find_if_not(begin(text), end(text), is_digit);
  auto res = to_int(begin(text), last);
  text.remove_prefix(std::distance(begin(text), last));
  return res;
}

//////////////////////////////////////////////////////////////////////////

struct Interval // [lhs, rhs]
{
  int lhs;
  int rhs;
};

class IntervalSet {
  std::vector<Interval> intervals_;
public:
  void add(Interval interval) {
    intervals_.push_back(interval);
  }
  bool contains(int const i) const {
    return std::any_of(begin(intervals_), end(intervals_), [&](auto const& interval) {
      return interval.lhs <= i && i <= interval.rhs;
    });
  }
};

using Rules = std::unordered_map<std::string, IntervalSet>;

auto get_field(std::string_view line)
{
  auto const colon = line.find(':');
  std::string name(begin(line), std::next(begin(line), colon));
  IntervalSet intervals;
  while (!line.empty()) {
    auto const lhs = fetch_int_nosign(line);
    auto const rhs = !line.empty() && line.front() == '-'
                   ? fetch_int_nosign(line) : lhs;
    intervals.add({lhs, rhs});
  }
  return std::pair{name, intervals};
}

auto parse_fields(std::string_view& text)
{
  Rules res;
  for(auto line = fetch_line(text);
      !text.empty() && !line.empty();
      line = fetch_line(text)) {
    res.insert(get_field(line));
  }  
  return res;
}

struct Ticket {
  std::vector<int> values;
};

Ticket parse_ticket(std::string_view line) {
  Ticket res;
  while (!line.empty()) {
    res.values.push_back(fetch_int_nosign(line));
  }
  return res;
}

auto parse(std::string_view text)
{
  auto const fields = parse_fields(text);
  skip_to_digit(text);
  auto const ticket = parse_ticket(fetch_line(text));
  skip_to_digit(text);
  std::vector<Ticket> nearby;
  while (!text.empty()) {
    nearby.push_back(parse_ticket(fetch_line(text)));
  }
  return std::tuple{fields, ticket, nearby};
}

int error_rate(Ticket const& ticket, Rules const& rules) {
  int res = 0;
  for (auto v : ticket.values) { // accumulate
    if (std::none_of(begin(rules), end(rules), [&](auto const& p){
      return p.second.contains(v);
    })) {
      res += v;
    }
  }
  return res;
}

int total_error_rate(std::vector<Ticket> const& tickets, Rules const& rules) {
  int res = 0;
  for (auto const& t : tickets) { // accumulate
    res += error_rate(t, rules);
  }
  return res;
}

bool is_invalid(int const value, Rules const& rules) {
  return std::none_of(begin(rules), end(rules), [&](auto const& p) {
    return p.second.contains(value);
  });
}

bool is_invalid(Ticket const& ticket, Rules const& rules) {
  return std::any_of(begin(ticket.values), end(ticket.values), [&](int const v) {
    return is_invalid(v, rules);
  });
}

std::size_t remove_invalid(std::vector<Ticket>& tickets, Rules const& rules) {
  auto const invalid_ticket = [&](Ticket const& ticket) {
    return is_invalid(ticket, rules);
  };
  auto const n = tickets.size();
  tickets.erase(std::remove_if(begin(tickets), end(tickets), invalid_ticket), end(tickets));
  return n - tickets.size();
}

class Possibilities {
  std::unordered_map<std::string, std::unordered_set<std::size_t>> field_to_idx_;
  std::unordered_set<std::string> fields_to_solve_;
  std::unordered_set<std::size_t> indexes_to_solve_;
public:
  explicit Possibilities(Rules const& rules) {
    auto const n = rules.size();
    auto const indexes = [&] {
      std::unordered_set<std::size_t> res;
      for (std::size_t i=0; i < n; ++i) {
        res.insert(i);  
      }
      return res;
    }(); // IIFE
    indexes_to_solve_ = indexes;
    for (auto const& p : rules) {
      field_to_idx_.emplace(p.first, indexes);
      fields_to_solve_.insert(p.first);
    }
  }

  void remove(std::size_t idx, std::string const& field) {
    field_to_idx_.at(field).erase(idx);
  }

  void simplify() {
    simplify_by_field();
    simplify_by_idx();
  }

  bool is_unique() const {
    return fields_to_solve_.empty();
  }

  auto solution() const {
    std::unordered_map<std::string, std::size_t> res;
    if (is_unique()) {
      for (auto const& [f, idx] : field_to_idx_) {
        res.emplace(f, *begin(idx));
      }
    }
    return res;
  }

private:
  void simplify_by_field() {
    for (auto const& [field, indexes] : field_to_idx_) {
      if (indexes.size() == 1) {
        fields_to_solve_.erase(field);
        auto const idx = *begin(indexes);
        for (auto const& f : fields_to_solve_) {
          remove(idx, f);
        }
      }
    }
  }
  void simplify_by_idx() {
    for (std::size_t idx = 0; idx < field_to_idx_.size(); ++idx) {
      if (std::count_if(begin(field_to_idx_), end(field_to_idx_), [&](auto const& p) {
                        return p.second.contains(idx);
        }) == 1) {
        indexes_to_solve_.erase(idx);
        auto const it = std::find_if(begin(field_to_idx_), end(field_to_idx_), [&](auto const& p) {
                        return p.second.contains(idx);
        });
        for (auto const& i : indexes_to_solve_) {
          remove(i, it->first);
        }
      }
    }
  }
};

auto match(std::vector<Ticket>const& tickets, Rules const& rules) {
  auto ticks = tickets;
  remove_invalid(ticks, rules);
  auto p = Possibilities(rules);
  for (auto const& t : ticks) {
    for (std::size_t i=0; i < t.values.size(); ++i) { // enumerate
      auto const v = t.values[i];
      for (auto const& [f, intervals] : rules) {
        if (!intervals.contains(v)) {
          p.remove(i, f);
        }
      }
    }
  }
  while (!p.is_unique()) {
    p.simplify();
  }
  return p.solution();
}

using SolvedTicket = std::unordered_map<std::string, int>;

auto solve(Ticket const& ticket, Rules const& rules, std::vector<Ticket>const& tickets) {
  auto const solver = match(tickets, rules);
  SolvedTicket res;
  for (auto const& [field, idx] : solver) {
    res.emplace(field, ticket.values[idx]);
  }
  return res;
}

std::int64_t product_start(SolvedTicket const& ticket, std::string_view const s) {
  std::int64_t res = 1; // accumulate w/ product
  for (auto const& [f, v] : ticket) {
    if (f.starts_with(s)) {
      res *= v;
    }
  }
  return res;
}

//////////////////////////////////////////////////////////////////////

int main() {

auto const test = R"(class: 1-3 or 5-7
row: 6-11 or 33-44
seat: 13-40 or 45-50

your ticket:
7,1,14

nearby tickets:
7,3,47
40,4,50
55,2,20
38,6,12
)";

auto const test2 = R"(class: 0-1 or 4-19
row: 0-5 or 8-19
seat: 0-13 or 16-19

your ticket:
11,12,13

nearby tickets:
3,9,18
15,1,5
5,14,9
)";

auto const input = R"(departure location: 48-885 or 906-949
departure station: 28-420 or 431-970
departure platform: 45-112 or 129-967
departure track: 41-447 or 459-956
departure date: 36-913 or 934-962
departure time: 34-792 or 815-952
arrival location: 35-644 or 657-973
arrival station: 45-550 or 563-970
arrival platform: 41-533 or 539-973
arrival track: 33-729 or 745-967
class: 36-516 or 523-956
duration: 37-94 or 108-960
price: 35-669 or 678-974
route: 37-834 or 847-968
row: 35-295 or 312-956
seat: 35-563 or 588-951
train: 29-210 or 221-968
type: 50-853 or 877-958
wagon: 28-341 or 353-971
zone: 43-413 or 421-962

your ticket:
163,151,149,67,71,79,109,61,83,137,89,59,53,179,73,157,139,173,131,167

nearby tickets:
850,523,269,266,158,934,526,172,504,475,941,65,179,269,197,130,405,625,471,675
157,110,437,692,556,370,85,365,460,709,336,174,940,825,194,485,436,383,714,292
368,277,514,290,164,624,354,688,480,386,746,363,294,668,618,320,486,324,890,83
515,208,942,4,948,785,280,635,783,473,907,681,533,92,282,76,528,323,395,239
266,757,435,819,140,272,589,483,665,722,773,66,191,232,701,564,260,393,436,790
459,830,935,778,479,191,152,390,180,851,483,538,488,695,94,56,357,293,237,495
153,204,475,146,699,316,790,702,163,63,2,745,512,592,191,153,288,282,72,210
937,199,602,685,931,374,243,618,81,356,81,712,684,669,620,238,153,389,468,882
729,318,154,644,762,147,935,699,589,338,92,465,91,92,265,812,949,86,358,226
83,410,64,88,406,292,76,152,712,232,361,267,668,865,408,602,243,411,729,503
657,550,819,514,238,884,133,819,678,910,859,791,334,175,764,270,747,503,524,152
199,466,756,201,705,631,391,375,91,174,61,151,936,512,155,641,259,635,66,522
225,641,146,466,68,482,482,714,273,992,51,177,635,641,696,183,223,832,275,592
725,489,474,523,726,158,909,50,595,85,770,823,542,431,756,612,538,472,246,143
747,315,412,90,256,668,433,441,909,756,1,502,322,374,197,249,749,90,725,911
182,831,823,543,397,153,529,341,210,220,696,772,659,369,412,171,136,506,163,362
705,754,548,254,256,541,202,176,224,131,654,907,385,371,400,636,503,824,532,81
693,616,434,324,466,529,541,640,173,596,720,189,159,134,654,640,748,715,80,750
467,210,166,337,204,142,512,109,598,949,731,755,160,83,130,129,269,679,270,237
91,769,821,55,339,197,275,129,242,135,216,853,941,485,906,600,242,412,242,473
539,771,746,686,726,197,191,609,686,533,0,196,330,533,263,780,726,508,593,386
777,828,255,792,156,608,677,614,201,621,563,681,616,52,817,757,782,706,358,936
179,68,773,774,423,512,473,375,686,714,820,605,432,605,617,475,540,258,244,625
370,631,472,131,530,211,239,396,280,466,789,64,132,616,726,721,234,89,135,377
76,637,82,604,398,55,728,389,682,533,525,287,444,193,488,61,159,716,323,900
481,175,767,167,789,830,642,385,465,847,666,170,759,407,284,267,854,543,761,816
494,272,246,529,620,238,720,525,853,109,550,88,754,630,444,518,380,483,722,679
167,138,408,62,685,374,134,773,682,0,833,539,54,706,760,291,783,488,479,360
169,229,622,76,440,823,360,831,75,607,766,411,368,201,248,514,574,190,439,139
368,438,227,240,755,546,264,62,68,751,745,351,140,372,502,709,280,949,173,76
980,643,470,87,334,549,487,444,627,477,68,330,165,715,194,544,678,160,173,946
319,682,760,481,140,312,485,158,678,815,826,522,66,332,947,755,270,627,758,233
754,64,772,533,395,399,600,143,384,499,89,553,400,911,539,209,91,334,707,111
224,177,729,270,260,981,324,66,725,78,332,330,680,176,763,627,205,395,259,390
251,358,913,849,60,285,780,239,275,254,181,476,270,664,383,634,63,143,127,384
729,410,238,251,333,236,199,907,636,207,81,118,79,81,254,942,826,389,710,382
723,745,91,934,194,682,879,774,91,180,319,601,784,593,497,129,633,494,819,988
548,162,330,178,937,540,711,184,470,66,719,411,769,270,614,324,502,562,374,934
662,288,129,292,275,762,78,240,833,516,877,158,496,219,169,134,593,753,259,488
145,433,746,152,379,67,526,815,936,207,85,396,133,359,398,199,733,63,831,70
638,613,697,699,719,398,356,884,321,534,822,282,548,774,240,588,594,279,611,256
8,59,369,322,784,497,186,69,721,499,172,365,141,433,262,69,789,295,948,491
447,50,216,711,79,635,712,245,717,62,89,162,194,276,227,624,133,167,192,314
270,282,467,112,657,258,633,326,183,687,518,378,154,327,589,604,263,319,337,596
357,828,182,194,607,664,470,112,546,90,489,194,609,882,912,274,626,527,454,820
337,785,270,280,523,378,438,335,699,186,817,99,230,271,723,830,502,707,762,503
73,162,723,223,601,194,637,82,403,791,770,409,817,82,938,243,760,674,516,608
167,947,705,501,228,287,460,482,412,832,469,549,312,703,290,623,949,909,397,452
550,325,330,628,981,818,599,88,81,848,686,264,614,152,725,591,135,74,498,437
288,495,360,688,174,475,290,776,76,704,209,704,294,594,108,213,480,498,192,52
781,946,15,178,481,185,381,847,783,753,848,473,881,288,233,768,512,776,661,784
396,167,405,849,899,506,749,546,878,515,786,231,361,472,725,609,459,334,191,529
445,68,182,133,195,513,788,822,218,604,936,226,276,206,510,751,469,725,314,779
398,132,152,665,758,153,353,131,407,502,671,50,386,768,227,335,393,821,480,664
825,413,356,689,936,382,227,440,223,187,747,827,169,506,756,162,481,549,828,553
642,264,83,328,501,55,317,172,611,473,437,725,244,621,755,475,711,733,362,716
602,154,227,602,492,509,879,277,239,140,272,826,434,767,946,357,656,688,399,906
612,77,615,787,364,83,246,878,884,76,500,702,512,83,592,286,896,132,502,280
910,611,720,171,161,660,613,785,689,294,613,514,729,449,288,774,403,465,339,83
787,936,697,490,608,393,321,132,60,229,820,469,272,743,750,474,763,443,226,616
398,880,148,410,941,658,223,443,477,284,405,776,144,505,286,788,143,878,338,565
112,602,716,509,152,512,704,185,75,136,607,369,63,495,723,179,83,680,426,88
690,327,550,947,546,522,728,175,231,382,165,262,151,688,776,498,710,184,792,171
73,765,593,827,766,642,521,108,145,848,685,662,688,393,708,92,52,597,224,667
679,53,695,63,274,277,536,718,255,817,293,683,512,328,481,495,91,135,549,627
246,197,617,222,457,707,364,483,936,375,230,255,189,682,686,710,260,728,705,182
404,81,615,502,486,81,594,977,907,935,817,487,152,508,83,705,89,179,321,437
238,140,126,766,380,885,338,183,389,711,610,210,833,178,173,748,782,251,129,834
288,481,80,773,499,716,85,518,883,318,445,143,765,776,250,531,386,247,321,755
934,500,634,781,288,536,339,779,83,946,329,267,511,134,693,476,486,496,234,528
528,542,258,260,487,390,92,670,376,359,659,146,490,257,63,319,295,313,239,747
353,411,362,208,244,388,321,621,634,129,408,235,635,689,286,238,550,520,563,51
327,178,71,381,826,820,598,276,439,277,404,400,714,553,190,314,592,388,154,912
515,851,487,477,374,939,498,681,312,492,460,447,112,244,731,506,439,82,246,541
881,603,388,248,773,157,353,708,515,53,539,237,825,771,77,453,328,93,111,613
65,189,237,185,881,750,447,130,663,729,235,785,789,645,110,183,331,274,210,827
266,501,450,371,465,545,323,64,54,593,143,748,318,66,818,222,57,502,228,201
907,474,431,815,675,468,191,689,136,541,181,153,668,513,141,397,370,880,725,378
935,605,757,380,745,849,591,764,395,74,807,225,853,822,259,697,476,234,285,50
154,324,398,360,782,272,56,175,153,791,986,198,785,492,509,467,374,373,391,636
792,913,271,375,945,163,764,498,591,661,607,913,130,462,502,198,333,746,554,73
657,149,499,701,638,788,820,490,147,144,669,197,149,68,462,162,615,570,197,137
259,322,626,288,908,341,489,384,164,357,486,246,777,698,822,432,849,850,740,698
238,588,542,199,779,267,474,382,156,598,401,395,322,618,395,737,194,410,156,199
547,714,780,942,818,485,678,405,192,267,704,828,331,517,563,907,68,222,87,237
773,751,470,735,785,73,749,141,713,602,516,695,853,295,401,148,327,781,251,658
948,295,86,879,723,391,439,399,326,82,697,405,597,885,130,169,551,154,285,230
274,380,901,76,783,379,262,597,723,409,390,162,365,435,61,466,264,392,768,167
589,365,256,885,182,170,255,830,847,525,697,413,192,266,294,631,318,235,309,312
330,611,199,135,172,252,69,272,616,326,902,320,906,166,189,783,182,494,832,771
635,945,368,479,788,387,396,198,593,734,667,774,51,73,599,880,617,827,222,221
782,160,282,768,224,779,271,361,822,72,721,709,911,320,153,72,644,493,809,144
816,147,386,613,326,479,500,405,83,395,134,702,329,112,776,940,165,824,835,943
146,505,190,321,909,940,530,508,412,364,493,381,406,64,603,188,188,650,619,76
728,242,707,410,313,877,831,605,284,357,392,69,731,267,412,194,56,850,525,611
528,186,377,758,505,610,134,910,528,779,70,826,236,280,610,299,496,189,780,479
477,548,225,532,477,198,199,425,371,267,377,613,93,394,444,539,72,602,432,388
189,600,678,591,775,629,398,754,223,980,542,775,944,691,782,88,390,750,384,184
324,474,392,541,413,714,639,174,68,518,498,464,759,630,275,944,333,327,371,172
68,231,746,470,771,949,525,265,262,179,196,768,334,69,205,222,457,528,485,469
148,324,787,665,328,936,136,272,614,789,395,212,52,598,175,618,589,178,848,756
940,205,500,481,357,815,510,476,482,945,196,625,333,65,728,758,325,139,19,205
176,627,783,724,732,206,706,256,223,716,440,476,161,207,353,640,906,483,151,542
327,692,326,333,990,544,830,170,382,112,665,530,177,337,508,728,246,696,627,146
680,728,629,997,232,765,747,523,685,136,767,531,60,254,74,479,50,477,135,134
111,181,172,503,52,205,679,228,325,341,512,755,466,77,394,382,632,775,870,313
466,486,443,166,240,322,470,721,179,638,539,480,536,130,329,783,340,143,637,641
398,367,531,356,684,501,272,766,681,641,620,632,548,82,530,375,675,644,911,330
991,269,86,725,144,65,605,882,161,664,617,65,714,169,184,353,228,433,65,93
230,718,362,832,949,225,64,60,93,325,316,155,776,631,179,942,827,376,724,864
382,269,489,408,573,295,907,146,275,62,472,85,56,238,443,185,635,774,408,848
136,770,431,289,614,695,368,325,775,319,493,357,828,365,828,104,816,183,685,133
697,684,662,386,607,592,111,539,480,728,200,433,528,589,535,748,544,322,135,701
877,407,500,713,268,252,195,158,721,691,162,725,363,469,93,174,404,855,548,92
207,642,153,74,699,136,526,338,363,858,243,357,488,949,784,693,663,461,782,268
529,224,702,75,61,138,636,207,193,131,412,587,79,765,55,73,766,412,606,447
550,81,16,335,282,608,50,366,595,754,400,384,147,55,242,512,438,129,527,591
189,779,652,816,477,439,295,498,615,644,694,280,495,722,278,165,354,172,824,495
947,88,293,512,131,722,714,77,164,559,467,229,850,783,336,244,77,271,251,697
336,659,848,544,355,141,655,189,942,506,328,496,877,729,137,619,162,68,504,78
472,622,668,268,229,494,466,403,158,701,369,167,135,779,441,617,946,132,538,192
481,626,203,507,483,292,54,143,910,225,753,137,596,881,254,861,669,616,286,382
237,248,136,321,431,531,133,360,487,277,660,942,709,130,766,247,169,217,788,623
822,563,295,112,753,879,321,833,480,191,642,921,881,603,510,241,497,387,283,749
823,337,158,617,473,979,240,164,505,489,831,147,54,397,486,624,198,852,232,399
629,994,600,56,406,294,947,158,628,440,329,698,706,202,149,204,337,369,851,729
396,644,626,777,682,408,205,691,595,717,229,625,356,145,165,821,261,472,352,760
668,405,160,404,609,608,549,735,435,626,949,825,231,247,341,92,695,403,411,756
660,151,692,320,937,262,261,372,503,205,467,180,325,712,502,201,220,880,641,294
945,757,637,145,507,694,532,632,401,232,644,408,3,328,631,145,151,697,388,334
400,481,239,825,382,706,544,221,818,474,83,330,74,849,213,411,221,75,850,589
546,375,509,818,660,724,57,534,87,465,93,625,313,681,337,366,162,880,539,623
592,411,149,563,260,945,383,234,881,292,548,946,262,943,751,918,851,197,543,265
74,742,246,235,668,882,263,376,289,781,85,176,135,92,634,530,396,588,906,189
643,605,510,210,868,242,592,530,87,316,168,447,512,209,524,393,909,222,509,357
632,818,623,593,274,64,208,174,741,314,753,176,162,271,635,508,163,434,822,181
262,269,986,364,323,698,381,542,909,526,181,942,143,851,667,258,143,761,774,232
710,694,376,506,948,647,758,501,847,824,627,550,290,206,386,765,718,73,848,705
749,767,62,206,394,187,441,523,132,339,282,180,328,473,526,887,716,66,90,588
830,833,728,466,631,111,981,681,466,747,783,935,244,688,478,758,702,180,609,395
507,225,381,479,745,111,852,758,154,774,188,377,906,762,722,477,4,548,356,355
372,109,91,776,134,317,710,362,200,821,762,940,526,136,817,752,16,702,817,776
778,824,249,686,60,196,327,483,89,604,941,773,2,436,155,937,176,82,334,270
172,318,560,714,251,470,495,515,185,318,83,57,459,548,327,669,170,249,488,698
205,533,163,412,292,268,699,475,488,202,495,309,150,778,616,400,466,659,146,724
849,625,805,174,711,678,65,327,438,727,601,150,463,200,641,67,326,745,589,482
678,643,318,133,372,373,370,368,390,276,137,887,283,410,194,93,158,223,404,591
191,476,355,258,372,780,217,461,783,413,707,163,137,747,591,690,111,530,195,439
261,512,873,59,523,909,177,435,688,152,204,787,224,497,513,621,531,386,133,188
666,253,104,447,86,823,756,833,773,610,179,700,539,702,192,256,265,255,936,637
168,706,132,337,360,165,85,656,696,188,331,776,244,57,401,779,94,411,611,948
763,447,135,722,406,910,87,729,287,331,786,503,287,132,706,111,880,174,675,437
258,269,711,285,404,271,328,158,508,242,81,717,365,203,604,639,224,470,583,181
604,949,321,621,217,159,151,700,622,339,62,493,528,482,729,180,111,366,701,607
465,773,709,248,78,136,830,409,748,545,537,636,775,192,547,491,944,376,249,494
413,510,424,205,715,948,716,383,180,949,788,754,446,685,689,275,530,388,482,149
718,644,142,68,747,633,91,590,462,792,153,849,770,662,354,287,378,335,727,982
183,198,353,827,850,191,716,85,203,181,325,934,506,771,987,628,549,273,751,694
563,353,774,66,185,602,287,547,819,710,317,78,728,784,110,635,186,997,830,254
669,207,727,152,377,719,80,175,544,285,595,755,136,75,517,697,317,395,243,461
173,907,193,234,359,63,247,544,384,384,22,151,663,516,505,284,526,709,254,164
617,285,62,162,549,468,831,780,497,57,478,453,66,241,91,407,945,777,542,446
658,171,439,749,382,829,94,927,634,664,397,134,705,316,338,444,467,378,663,478
354,700,621,599,129,524,595,323,260,491,541,466,171,436,592,263,282,486,330,18
792,638,535,541,401,489,939,334,93,753,601,262,884,884,372,724,246,72,460,277
57,391,606,144,279,56,143,243,65,333,495,677,466,723,767,155,786,681,61,828
314,817,500,468,198,849,293,726,630,394,479,67,68,51,168,206,949,208,471,213
668,511,777,205,175,135,65,256,758,368,283,598,613,674,881,789,595,381,432,662
55,722,750,358,643,768,761,631,81,644,462,655,317,397,221,852,333,657,201,203
847,159,608,81,331,436,180,510,432,501,325,180,283,230,502,200,984,237,761,786
382,476,624,66,232,694,877,693,792,74,481,396,249,503,605,433,521,749,72,604
276,60,276,937,818,738,393,471,382,59,53,550,392,641,92,264,883,221,147,240
853,364,164,493,57,411,270,494,713,446,558,436,470,407,232,669,722,849,760,327
327,188,313,643,153,243,70,315,175,256,232,410,490,5,146,165,188,278,61,753
404,523,264,207,707,80,885,690,50,161,259,226,624,637,258,645,284,595,108,883
265,396,605,69,175,609,885,525,879,676,332,267,465,199,71,87,543,693,497,818
84,749,313,360,766,818,355,583,725,76,697,728,201,137,688,86,279,827,701,637
188,471,747,157,771,50,231,317,631,203,601,473,214,619,476,191,472,760,54,817
589,704,735,625,877,540,763,243,943,591,911,852,545,825,541,766,365,59,431,644
159,546,369,222,382,619,146,448,701,704,825,784,67,482,528,239,784,293,543,512
626,340,2,823,605,666,403,177,533,433,473,642,275,385,259,355,85,321,318,532
946,751,442,681,314,108,387,828,936,691,189,936,58,504,378,558,701,361,698,271
769,817,850,635,704,492,434,548,606,831,789,169,882,532,360,442,789,370,518,947
601,276,977,272,685,657,662,719,439,643,880,252,880,701,786,384,680,363,336,882
680,615,698,199,55,378,478,788,532,385,627,460,150,462,322,329,58,360,214,194
754,336,90,936,912,851,771,663,8,57,397,602,726,148,467,466,254,764,268,944
180,139,172,545,276,640,501,821,462,205,627,437,511,206,380,949,488,993,657,185
462,72,239,822,877,938,481,783,160,443,702,644,393,629,725,334,761,878,788,655
364,526,930,591,263,406,501,61,704,642,640,711,682,667,774,770,57,188,341,509
262,883,680,359,386,865,817,597,760,719,766,479,245,438,281,206,948,487,399,158
948,190,441,789,746,433,331,150,110,819,709,609,290,193,331,946,265,942,930,613
142,975,788,80,60,239,490,183,412,941,628,942,754,324,438,361,139,611,56,248
189,290,144,770,936,399,529,996,163,176,432,629,482,370,64,341,824,295,65,819
490,705,852,159,441,437,747,54,760,152,729,752,170,434,323,128,788,148,270,663
563,354,165,321,777,634,591,869,631,412,631,667,765,691,239,480,382,265,182,530
394,815,401,700,517,143,669,434,909,368,170,642,268,881,235,512,695,338,57,473
544,948,203,507,463,395,603,87,641,312,281,719,756,153,166,674,52,326,251,776
588,629,830,941,535,461,84,364,694,246,910,182,470,853,549,852,226,461,165,174
691,330,989,727,636,178,906,705,757,747,506,364,724,162,139,668,186,754,162,226
399,600,325,207,818,74,631,688,544,180,207,321,539,639,541,91,431,203,492,536
591,285,619,723,801,701,729,355,263,290,753,325,170,779,248,934,140,246,238,703
664,70,832,669,146,284,659,253,710,382,918,487,851,880,819,208,66,398,433,644
130,132,108,823,193,672,179,232,252,470,282,701,822,715,230,472,849,65,150,135
318,721,821,181,948,643,69,618,318,766,163,255,317,642,832,776,290,481,934,731
79,436,364,767,366,94,173,882,312,276,509,629,775,533,613,200,67,536,151,757
403,378,986,778,726,111,605,270,372,240,705,200,721,181,632,161,853,268,150,435
372,908,671,385,433,162,910,679,828,267,683,383,466,470,201,714,787,54,66,597
324,824,483,705,174,385,72,134,247,50,338,177,436,534,755,177,757,109,690,330
335,713,235,506,909,254,614,363,488,75,569,330,314,272,678,708,705,700,171,687
722,658,157,509,0,129,510,748,188,658,232,77,462,697,550,226,496,56,707,334
906,625,353,678,619,54,468,369,606,695,718,813,438,284,367,246,497,619,592,252
481,68,727,507,655,822,90,439,359,463,716,406,721,491,624,400,596,714,438,372
205,365,579,247,763,821,91,167,232,318,274,791,669,93,69,61,59,276,232,746
75,719,194,266,691,289,539,689,609,603,195,384,680,273,323,315,357,620,475,808
174,482,249,775,766,178,911,219,827,532,909,248,228,63,528,295,270,691,606,498
471,512,110,771,224,241,489,65,635,81,680,741,50,817,666,153,943,195,152,714
290,747,227,225,379,631,236,804,181,550,621,533,253,383,54,164,462,604,727,373
728,683,624,388,401,912,79,361,757,818,695,428,82,281,751,372,476,179,909,765
259,80,181,136,589,532,704,376,354,645,405,540,779,906,312,207,754,249,73,259
159,215,509,607,883,934,402,223,697,112,408,934,183,361,84,182,942,372,937,370
207,359,642,661,594,321,504,698,751,438,144,857,779,320,403,321,544,771,192,478
282,604,174,186,123,499,284,490,936,516,447,146,356,945,667,257,432,243,472,720
728,243,518,640,186,525,432,510,758,285,659,291,400,948,238,470,550,826,162,58
880,292,662,588,406,174,471,561,501,90,187,89,403,612,500,355,390,751,468,158
317,762,987,331,434,174,669,883,595,683,167,165,589,362,604,243,723,53,658,591
317,780,388,747,497,542,734,943,201,481,165,371,548,767,771,907,196,659,155,513
549,491,269,699,84,850,64,847,773,233,187,911,183,134,729,527,505,275,654,361
509,829,2,461,550,357,145,151,783,174,65,379,369,412,683,682,54,470,71,384
232,185,436,398,632,502,754,433,502,643,878,622,510,776,80,421,73,792,504,713
132,312,271,502,406,539,823,781,208,465,290,601,235,289,62,259,815,484,106,747
489,438,906,750,437,944,723,911,225,434,247,882,184,790,278,534,681,327,729,93
700,816,682,543,265,170,461,511,462,851,454,321,527,504,936,830,680,401,247,221
632,624,232,279,253,939,825,5,385,700,480,879,202,159,206,195,612,729,588,683
698,534,605,369,530,746,745,711,525,789,338,332,143,321,705,380,882,200,681,501
55,512,181,758,937,782,698,888,826,658,611,288,87,197,682,279,196,716,531,724
830,877,690,259,109,146,935,690,205,477,132,230,382,569,130,404,701,852,719,880
729,389,377,642,261,468,74,377,133,206,313,634,652,156,256,641,175,291,355,319
)";

auto const [fields, ticket, nearby] = parse(input);

auto const res1 = total_error_rate(nearby, fields);
fmt::print("ticket scanning error rate is {}\n", res1);

auto const t = solve(ticket, fields, nearby);
for (auto const& [f, v] : t) {
  fmt::print("{}: {}\n", f, v);
}
auto const res2 = product_start(t, "departure");
fmt::print("solution to part 2 is {}\n", res2);

}