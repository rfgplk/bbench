
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
  std::cout << bbench::bench_bin<bbench::time_resolution::us>(argv[1]) << std::endl;
  return 0;
}
