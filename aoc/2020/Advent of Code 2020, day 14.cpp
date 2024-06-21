// Advent of Code 2020: day 14

// Godbolt link: https://godbolt.org/z/6Y74x1

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include <fmt/core.h>

bool is_digit(char const c) {
  return '0' <= c && c <= '9';
}

template<class It>
std::uint64_t to_uint(It first, It const last) {
  std::uint64_t res = 0;
  while (first != last) {
    res = res*10 + (*first++ - '0');
  }
  return res;
}

std::string_view fetch_line(std::string_view& text) {
  auto pos = text.find('\n');
  auto res = text.substr(0, pos);
  text.remove_prefix(pos+1);
  return res;
}

//////////////////////////////////////////////////////////////////////////

class Mask {
public:
  static auto constexpr N = 36;
  std::array<char, N> mask_;

  class Protocol1 {
    std::bitset<N> zero_;
    std::bitset<N> one_;
  public:
    explicit Protocol1(std::array<char, N> mask) {
      zero_.set();
      for (std::size_t i=0; i < N; ++i) { // enumerate
        auto const c = mask[N - 1 - i];
        if (c == '0') {
          zero_.flip(i);
        }
        if (c == '1') {
          one_.flip(i);
        }
      }
    }
    std::uint64_t apply(std::uint64_t const value) const {
      auto const res = (std::bitset<N>{value} & zero_) | one_;
      return res.to_ulong();
    }
  };

  class Protocol2 {
    std::vector<int> ones_;
    std::vector<int> xs_;
  public:
    explicit Protocol2(std::array<char, N> mask) {
      for (std::size_t i = 0; i < N; ++i) { // enumerate
        auto const c = mask[N - 1 - i];
        if (c == '1') {
          ones_.push_back(i);
        }
        if (c == 'x' || c == 'X') {
          xs_.push_back(i);
        }
      }
    }
    std::vector<std::uint64_t> apply(std::uint64_t const value) const {
      auto base = value;
      for (auto b : ones_) {
        base |= std::uint64_t{1} << b;
      }
      for (auto b : xs_) { // initially all zeros
        base &= ~(std::uint64_t{1} << b);
      }
      //fmt::print("base address from {} [{:036b}] is {}[{:036b}]\n", value, value, base, base);
      std::vector<std::uint64_t> res(1 << xs_.size(), base);
      for (std::size_t i = 0; i < xs_.size(); ++i) { // enumerate
        auto const block = 1 << i;
        auto const mask = std::uint64_t{1} << xs_[i];
        for (auto it = begin(res); it != end(res); it += 2*block) {
          std::for_each(it, std::next(it, block), [&](auto& v) {
            v |= mask;
          });
        }
      }
      return res;
    }
  };

  explicit Mask(std::string_view const s) {
    assert( s.size() == N );
    std::copy(begin(s), end(s), begin(mask_));
  }

  Protocol1 protocol1() const {
    return Protocol1{mask_};
  }

  Protocol2 protocol2() const {
    return Protocol2{mask_};
  }

};

Mask get_mask(std::string_view const line) {
  return Mask{line.substr(line.size() - Mask::N)};
}

struct Instruction {
  std::uint64_t address;
  std::uint64_t value;
};

Instruction get_instruction(std::string_view const line) {
  auto const f_a = std::find_if(begin(line), end(line), is_digit);
  auto const l_a = std::find_if_not(f_a, end(line), is_digit);
  auto const a = to_uint(f_a, l_a);
  auto const f_v = std::find_if(l_a, end(line), is_digit);
  auto const l_v = std::find_if_not(f_v, end(line), is_digit);
  auto const v = to_uint(f_v, l_v);
  return {a, v};
}

std::pair<Mask, std::vector<Instruction>> parse_block(std::string_view& text) {
  assert( text.substr(0, 4) == "mask" );
  auto const m = get_mask(fetch_line(text));
  std::vector<Instruction> instructions;
  while (!text.empty() && text.substr(0, 4) != "mask") {
    instructions.push_back(get_instruction(fetch_line(text)));
  }
  return {m, instructions};
}

std::vector<std::pair<Mask, std::vector<Instruction>>> parse(std::string_view text)
{
  std::vector<std::pair<Mask, std::vector<Instruction>>> res;
  while (!text.empty()) {
    res.push_back(parse_block(text));
  }
  return res;
}

