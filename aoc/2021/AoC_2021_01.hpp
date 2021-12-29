#ifndef AOC_2021_01_HEADER_GUARD
#define AOC_2021_01_HEADER_GUARD
///////////////////////////////////////////////////////////////////////////////
// File AoC_2021_01.hpp
///////////////////////////////////////////////////////////////////////////////
#include <cstddef> // size_t

namespace aoc {

// Return the number of elements in 'measurements' which are larger than the
// previous element. Note that if 'measurements' is empty, the result is 0.
// 'Range' must model a range whose value type can be ordered.
template <typename Range>
std::size_t count_increases(Range const& measurements) noexcept;

} // namespace aoc

///////////////////////////////////////////////////////////////////////////////
// Template definitions
///////////////////////////////////////////////////////////////////////////////
#include <iterator> // next

template <typename Range>
std::size_t aoc::count_increases(Range const& measurements) noexcept {
  if (cbegin(measurements) == cend(measurements)) {
    return 0;                                                         // RETURN
  }
  // Note that we keep track of two iterators, which implies we are using
  // forward iterators; extending to input iterators would require storing a
  // copy of the read element.
  // TODO: implement an iterator which zips adjacent elements.
  std::size_t count = 0;
  for (auto prev = cbegin(measurements), cur = std::next(prev);
       cur != cend(measurements);
       prev = cur++) { // adjacent_zip | count_if(<)
    if (*prev < *cur) ++count;
  }
  return count;
}

#endif // AOC_2021_01_HEADER_GUARD