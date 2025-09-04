
//          Copyright David Lucius Severus 2024-.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "../src/bench.hpp"
#include "../src/clock.hpp"
#include "../src/process.hpp"

#include <iostream>

using namespace bbench;

#include <vector>

#include <array>

void
adder_arrx(size_t s)
{
  volatile int x = 0;
  std::array<size_t, 4096> arr;
  for ( size_t i = 0; i < s; i++ ) {
    for ( auto &n : arr ) {
      n = i + x;
      x++;
    }
    x++;
  }
}

void
adder_arr(size_t s)
{
  volatile int x = 0;
  std::array<size_t, 4096> arr;
  for ( auto &n : arr ) {
    x++;
    n = s++;
  }
}

void
adder(size_t s)
{
  std::vector<size_t> arr;
  arr.resize(5e7, 11);
  for ( auto &n : arr )
    n = s++;
}

// bench() - time bench, in ms
//

int
main(void)
{
  auto bmx = bbench::benchmark_batch<bbench::time_resolution::ns>(adder_arrx, 1024);
  std::cout << per_cycle(bmx) << std::endl;
  std::cout << per_instruction(bmx) << std::endl;
  std::cout << miss_percent(bmx) << std::endl;
  std::cout << cycles_per_instruction(bmx) << std::endl;
  return 0;
}