auto get_values_1(std::vector<std::pair<Mask, std::vector<Instruction>>> const& code) {
  std::unordered_map<std::uint64_t, std::uint64_t> res;
  for (auto const& [m, instructions] : code) {
    auto const mask = m.protocol1();
    for (auto const& [a, i] : instructions) {
      res[a] = mask.apply(i);
      //fmt::print("mask turns {} to {} at address {}\n", i, res[a], a);
    }
  }
  return res;
}

auto get_values_2(std::vector<std::pair<Mask, std::vector<Instruction>>> const& code) {
  std::unordered_map<std::uint64_t, std::uint64_t> res;
  for (auto const& [m, instructions] : code) {
    auto const mask = m.protocol2();
    for (auto const& [a, v] : instructions) {
      for (auto addr : mask.apply(a)) {
        //fmt::print("write {} at address {}\n", v, addr);
        res[addr] = v;
      }
      
    }
  }
  return res;
}

std::uint64_t sum(std::unordered_map<std::uint64_t, std::uint64_t> const& values) {
  return std::accumulate(begin(values), end(values), std::uint64_t{0}, [](std::uint64_t res, auto const& p){
    return res + p.second;
  });
}

//////////////////////////////////////////////////////////////////////

int main() {

auto const test = R"(mask = XXXXXXXXXXXXXXXXXXXXXXXXXXXXX1XXXX0X
mem[8] = 11
mem[7] = 101
mem[8] = 0
)";

auto const test2 = R"(mask = 000000000000000000000000000000X1001X
mem[42] = 100
mask = 00000000000000000000000000000000X0XX
mem[26] = 1
)";

