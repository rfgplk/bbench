
//          Copyright David Lucius Severus 2024-.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>

#include "clock.hpp"
#include "perf.hpp"

namespace bbench {
// Util Functions
// moved here for convenience
// main container for storing all benchmark data from benchmark()
struct benchmark_t {
  std::string name;
  double time; // heh
  long long cycles;
  long long instructions;
  long long cache_misses;
  long long total_branches;
  long long branch_misses;
  long long total_cycles;
  long long cpu_time;
  long long context_switches;
  long long migrations;

  long long l1_cache;
  long long l1t_cache;
  long long ll_cache;
  long long access;
  long long bpu;
};

auto per_op(double x, long long a) { return x / static_cast<double>(a); }

auto per_cycle(const benchmark_t &b) {
  return static_cast<double>((double)b.cycles / static_cast<double>(b.time));
}

auto per_instruction(const benchmark_t &b) {
  return static_cast<double>((double)b.instructions /
                             static_cast<double>(b.time));
}

auto miss_percent(const benchmark_t &b) {
  return static_cast<double>((double)b.branch_misses /
                             (double)b.total_branches);
}

auto cycles_per_instruction(const benchmark_t &b) {
  return static_cast<double>((double)b.cycles / (double)b.instructions);
}

template <typename T> void perf_init_leader(T &&leader) {
  hardware_cycles a;
  a.start_as_leader();
}

}; // namespace bbench
