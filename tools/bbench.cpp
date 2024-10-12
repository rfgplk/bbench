
//          Copyright David Lucius Severus 2024-.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "../src/bench.hpp"
#include <iostream>

int
main(int argc, char **argv)
{
  if ( argc < 2 )
    return -1;
  auto b = bbench::benchmark_bin(argv[1]);
  std::cout << "Total time elapsed:   " << b.time << std::endl;
  std::cout << "Cycles Spent:         " << b.cycles << std::endl;
  std::cout << "Total Instructions:   " << b.instructions << std::endl;
  std::cout << "Total Branches:       " << b.total_branches << std::endl;
  std::cout << "Branch Misses:        " << b.branch_misses << std::endl;
  std::cout << "Total CPU Cycles:     " << b.total_cycles << std::endl;
  std::cout << "Total CPU Time Spent: " << b.cpu_time << std::endl;
  std::cout << "Context Switches:     " << b.context_switches << std::endl;
  std::cout << "Core Migrations:      " << b.migrations << std::endl;
  std::cout << "Cache Misses:         " << b.cache_misses << std::endl;
  std::cout << "L1 Cache:             " << b.l1_cache << std::endl;
  std::cout << "L1T Cache:            " << b.l1t_cache << std::endl;
  std::cout << "Last Level Cache:     " << b.ll_cache << std::endl;
  std::cout << "Cache Accesses:       " << b.access << std::endl;
  std::cout << "Branch Predictions:   " << b.bpu << std::endl;
  return 0;
}
