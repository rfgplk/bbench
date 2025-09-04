
//          Copyright David Lucius Severus 2024-.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "../src/bench.hpp"
#include "../src/cargs.hpp"
#include "../src/print.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

int main(int argc, char **argv) {

  if (argc < 2)
    return -1;
  size_t n_times = 0;

  cargs::parser arg(argc, argv);
  auto &&flags = arg.list();
  auto &&vals = arg.data();
  auto &&paths = arg.operands();
  if (flags.size() != vals.size())
    return -1;
  try {
    for (int k = 0; k < flags.size(); ++k)
      if (flags.at(k) == "n")
        n_times = std::stoll(vals.at(k).c_str());
  } catch (...) {
    bbench::print("Invalid command line parameters");
    return -1;
  }
  std::vector<bbench::benchmark_t> benches;
  std::vector<std::vector<bbench::benchmark_t>> bench_results;
  for (auto &n : paths) {
    for (size_t t = 0; t < n_times; ++t)
      benches.emplace_back(bbench::benchmark_bin(n.c_str()));
    if (benches.size())
      bench_results.push_back(benches);
    benches.clear();
  }
  for (auto &m : bench_results) {
    m.front().time =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.time;
                                  }) /
                  m.size());
    m.front().cycles =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.cycles;
                                  }) /
                  m.size());

    m.front().instructions =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.instructions;
                                  }) /
                  m.size());

    m.front().total_branches =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.total_branches;
                                  }) /
                  m.size());

    m.front().branch_misses =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.branch_misses;
                                  }) /
                  m.size());

    m.front().total_cycles =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.total_cycles;
                                  }) /
                  m.size());

    m.front().cpu_time =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.cpu_time;
                                  }) /
                  m.size());

    m.front().context_switches =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.context_switches;
                                  }) /
                  m.size());

    m.front().migrations =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.migrations;
                                  }) /
                  m.size());

    m.front().cache_misses =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.cache_misses;
                                  }) /
                  m.size());

    m.front().l1_cache =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.l1_cache;
                                  }) /
                  m.size());

    m.front().l1t_cache =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.l1t_cache;
                                  }) /
                  m.size());

    m.front().access =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.access;
                                  }) /
                  m.size());

    m.front().bpu =
        std::ceil(std::accumulate(m.begin(), m.end(), 0,
                                  [](size_t sum, const bbench::benchmark_t &v) {
                                    return sum + v.bpu;
                                  }) /
                  m.size());
  }

  std::sort(bench_results.begin(), bench_results.end(),
            [](const std::vector<bbench::benchmark_t> &a,
               const std::vector<bbench::benchmark_t> &b) {
              return a.front().time < b.front().time;
            });
  bool _f = false;
  bbench::print("Sorting according to fastest time.");
  for (const auto &res : bench_results) {
    const auto &b = res.front();
    bbench::print("\033[34m", "Attempted ", n_times, " runs\033[0m");
    if (!_f) {
      bbench::print("\033[34m", "Results for: ", "\033[0m", b.name,
                    " <- winner");
      _f = true;
    } else
      bbench::print("\033[34m", "Results for: ", "\033[0m", b.name);
    bbench::print("\033[34m", "Total time elapsed:   ", "\033[0m", b.time,
                  " ms");
    bbench::print("\033[34m", "Cycles Spent:         ", "\033[0m", b.cycles);
    bbench::print("\033[34m", "Total Instructions:   ", "\033[0m",
                  b.instructions);
    bbench::print("\033[34m", "Total Branches:       ", "\033[0m",
                  b.total_branches);
    bbench::print("\033[34m", "Branch Misses:        ", "\033[0m",
                  b.branch_misses);
    bbench::print("\033[34m", "Total CPU Cycles:     ", "\033[0m",
                  b.total_cycles);
    bbench::print("\033[34m", "Total CPU Time Spent: ", "\033[0m", b.cpu_time);
    bbench::print("\033[34m", "Context Switches:     ", "\033[0m",
                  b.context_switches);
    bbench::print("\033[34m", "Core Migrations:      ", "\033[0m",
                  b.migrations);
    bbench::print("\033[34m", "Cache Misses:         ", "\033[0m",
                  b.cache_misses);
    bbench::print("\033[34m", "L1 Cache:             ", "\033[0m", b.l1_cache);
    bbench::print("\033[34m", "L1T Cache:            ", "\033[0m", b.l1t_cache);
    bbench::print("\033[34m", "Last Level Cache:     ", "\033[0m", b.ll_cache);
    bbench::print("\033[34m", "Cache Accesses:       ", "\033[0m", b.access);
    bbench::print("\033[34m", "Branch Predictions:   ", "\033[0m", b.bpu);
  }
  return 0;
}
