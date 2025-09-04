//          Copyright David Lucius Severus 2024-.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <iostream>

namespace bbench {

template <typename T>
concept string = requires(T t) {
  { t.c_str() } -> std::same_as<const char *>;
  { t.empty() } -> std::convertible_to<bool>;
  { t.size() } -> std::convertible_to<size_t>;
  { t.begin() } -> std::same_as<typename T::iterator>;
  { t.end() } -> std::same_as<typename T::iterator>;
  { t.cbegin() } -> std::same_as<typename T::const_iterator>;
  { t.cend() } -> std::same_as<typename T::const_iterator>;
};

template <typename... T> void print(const T &...str) {
  ((std::cout << str), ...);
  std::cout << std::endl;
}

}; // namespace bbench
