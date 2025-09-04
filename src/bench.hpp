
//          Copyright David Lucius Severus 2024-.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <chrono>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

#include "clock.hpp"
#include "funcs.hpp"
#include "process.hpp"

namespace bbench {

template <time_resolution R = time_resolution::us, typename F, typename... Args>
inline benchmark_t benchmark_batch(F func, Args &&...args) {
  bbench::time_clock cl;
  bbench::event_group<hardware_cycles, hardware_instructions, cache_misses,
                      branches, branch_misses, total_cycles, cpu_time,
                      context_switches, proc_migrations, level1d, level1t,
                      llcache, cache_node, bpu>
      gr {quiet{}};

  // fixed it
  gr.open();
  cl.begin();
  gr.begin();
  func(args...);
  cl.end();
  gr.end();
  return {.name = "",
          .time = cl.elapsed<R>(),
          .cycles = (long long int)gr.get<hardware_cycles>().retrieve(),
          .instructions = (long long int)gr.get<hardware_instructions>().retrieve(),
          .cache_misses = (long long int)gr.get<cache_misses>().retrieve(),
          .total_branches = (long long int)gr.get<branches>().retrieve(),
          .branch_misses = (long long int)gr.get<branch_misses>().retrieve(),
          .total_cycles = (long long int)gr.get<total_cycles>().retrieve(),
          .cpu_time = (long long int)gr.get<cpu_time>().retrieve(),
          .context_switches = (long long int)gr.get<context_switches>().retrieve(),
          .migrations = (long long int)gr.get<proc_migrations>().retrieve(),
          .l1_cache = (long long int)gr.get<level1d>().retrieve(),
          .l1t_cache = (long long int)gr.get<level1t>().retrieve(),
          .ll_cache = (long long int)gr.get<llcache>().retrieve(),
          .access = (long long int)gr.get<cache_node>().retrieve(),
          .bpu = (long long int)gr.get<bpu>().retrieve()};
};

template <time_resolution R = time_resolution::us, typename F, typename... Args>
inline benchmark_t benchmark(F func, Args &&...args) {
  bbench::time_clock cl;
  hardware_cycles a;
  hardware_instructions b;
  cache_misses c;
  branches d;
  branch_misses e;
  total_cycles f;
  cpu_time g;
  context_switches h;
  proc_migrations i;
  level1d j;
  level1t k;
  llcache l;
  cache_node m;
  bpu n;

  // NOTE: there's really no other way to do this, eventually I'll merge all of
  // them into a single call, but regardless the syscall for perf_event accents
  // one type of event at once without grouping. the overhead is low enough per
  // call that it doesn't really matter
  c.begin();
  d.begin();
  e.begin();
  f.begin();
  g.begin();
  h.begin();
  i.begin();
  j.begin();
  k.begin();
  l.begin();
  m.begin();
  n.begin();
  cl.begin();
  a.begin();
  b.begin();
  func(std::forward<Args>(args)...);
  cl.end();
  a.end();
  b.end();
  c.end();
  d.end();
  e.end();
  f.end();
  g.end();
  h.end();
  i.end();
  j.end();
  k.end();
  l.end();
  m.end();
  n.end();
  return {.name = "",
          .time = cl.elapsed<R>(),
          .cycles = a.retrieve(),
          .instructions = b.retrieve(),
          .cache_misses = c.retrieve(),
          .total_branches = d.retrieve(),
          .branch_misses = e.retrieve(),
          .total_cycles = f.retrieve(),
          .cpu_time = g.retrieve(),
          .context_switches = h.retrieve(),
          .migrations = i.retrieve(),
          .l1_cache = j.retrieve(),
          .l1t_cache = k.retrieve(),
          .ll_cache = l.retrieve(),
          .access = m.retrieve(),
          .bpu = n.retrieve()};
};

// Total benchmark, test everything
template <time_resolution R = time_resolution::us, typename F, typename... Args>
inline benchmark_t benchmark(const std::string &_name, F func, Args &&...args) {
  bbench::time_clock cl;
  hardware_cycles a;
  hardware_instructions b;
  cache_misses c;
  branches d;
  branch_misses e;
  total_cycles f;
  cpu_time g;
  context_switches h;
  proc_migrations i;
  level1d j;
  level1t k;
  llcache l;
  cache_node m;
  bpu n;

  // NOTE: there's really no other way to do this, eventually I'll merge all of
  // them into a single call, but regardless the syscall for perf_event accents
  // one type of event at once without grouping. the overhead is low enough per
  // call that it doesn't really matter
  c.begin();
  d.begin();
  e.begin();
  f.begin();
  g.begin();
  h.begin();
  i.begin();
  j.begin();
  k.begin();
  l.begin();
  m.begin();
  n.begin();
  cl.begin();
  a.begin();
  b.begin();
  func(args...);
  cl.end();
  a.end();
  b.end();
  c.end();
  d.end();
  e.end();
  f.end();
  g.end();
  h.end();
  i.end();
  j.end();
  k.end();
  l.end();
  m.end();
  n.end();
  return {.name = _name,
          .time = cl.elapsed<R>(),
          .cycles = a.retrieve(),
          .instructions = b.retrieve(),
          .cache_misses = c.retrieve(),
          .total_branches = d.retrieve(),
          .branch_misses = e.retrieve(),
          .total_cycles = f.retrieve(),
          .cpu_time = g.retrieve(),
          .context_switches = h.retrieve(),
          .migrations = i.retrieve(),
          .l1_cache = j.retrieve(),
          .l1t_cache = k.retrieve(),
          .ll_cache = l.retrieve(),
          .access = m.retrieve(),
          .bpu = n.retrieve()};
};

template <class C = hardware_cycles, typename... A>
inline benchmark_t benchmark_bin(const char *s, A... args) {
  bbench::time_clock cl;
  hardware_cycles a;
  hardware_instructions b;
  cache_misses c;
  branches d;
  branch_misses e;
  total_cycles f;
  cpu_time g;
  context_switches h;
  proc_migrations i;
  level1d j;
  level1t k;
  llcache l;
  cache_node m;
  bpu n;
  int pid = process<false>(s, args...);
  a.reopen(pid);
  b.reopen(pid);
  c.reopen(pid);
  d.reopen(pid);
  e.reopen(pid);
  f.reopen(pid);
  g.reopen(pid);
  h.reopen(pid);
  i.reopen(pid);
  j.reopen(pid);
  k.reopen(pid);
  l.reopen(pid);
  m.reopen(pid);
  n.reopen(pid);

  cl.begin();
  a.begin();
  b.begin();
  c.begin();
  d.begin();
  e.begin();
  f.begin();
  g.begin();
  h.begin();
  i.begin();
  j.begin();
  k.begin();
  l.begin();
  m.begin();
  n.begin();

  ::waitpid(pid, nullptr, 0);
  cl.end();
  a.end();
  b.end();
  c.end();
  d.end();
  e.end();
  f.end();
  g.end();
  h.end();
  i.end();
  j.end();
  k.end();
  l.end();
  m.end();
  n.end();
  return {.name = s,
          .time = cl.elapsed<time_resolution::us>(),
          .cycles = a.retrieve(),
          .instructions = b.retrieve(),
          .cache_misses = c.retrieve(),
          .total_branches = d.retrieve(),
          .branch_misses = e.retrieve(),
          .total_cycles = f.retrieve(),
          .cpu_time = g.retrieve(),
          .context_switches = h.retrieve(),
          .migrations = i.retrieve(),
          .l1_cache = j.retrieve(),
          .l1t_cache = k.retrieve(),
          .ll_cache = l.retrieve(),
          .access = m.retrieve(),
          .bpu = n.retrieve()};
};

template <class C = hardware_cycles, is_string T, is_string... A>
inline benchmark_t benchmark_bin(const T &s, A... args) {
  bbench::time_clock cl;
  hardware_cycles a;
  hardware_instructions b;
  cache_misses c;
  branches d;
  branch_misses e;
  total_cycles f;
  cpu_time g;
  context_switches h;
  proc_migrations i;
  level1d j;
  level1t k;
  llcache l;
  cache_node m;
  bpu n;
  int pid = process<false>(s, args...);
  a.reopen(pid);
  b.reopen(pid);
  c.reopen(pid);
  d.reopen(pid);
  e.reopen(pid);
  f.reopen(pid);
  g.reopen(pid);
  h.reopen(pid);
  i.reopen(pid);
  j.reopen(pid);
  k.reopen(pid);
  l.reopen(pid);
  m.reopen(pid);
  n.reopen(pid);

  cl.begin();
  a.begin();
  b.begin();
  c.begin();
  d.begin();
  e.begin();
  f.begin();
  g.begin();
  h.begin();
  i.begin();
  j.begin();
  k.begin();
  l.begin();
  m.begin();
  n.begin();

  ::waitpid(pid, nullptr, 0);
  cl.end();
  a.end();
  b.end();
  c.end();
  d.end();
  e.end();
  f.end();
  g.end();
  h.end();
  i.end();
  j.end();
  k.end();
  l.end();
  m.end();
  n.end();
  return {.name = s,
          .time = cl.elapsed<time_resolution::us>(),
          .cycles = a.retrieve(),
          .instructions = b.retrieve(),
          .cache_misses = c.retrieve(),
          .total_branches = d.retrieve(),
          .branch_misses = e.retrieve(),
          .total_cycles = f.retrieve(),
          .cpu_time = g.retrieve(),
          .context_switches = h.retrieve(),
          .migrations = i.retrieve(),
          .l1_cache = j.retrieve(),
          .l1t_cache = k.retrieve(),
          .ll_cache = l.retrieve(),
          .access = m.retrieve(),
          .bpu = n.retrieve()};
};

// Performance event based bench function
// queries the hardware counters
template <class C = hardware_cycles, typename F, typename... Args>
inline long long cpu_bench(F func, Args &&...args) {
  C cl;
  cl.begin();
  func(args...);
  cl.end();
  return cl.retrieve();
};

// same as above but bench a binary
template <class C = hardware_cycles, is_string T, is_string... A>
inline long long cpu_bench_bin(const T &s, A... a) {
  // there's a little bit of lost time here, BUT most of that will technically
  // be spent in _start and main we need to recall perf_event and start timing
  // after the process has already forked reopen + begin together should run up
  // right to before main() gets called, or a little bit later NOTE: i can get
  // better resolution by halting the forked process and then resuming right
  // before waitpid but it's pointless (within the margin of error)
  C cl;
  int pid = process<false>(s, a...);
  cl.reopen(pid);
  cl.begin();
  ::waitpid(pid, nullptr, 0);
  cl.end();
  return cl.retrieve();
};

// same as above but bench a binary
template <class C = hardware_cycles, typename... A>
inline long long cpu_bench_bin(const char *s, A... a) {
  C cl;
  int pid = process<false>(s, a...);
  cl.reopen(pid);
  cl.begin();
  ::waitpid(pid, nullptr, 0);
  cl.end();
  return cl.retrieve();
};

// Simple regular time bench
// count how long it takes for the given function to complete
// time_resolution controls the type of the output time
template <time_resolution R = time_resolution::milliseconds, typename F,
          typename... Args>
inline double bench(F func, Args &&...args) {
  bbench::time_clock cl;
  cl.begin();
  func(args...);
  cl.end();
  return cl.elapsed<R>();
};

// same as above but bench a binary
template <time_resolution R = time_resolution::milliseconds, is_string T,
          is_string... A>
inline double bench_bin(const T &s, A... a) {
  bbench::time_clock_mono cl;
  cl.begin();
  process<true>(s, a...);
  cl.end();
  return cl.elapsed<R>();
};

// same as above but bench a binary
template <time_resolution R = time_resolution::milliseconds, typename... A>
inline double bench_bin(const char *s, A... a) {
  bbench::time_clock_mono cl;
  cl.begin();
  process<true>(s, a...);
  cl.end();
  return cl.elapsed<R>();
};

// Simple regular time bench for multiple functions
template <time_resolution R = time_resolution::milliseconds, typename... F>
inline std::vector<double> bench(F... funcs) {
  std::vector<double> results;
  results.reserve(sizeof...(F));
  bbench::time_clock cl;
  auto call = [&](auto func) {
    cl.begin();
    func();
    cl.end();
    results.push_back(cl.elapsed<R>());
  };
  (call(funcs), ...);
  return results;
};

// time N functions per func
template <size_t N, time_resolution R = time_resolution::milliseconds,
          typename F, typename... Args>
inline std::vector<double> bench_repeat(F func, Args... args) {
  std::vector<double> results;
  results.reserve(N);
  bbench::time_clock cl;
  auto call = [&]() {
    cl.begin();
    func(args...);
    cl.end();
    results.push_back(cl.elapsed<R>());
  };
  for (size_t i = 0; i < N; i++)
    call();
  return results;
};

/*
// time N functions per func
template <typename... F>
inline std::vector<double>
lap(F... funcs)
{
  auto clock = create_started_clock();
  auto call = [&](auto func) {
    func();
    clock->time();
  };
  (call(funcs), ...);
  std::vector<std::chrono::duration<double>> times;
  for ( unsigned long long i = 0; i < clock->length(); i++ )
    times.push_back(
        std::chrono::duration_cast<std::chrono::duration<double>>(clock->checkpoints[i]
- clock->start_time)); return times;
};*/
}; // namespace bbench
