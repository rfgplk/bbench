
//          Copyright David Lucius Severus 2024-.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>
#include <time.h>

namespace bbench
{
// a reasonable timing library, name inherited from the STL
// NOTE: unlike STL chrono, this library isn't meant to be used as an absolute reflection of "real world" time, but
// rather cpu specific relative time to the binary executing this code, or the machine. as such most STL-like methods
// aren't provided. likewise the API is entirely different.
namespace chrono
{
template <uintmax_t N, uintmax_t D = 1> struct ratio {
  static constexpr uintmax_t num = N;
  static constexpr uintmax_t denom = D;

protected:
  static constexpr uintmax_t
  gcd(uintmax_t a, uintmax_t b)
  {
    return b == 0 ? a : gcd(b, a % b);
  }
};

typedef ratio<static_cast<uintmax_t>(1e18), 1> exa;
typedef ratio<static_cast<uintmax_t>(1e15), 1> peta;
typedef ratio<static_cast<uintmax_t>(1e12), 1> tera;
typedef ratio<static_cast<uintmax_t>(1e9), 1> giga;
typedef ratio<static_cast<uintmax_t>(1e6), 1> mega;
typedef ratio<static_cast<uintmax_t>(1e3), 1> kilo;
typedef ratio<static_cast<uintmax_t>(1e2), 1> hecto;
typedef ratio<static_cast<uintmax_t>(1e1), 1> deca;
typedef ratio<1, 1> base_ratio;
typedef ratio<1, static_cast<uintmax_t>(1e1)> deci;
typedef ratio<1, static_cast<uintmax_t>(1e2)> centi;
typedef ratio<1, static_cast<uintmax_t>(1e3)> milli;
typedef ratio<1, static_cast<uintmax_t>(1e6)> micro;
typedef ratio<1, static_cast<uintmax_t>(1e9)> nano;
typedef ratio<1, static_cast<uintmax_t>(1e12)> pico;
typedef ratio<1, static_cast<uintmax_t>(1e15)> femto;
typedef ratio<1, static_cast<uintmax_t>(1e18)> atto;

// standard duration
using duration_t = __time_t;
using duration_d = double;
constexpr duration_d duration_ratio = 1000;     // the standard SI metric prefix (power of 1000)

template <typename S = base_ratio>
inline constexpr duration_d
seconds(const duration_d s)
{
  return (s * S::denom) / S::num;
}
template <typename S = milli>
inline constexpr duration_d
milliseconds(const duration_d s)
{
  return (s * S::denom) / S::num;
}
template <typename S = micro>
inline constexpr duration_d
microseconds(const duration_d s)
{
  return (s * S::denom) / S::num;
}
template <typename S = nano>
inline constexpr duration_d
nanoseconds(const duration_d s)
{
  return (s * S::denom) / S::num;
}
};
};