auto const input = R"(mask = 00111X0X10X0000XX00011111110000011X0
mem[52006] = 4929712
mem[43834] = 524429393
mem[12235] = 5761436
mem[46892] = 35146
mem[31939] = 16563655
mem[59302] = 423
mask = 0110111X1110001X1X10XX1101010011X000
mem[41405] = 37218266
mem[26084] = 35933
mem[56863] = 117475013
mem[62063] = 3066
mask = 0110110110110000X0X00X00X0X011100000
mem[17255] = 1409501
mem[3182] = 922832
mem[612] = 155268
mem[54089] = 238718351
mask = 0011001X0X11100110100011110X001100X0
mem[36228] = 813819599
mem[26530] = 199116285
mem[65466] = 823
mem[21514] = 6079436
mask = X0101000X00X011X010001X01XX110100100
mem[55733] = 400
mem[12132] = 220502402
mem[59992] = 63891518
mem[28423] = 3058
mem[60385] = 721572
mask = 101011011000X110010XX01X001100000010
mem[17718] = 5256
mem[26460] = 112823
mem[5706] = 1814611
mem[63904] = 4222
mem[23015] = 1178098
mem[46616] = 2037
mem[34774] = 229522722
mask = 00101111111X00101010011X01100100X1X0
mem[37506] = 7646
mem[25724] = 7532075
mem[46734] = 1856
mem[56304] = 6237594
mem[17886] = 51040
mem[43844] = 335
mask = 001101011110001XX00001100X00X111110X
mem[28036] = 122944
mem[62839] = 208592302
mem[61962] = 271691652
mask = 0110110X1011X000X00010010XX1101000XX
mem[2761] = 7531352
mem[49376] = 2355483
mem[4216] = 918613
mem[56927] = 5956
mem[63510] = 176827
mask = 0X1011X111100X10101000X0X100X0X1110X
mem[29120] = 2006354
mem[19556] = 3671899
mem[2168] = 5030
mem[1739] = 1419
mem[16584] = 4603711
mem[1274] = 754034
mem[27061] = 284641
mask = 0X111X010011X10011X10010001010100100
mem[38703] = 6183
mem[10881] = 50618
mem[25559] = 4348205
mem[11367] = 105492
mask = 0X1X11X1X01000X010X0000001X0001X01X0
mem[24466] = 281526
mem[2756] = 345582958
mem[27821] = 51329276
mem[3182] = 1004
mem[64312] = 14160
mem[44904] = 25504
mask = 001101X1111000111X0010110XXX00101010
mem[9655] = 8358
mem[58805] = 59610179
mem[4017] = 2556
mem[21076] = 84081646
mem[12544] = 29200337
mem[58825] = 849315134
mask = X0101001101101X0100X0110X0001001110X
mem[52416] = 80677
mem[24809] = 107220
mask = 0X10111X1X10001X101X0000110011X0X11X
mem[30145] = 59849
mem[3316] = 1661693
mem[20518] = 2429070
mask = 0XX010111010110XX01100001X1001101010
mem[10481] = 534671
mem[3232] = 88771
mem[42476] = 829903139
mem[23957] = 82713
mem[59410] = 783351894
mem[54338] = 713731093
mask = 011XX110111000X0101101011000X1001111
mem[49852] = 266900676
mem[27265] = 226
mem[28046] = 160578
mask = 0011XX110011X00X10X0X001110X1010X010
mem[48742] = 401824
mem[63122] = 28688
mem[47126] = 7096560
mem[16772] = 1939
mem[10570] = 10714
mem[61299] = 509453
mask = 101010010000111001001X1X000X11X10X01
mem[46759] = 1542447
mem[40767] = 4698135
mem[61684] = 350
mem[22031] = 270
mask = 1X100001X1011110010010100011X10000X1
mem[47812] = 224148024
mem[59070] = 4675
mem[8910] = 87677
mask = 0110X1110010001X1X100X1X01100111X000
mem[52883] = 15219771
mem[25333] = 112222
mem[3339] = 1843398
mask = 1X101X01101X00X010X001XX0101100X0X11
mem[4298] = 639594
mem[8377] = 9230
mem[10177] = 66175935
mem[10999] = 1732
mem[19417] = 920705
mask = 00XXX1111110000X101X00X10100X1100111
mem[21108] = 579440
mem[61811] = 12022501
mem[4298] = 6241794
mem[15882] = 291892238
mem[34076] = 22758
mem[44997] = 401340
mem[10203] = 4880
mask = 0010X1010X1000XX10X001X111010001X110
mem[10169] = 84875
mem[40007] = 724198522
mem[26317] = 5062
mem[59565] = 14796158
mem[35179] = 9273
mem[11775] = 425020
mem[61734] = 277758
mask = X11010X1X011110X101100X0011X00010000
mem[1616] = 5642
mem[29008] = 40378
mem[54571] = 2275046
mem[56598] = 54180
mem[44904] = 52919006
mask = 011011110010000110X010X0XX0X11X00010
mem[58510] = 826924986
mem[20287] = 2697255
mem[12002] = 58954
mem[23957] = 9816
mem[20851] = 781642
mask = 01X011011011XX00XX011100110X100X011X
mem[35399] = 170571736
mem[7307] = 174037504
mask = XX1011100X00000X10100010X10001X0X0X0
mem[59534] = 32654
mem[50813] = 982116
mem[17460] = 660730376
mask = 0X101X01101X0X001X0X00000X1011X00000
mem[2628] = 179572
mem[37874] = 64485456
mem[2757] = 21090
mem[63548] = 18421
mem[34496] = 25814
mask = X01XX111X110X00X10000001000100100101
mem[29395] = 1469411
mem[39019] = 129148834
mem[34858] = 2143
mem[38555] = 5407018
mem[64659] = 6721809
mem[15877] = 276533
mem[13477] = 1551765
mask = 0X111X11001000XX100001000X0X10011101
mem[27701] = 815118
mem[47892] = 675192690
mem[1921] = 186957
mem[41394] = 104302
mem[46219] = 391
mask = XX1111110010001X10X0011010010X1X1101
mem[18848] = 944747776
mem[65221] = 1488890
mem[8962] = 1787
mask = 01101111X0110X00XX00000110X00101X001
mem[869] = 196011
mem[59565] = 218484
mem[3695] = 30633
mem[29495] = 63295076
mem[41574] = 28810
mask = 0010100X10X00XX0X100000XX00011X00100
mem[26317] = 418
mem[55269] = 194823
mem[2788] = 107445850
mem[58602] = 40976
mem[52055] = 97759722
mask = 100X100100XX0101110100010011X1111X01
mem[33519] = 52748
mem[2628] = 247114902
mem[12920] = 118
mem[19314] = 24512
mem[17460] = 16794
mask = 00X1X0X1X0X011000100010X10111X1X11X1
mem[40330] = 64426
mem[30794] = 88457
mem[46817] = 3402
mem[48648] = 224349
mem[13744] = 16799
mem[36227] = 2196
mask = 00X011X110110X0X1101XX00101011100001
mem[50422] = 1636652
mem[46734] = 566
mem[8709] = 5494892
mem[35996] = 11312587
mem[47720] = 63678
mem[37938] = 21
mask = 001X1000000X0X1001000010001001100XX0
mem[4298] = 789383
mem[49605] = 29507
mem[4850] = 23534344
mask = 10101011101010000X001X001X0110X011X0
mem[36247] = 55966312
mem[47774] = 7827
mem[47104] = 179521
mem[10169] = 8480531
mem[64125] = 1496
mem[9860] = 102302115
mask = XX10X1110010000XXX0010001000010XX10X
mem[63122] = 2529
mem[15234] = 3390
mem[47165] = 131784350
mask = X110XX1X10100XX0101000001110001011XX
mem[42598] = 205125350
mem[45213] = 875385
mem[1630] = 374698
mask = 001X110X10100000110000001X00100X0101
mem[869] = 262
mem[1729] = 7311
mem[38532] = 37873869
mask = 0X10111X111000101XX000010110X1100111
mem[40337] = 110094841
mem[23200] = 5963831
mask = 0XX0X101XX1101X010100X11001X11X00111
mem[39850] = 194184
mem[15113] = 14352
mem[37359] = 62840981
mask = 001010X11X10X100X1000X1X0X0X0X101101
mem[14025] = 1248
mem[4446] = 60798259
mem[54198] = 747825
mem[8222] = 2843610
mem[46819] = 134827
mask = 001X10X11010110X010X01010000011X01X1
mem[4571] = 3
mem[45153] = 354688624
mem[23739] = 1747241
mem[2180] = 1501
mem[31640] = 314996015
mem[22838] = 1456
mem[9279] = 1793
mask = 0110X1X1X0110000110X00001110011X0111
mem[2293] = 860
mem[52148] = 31128529
mem[6212] = 62
mem[43300] = 57169
mask = 1000X001101X010011X100X110100X001X01
mem[2628] = 2308
mem[2849] = 57923736
mem[12286] = 287875
mem[12201] = 72
mem[34425] = 59856
mask = 0110X101101X010011010X0101100X1001X0
mem[7749] = 185738
mem[4446] = 594
mem[51626] = 351621474
mem[24035] = 739934921
mask = 1XX011011011XX001X1X0110X1110X000111
mem[54919] = 2711949
mem[2709] = 208984376
mem[2761] = 28699
mem[45547] = 34108
mem[55552] = 4945
mem[18632] = 150986875
mem[10947] = 400341
mask = 011010X1101XX1X010X1X000X100X00X0000
mem[22250] = 2516
mem[34496] = 31839
mask = 0110X1X000100X00XXX0X00111100X001000
mem[11889] = 506
mem[34264] = 2791
mem[35884] = 3473536
mask = 1X1XX11110X0110011X1001X1001XX01000X
mem[33997] = 1179307
mem[50647] = 669335
mem[61780] = 9982626
mem[13748] = 1981
mem[29233] = 950
mask = 0110111111100X1010X0X00X100000110110
mem[22910] = 1180108
mem[5562] = 21631
mem[1222] = 504800403
mask = 001X100X00X0011X01001010010010000001
mem[24198] = 119566
mem[35001] = 16283323
mem[59436] = 119739774
mem[62948] = 64713
mem[2849] = 255
mem[23156] = 1485
mask = 001X111X0X0X0000X000100100000XX01X1X
mem[35160] = 1583418
mem[43805] = 771
mem[8313] = 216593668
mem[43300] = 4138437
mem[26057] = 513262
mem[3182] = 46056
mem[10789] = 8045
mask = 001X101X1110010XX10010XX011000X00011
mem[46633] = 4097498
mem[41631] = 626
mem[35179] = 922
mem[63510] = 6031
mem[18031] = 6879
mask = 10X0X001100X111011000X1X0010X11XX000
mem[28323] = 16663
mem[55733] = 11506187
mask = 0X1011110X100010X010011X10001010X11X
mem[6698] = 11724271
mem[55597] = 10930
mem[11310] = 56566
mem[4411] = 489
mem[7361] = 431285
mask = 1X10100110110100100100X00000XX010100
mem[16972] = 963
mem[34942] = 1374714
mem[33641] = 34676
mem[53248] = 1039
mem[50381] = 239
mem[41003] = 271150
mem[59255] = 1046488
mask = 10111X01110XX11001001001X0001XX01X11
mem[22876] = 1261577
mem[23514] = 83628146
mem[46492] = 16023174
mem[42168] = 22907486
mem[56233] = 1208
mask = 0X1X111100XX00001000000110XX001X01X0
mem[41579] = 106791107
mem[37293] = 76855011
mem[23712] = 3066
mem[53928] = 206585650
mem[38356] = 58860
mask = 0010X001101X0100X001101X1100100111X0
mem[40721] = 1522
mem[35179] = 41746958
mem[19534] = 58840945
mem[32324] = 147835050
mem[48430] = 245
mask = 00XXX1110001000X00000011101001X01011
mem[47713] = 371239
mem[12557] = 896
mem[23039] = 728213024
mem[39609] = 19414
mem[44321] = 11334054
mem[36247] = 398030
mask = 00101101001X0X0XX0X0001X0X01X110011X
mem[46734] = 676353
mem[60374] = 267786
mem[38508] = 446859055
mem[64904] = 14216866
mem[31959] = 813920705
mem[27255] = 784
mem[36553] = 255261
mask = 0011X1X1X01X0X111000101100000X001000
mem[46633] = 1221
mem[33954] = 680347
mem[21297] = 35894
mem[41405] = 6184
mask = 001X111111100X001XX000X01001XXX0X010
mem[39635] = 477960
mem[39405] = 161170
mem[36252] = 453585
mem[55397] = 2058746
mem[33107] = 2663
mask = 101X100X1001XX00010010001X1X1X010000
mem[35277] = 200785
mem[19680] = 1119384
mem[46603] = 2780262
mask = 111X01110X1000001X0010011000011000X0
mem[30145] = 7700
mem[61472] = 979688
mem[14460] = 21055
mem[16944] = 313655989
mask = 0110111X00X0000X10X00X0X010X000X11X0
mem[2168] = 26991523
mem[5264] = 980832681
mem[36646] = 813667866
mem[48602] = 1783
mask = 0X101000X010001001001110110XX11X111X
mem[23627] = 807818
mem[61811] = 23479
mem[64] = 219255
mem[37128] = 1553397
mem[14691] = 67418150
mask = 1X1010011000110001X01XX0010X100110X0
mem[27135] = 64061097
mem[19834] = 2824449
mem[50521] = 3939
mem[58503] = 8393
mem[28423] = 43394
mask = XX1X1111X01011X011X100010X1100100XX1
mem[23429] = 130936
mem[48602] = 7532488
mem[6436] = 310907
mem[24886] = 27122161
mem[27957] = 50861195
mem[54279] = 180122731
mask = X0X01X1011100X01110XX0X1X11101X00010
mem[14025] = 691
mem[2825] = 249
mem[3925] = 3303251
mask = 00XX111X111000101X10000X10X011X11110
mem[17498] = 634241
mem[15524] = 20855180
mask = 0X1X1101X01X0100110100XX1X10XX100100
mem[30145] = 625
mem[9797] = 1359
mem[12286] = 1127042
mask = 101X100100X011101X00X100X1X1111X1X11
mem[10972] = 42023592
mem[61376] = 3427840
mem[27255] = 6685615
mem[13520] = 10945
mem[55597] = 807895898
mem[60531] = 5121
mask = 0X10100X101X0X0X1X000000X01011100100
mem[10352] = 861247
mem[2656] = 3492337
mem[55397] = 2392591
mem[29495] = 98579
mem[2757] = 3455299
mem[11236] = 4020
mask = 011011100X1XXX00100XX1001X00X0000100
mem[45068] = 386
mem[8960] = 151
mem[17784] = 108694472
mem[26289] = 4159
mem[3665] = 674
mem[54896] = 131398121
mask = 00101X11X110000X1010X0100100X1010110
mem[34512] = 162
mem[36639] = 15024013
mem[34942] = 80023258
mem[24555] = 418619
mem[50642] = 27165886
mem[345] = 110421710
mask = X0101101101100X010000X00111011X00101
mem[1153] = 62642
mem[56846] = 5129
mem[20775] = 4212056
mem[19328] = 216506
mem[29495] = 8683991
mask = 0010111X111000X010100X00X1XX11X0X11X
mem[3771] = 4382805
mem[57881] = 16921
mem[63654] = 6152
mem[23552] = 5702333
mem[6083] = 220540005
mask = X01X1X0110001110010X011XX100X001X110
mem[59847] = 564882739
mem[51385] = 221685661
mem[61811] = 204871661
mem[56244] = 31583475
mem[6980] = 1527
mem[26289] = 15857
mask = X010100XX000X110X100110X0XX1111X0101
mem[4571] = 54901211
mem[57199] = 45702
mem[42452] = 43929335
mem[19680] = 48875
mem[10352] = 122542
mem[34374] = 2882
mem[61522] = 3971
mask = 01X011X10010001010X00110X1000X11X001
mem[25036] = 2952
mem[61299] = 2085542
mem[19117] = 186
mem[53853] = 187
mask = 0011X111X0100X1X10X0110XX00010111110
mem[57399] = 1006550549
mem[63028] = 186530
mem[48554] = 22962
mem[56976] = 47700
mem[6575] = 131534365
mem[52761] = 3853817
mask = XXX0XX0110110100X1010000X010010X0101
mem[11889] = 29237617
mem[17718] = 1630
mem[4636] = 229985
mem[52883] = 84375864
mask = X01010001011X1X0X10XX1010XX101100101
mem[49740] = 7868911
mem[37506] = 4002
mem[32663] = 869910098
mem[8572] = 125350
mem[8342] = 61042
mask = X0101X0100100101X00000101X00XX1100X0
mem[59565] = 1105
mem[27715] = 1745
mem[59206] = 330363729
mask = 01101111X0110000100000X010X01010X10X
mem[44171] = 899
mem[47812] = 21673008
mem[27608] = 13645404
mem[32326] = 491141
mem[63638] = 694
mem[53420] = 426003787
mem[51557] = 3275141
mask = X1101X11001X001010000010XXXX010XX000
mem[54777] = 804
mem[34172] = 6830067
mem[49202] = 63909
mem[13477] = 10986
mask = 101XX0011XXX11X00100101001001X011011
mem[16846] = 27316344
mem[50094] = 16967873
mem[61780] = 900178
mem[15882] = 23418
mem[22876] = 3337
mem[47284] = 230107
mask = 0111110X1X10010X1101001X1001011X0001
mem[12201] = 6745
mem[25284] = 182
mem[44850] = 1569
mem[47949] = 411159
mem[30793] = 845530
mem[47029] = 58274
mask = X11011X11010X100110100001111X0000101
mem[3182] = 10552
mem[48303] = 111746
mem[14883] = 15066
mem[4517] = 28345405
mem[25038] = 178092778
mask = 1X001001X01101XX11X10X01001100100001
mem[22910] = 1977
mem[15113] = 102588
mem[62218] = 1881
mask = X01X101110101XX00X0010100010000X111X
mem[33012] = 795
mem[50671] = 14579873
mem[55556] = 169319
mem[10502] = 3909
mem[36753] = 31795
mem[41712] = 8377123
mem[63904] = 4717
mask = 00110111XXX00X1110000001010XX0X0X010
mem[2564] = 131432438
mem[28323] = 3416844
mem[49852] = 3072
mem[42274] = 404
mask = 011011110X110000000001001000XX1001X1
mem[47316] = 107
mem[12286] = 12644
mem[37518] = 92320
mem[54777] = 2007
mem[54708] = 8481064
mem[48684] = 2372
mask = 01X0011000100100011XX0001110XX001100
mem[5220] = 1685
mem[11442] = 5413142
mask = 01001X0101110X0X10100001X0X0X1XX101X
mem[57860] = 3766
mem[699] = 147751906
mem[16648] = 30301
)";

auto const code = parse(input);

auto const res1 = sum(get_values_1(code));
fmt::print("res of part 1 is {}\n", res1);

auto const res2 = sum(get_values_2(code));
fmt::print("res of part 2 is {}\n", res2);

}