
//          Copyright David Lucius Severus 2024-.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "perf.hpp"
#include "time.hpp"
#include <array>
#include <queue>
#include <tuple>
#include <utility>

namespace bbench {

enum class time_resolution {
  seconds,
  sec,
  deciseconds,
  ds,
  milliseconds,
  ms,
  microseconds,
  us,
  nanoseconds,
  ns
};
// simple abstraction for kernel_clock/system_clock
// kernel_clock<time_userland, kernel_clock_types::hardware>
template <class C> // for now there's only two clocks,
class clock : public C {

public:
  ~clock() {}
  clock() : C() {}

  // alias to start
  inline __attribute__((always_inline)) auto begin() { return C::start_get(); }
  // alias to stop
  inline __attribute__((always_inline)) auto end() { return C::stop_get(); }
  // total time, last el
  template <time_resolution R> auto elapsed() -> double {
    if constexpr (R == time_resolution::seconds or R == time_resolution::sec) {
      return C::read();
    } else if constexpr (R == time_resolution::deciseconds or
                         R == time_resolution::ds) {
      return C::read_ds();
    } else if constexpr (R == time_resolution::milliseconds or
                         R == time_resolution::ms) {
      return C::read_ms();
    } else if constexpr (R == time_resolution::microseconds or
                         R == time_resolution::us) {
      return C::read_us();
    } else if constexpr (R == time_resolution::nanoseconds or
                         R == time_resolution::ns) {
      return C::read_ns();
    }
  }
};

template <typename C = system_clock<system_clocks::monotonic>> //
class stopwatch : public C {
  timespec start;
  std::queue<timespec> laps;

public:
  ~stopwatch() {}
  stopwatch() : C() {}

  // alias to start
  inline void begin() { start = C::start_get(); }
  // alias to stop
  inline void end() { laps.push(C::stop_get()); }
  // alias to stop
  inline void lap() { laps.push(C::stop_get()); }
  // total time, last el
  auto elapsed() -> double {
    if (laps.empty())
      return -1.0f;
    return C::read_ms(laps.back(), start);
  }

  void operator()(void) = delete;
  template <typename F, typename... Args> auto operator()(F f, Args &&...args) {
    C::start();
    f(std::forward<Args>(args)...);
    C::stop();
    return C::read();
  }
};

struct quiet {};

// simple abstraction for kernel_clock/system_clock
// kernel_clock<time_userland, kernel_clock_types::hardware>
template <class C = kernel_clock<time_userland, kernel_clock_types::hardware>,
          class Z = options::hardware,
          Z Y =
              options::hardware::total_inst> // for now there's only two clocks,
class event : public C {

public:
  ~event() {}
  event() : C(Y) { C::open(); }
  // event(void) : C(Y) {}
  event(const quiet &) : C(Y) {}
  event(const event &) = delete;
  event(event &&o) = default;
  template <typename T>
  inline __attribute__((always_inline)) void attach(T &&t) {
    C::attach(std::forward<T>(t));
  }
  inline __attribute__((always_inline)) void reopen(pid_t pid) {
        C::reopen(pid);
  }
  inline __attribute__((always_inline)) void open(void) { C::open(); }
  inline __attribute__((always_inline)) auto begin_as_leader() {
    return C::start_as_leader();
  }

