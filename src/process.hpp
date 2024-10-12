
//          Copyright David Lucius Severus 2024-.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <signal.h>
#include <spawn.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <concepts>
#include <exception>
#include <stdexcept>
#include <type_traits>

#include <string>
#include <vector>

namespace bbench
{

// swap out for whatever underlying container you want to use
template <typename T> using __impl_vector = std::vector<T>;
// template <typename T> using __impl_string = std::string<T>;

template <typename T>
concept is_string = requires(T t) {
  { t.c_str() } -> std::same_as<const char *>;
  { t.data() } -> std::same_as<typename T::pointer>;
  { t.size() } -> std::convertible_to<size_t>;
  { t.begin() } -> std::same_as<typename T::iterator>;
  { t.end() } -> std::same_as<typename T::iterator>;
  { t.cbegin() } -> std::same_as<typename T::const_iterator>;
  { t.cend() } -> std::same_as<typename T::const_iterator>;
};

// fork and run process at path location specified by T
template <bool W = false, is_string T, is_string... R>
int
process(const T &t, const R &...args)
{
  pid_t pid;
  int status = 0;
  posix_spawnattr_t flags;
  if ( posix_spawnattr_init(&flags) != 0 ) {
    throw std::runtime_error("micron process failed to init spawnattrs");
  }
  __impl_vector<char *> argv = { &t[0], nullptr };

  (argv.insert(argv.begin() + 1, &args[0]), ...);

  if ( ::posix_spawn(&pid, t.c_str(), NULL, &flags, &argv[0], environ) ) {
    throw std::runtime_error("micron process failed to start posix_spawn");
  }
  if constexpr ( W )
    ::waitpid(pid, &status, 0);
  return pid;
}

template <bool W = false, typename... R>
int
process(const char *t, const R *...args)
{
  pid_t pid;
  int status = 0;
  posix_spawnattr_t flags;
  if ( posix_spawnattr_init(&flags) != 0 ) {
    throw std::runtime_error("micron process failed to init spawnattrs");
  }
  __impl_vector<char *> argv = { const_cast<char *>(t), nullptr };

  (argv.insert(argv.begin() + 1, const_cast<char *>(args)), ...);

  if ( ::posix_spawn(&pid, t, NULL, &flags, &argv[0], environ) ) {
    throw std::runtime_error("micron process failed to start posix_spawn");
  }
  if constexpr ( W )
    ::waitpid(pid, &status, 0);
  return pid;
}

};
