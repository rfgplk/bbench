
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
  auto bmx = bbench::benchmark<bbench::time_resolution::ns>(adder_arrx, 1024);
  std::cout << per_cycle(bmx) << std::endl;
  std::cout << per_instruction(bmx) << std::endl;
  std::cout << miss_percent(bmx) << std::endl;
  std::cout << cycles_per_instruction(bmx) << std::endl;
  return 0;
  std::cout << "Cycles of whoami: " << bbench::cpu_bench_bin("/bin/whoami") << std::endl;
  std::cout << "Nanoseconds for whoami: " << bbench::bench_bin<time_resolution::ns>("/bin/whoami") << std::endl;
  auto bm = bbench::benchmark(adder, 10);
  std::cout << "Branches: " << bm.total_branches << std::endl;
  std::cout << bbench::bench<time_resolution::ms>(adder, 5) << std::endl;
  std::cout << "CPU cycles: " << bbench::cpu_bench<hardware_cycles>(adder_arr, 5) << std::endl;
  std::cout << "CPU Instructions: " << bbench::cpu_bench<hardware_instructions>(adder_arr, 5) << std::endl;
  std::cout << "Cache misses: " << bbench::cpu_bench<cache_misses>(adder_arr, 5) << std::endl;
  std::cout << "Branches: " << bbench::cpu_bench<branches>(adder_arr, 5) << std::endl;
  std::cout << "Branch misses: " << bbench::cpu_bench<branch_misses>(adder_arr, 5) << std::endl;
  std::cout << "Total cycles: " << bbench::cpu_bench<total_cycles>(adder_arr, 5) << std::endl;
  std::cout << "CPU Time: " << bbench::cpu_bench<cpu_time>(adder_arr, 5) << std::endl;

  auto t = bbench::bench_repeat<10>(adder, 5);
  for ( const auto &n : t )
    std::cout << n << std::endl;
  // std::cout << bbench::chrono::seconds(cl.read()) << std::endl;
  // bbench::process("/bin/whoami");
  // bbench::clock c;
  // std::cout << c(adder, (size_t)500) << std::endl;
  /*std::array<size_t, 256> arr;
  kernel_clock<time_userland, kernel_clock_types::hardware> c(options::hardware::total_inst);
  c.open();
  c.start();
  size_t i = 0;
  for(auto& n : arr)
    n = i++;
  c.stop();
  std::cout << c.read() << std::endl;
  */
  return 0;
}