  inline __attribute__((always_inline)) auto end_as_leader() {
    return C::stop_as_leader();
  }
  // alias to start
  inline __attribute__((always_inline)) void begin() { C::start(); }
  // alias to stop
  inline __attribute__((always_inline)) void end() { C::stop(); }
  // alias to stop
  inline void lap() { C::stop(); }
  // total time, last el
  inline __attribute__((always_inline)) auto retrieve() -> long long {
    return C::read();
  }
  void operator()(void) = delete;
  template <typename F, typename... Args> auto operator()(F f, Args &&...args) {
    C::start();
    f(std::forward<Args>(args)...);
    C::stop();
    return C::read();
  }
};

template <class... C> class event_group {
  std::tuple<C...> members;
  template <size_t... Ts> void __impl_begin_all(std::index_sequence<Ts...>) {
    auto call_begin = [](auto &m) { m.begin(); }; // called by reference
    (call_begin(std::get<Ts>(members)), ...);     // fold expression
    //(std::get<Ts>(members).begin(), ...);
  }
  template <size_t... Ts> void __impl_open_all(std::index_sequence<Ts...>) {
    auto call_end = [](auto &m) { m.open(); }; // called by reference
    (call_end(std::get<Ts>(members)), ...);    // fold expression
  }
  template <size_t... Ts> void __impl_end_all(std::index_sequence<Ts...>) {
    auto call_end = [](auto &m) { m.end(); }; // called by reference
    (call_end(std::get<Ts>(members)), ...);   // fold expression
  }
  template <size_t... Ts>
  void __impl_reopen_all(int pid, std::index_sequence<Ts...>) {
    auto call_reopen = [](int _pid, auto &m) {
      m.reopen(_pid);
    }; // called by reference
    (call_reopen(pid, std::get<Ts>(members)), ...); // fold expression
  }

public:
  event_group(void) : members(std::move(C{})...) {}
  event_group(C... args) : members(C(args)...) {}
  template <typename S> event_group(S arg) : members(C(arg)...) {}

  // template <std::size_t I> auto &get() { return std::get<I>(members); }
  template <typename T> auto &get() { return std::get<T>(members); }
  void open() { __impl_open_all(std::make_index_sequence<sizeof...(C)>{}); }
  void begin() { __impl_begin_all(std::make_index_sequence<sizeof...(C)>{}); }
  void end() { __impl_end_all(std::make_index_sequence<sizeof...(C)>{}); }
  void reopen(int pid) {
    __impl_reopen_all(pid, std::make_index_sequence<sizeof...(C)>{});
  }
};

// these 'typedefs' are here to prevent obnoxiously repetitive typing
using time_clock = clock<system_clock<system_clocks::realtime>>;
using time_clock_mono = clock<system_clock<system_clocks::monotonic>>;
using boot_time = clock<system_clock<system_clocks::since_boot>>;
using stopwatch_rt = stopwatch<system_clock<system_clocks::realtime>>;

// userland
using hardware_cycles =
    event<kernel_clock<time_userland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::cpu_cycles>;
using hardware_instructions =
    event<kernel_clock<time_userland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::total_inst>;
using cache_misses =
    event<kernel_clock<time_userland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::cache_misses>;
using branches =
    event<kernel_clock<time_userland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::branches>;
using branch_misses =
    event<kernel_clock<time_userland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::branch_misses>;
using total_cycles =
    event<kernel_clock<time_userland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::total_cycles>;
using u_hardware_cycles =
    event<kernel_clock<time_userland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::cpu_cycles>;
using u_hardware_instructions =
    event<kernel_clock<time_userland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::total_inst>;
using u_cache_misses =
    event<kernel_clock<time_userland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::cache_misses>;
using u_branches =
    event<kernel_clock<time_userland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::branches>;
using u_branch_misses =
    event<kernel_clock<time_userland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::branch_misses>;
using u_total_cycles =
    event<kernel_clock<time_userland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::total_cycles>;

// kernelland
using k_hardware_cycles =
    event<kernel_clock<time_kernelland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::cpu_cycles>;
using k_hardware_instructions =
    event<kernel_clock<time_kernelland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::total_inst>;
using k_cache_misses =
    event<kernel_clock<time_kernelland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::cache_misses>;
using k_branches =
    event<kernel_clock<time_kernelland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::branches>;
using k_branch_misses =
    event<kernel_clock<time_kernelland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::branch_misses>;
using k_total_cycles =
    event<kernel_clock<time_kernelland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::total_cycles>;
// everyland
using a_hardware_cycles =
    event<kernel_clock<time_everyland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::cpu_cycles>;
using a_hardware_instructions =
    event<kernel_clock<time_everyland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::total_inst>;
using a_cache_misses =
    event<kernel_clock<time_everyland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::cache_misses>;
using a_branches =
    event<kernel_clock<time_everyland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::branches>;
using a_branch_misses =
    event<kernel_clock<time_everyland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::branch_misses>;
using a_total_cycles =
    event<kernel_clock<time_everyland, kernel_clock_types::hardware>,
          options::hardware, options::hardware::total_cycles>;

using cpu_time =
    event<kernel_clock<time_userland, kernel_clock_types::software>,
          options::software, options::software::cpu_clock>;
using context_switches =
    event<kernel_clock<time_userland, kernel_clock_types::software>,
          options::software, options::software::cntx_swtch>;
using proc_migrations =
    event<kernel_clock<time_userland, kernel_clock_types::software>,
          options::software, options::software::cpu_migrations>;

using level1d = event<kernel_clock<time_userland, kernel_clock_types::cache>,
                      options::cache, options::cache::level1d>;
using level1t = event<kernel_clock<time_userland, kernel_clock_types::cache>,
                      options::cache, options::cache::level1t>;
using llcache = event<kernel_clock<time_userland, kernel_clock_types::cache>,
                      options::cache, options::cache::last_level>;
using cache_node = event<kernel_clock<time_userland, kernel_clock_types::cache>,
                         options::cache, options::cache::local_access>;
using bpu = event<kernel_clock<time_userland, kernel_clock_types::cache>,
                  options::cache, options::cache::branch>;

}; // namespace bbench
