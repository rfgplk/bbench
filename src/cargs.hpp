#pragma once

#include <array>
#include <cstring>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#ifdef CARGS_EXC
#include <signal.h>
#endif
#include "inplace_string.h"

namespace cargs
{
typedef void (*gfcallback)(int, const char *, const char *);
typedef void (*fcallback)(const char *, const char *);

enum class Accepts : int { czero, cvoid, cpath, cparam, coperand, ckey, cbool, cint, clong, cfloat, cdouble, cstring };

namespace
{
template <std::size_t N> struct packet {
  inplace_string<N> key;
  inplace_string<N> data;
  Accepts type = Accepts::czero;
};

template <typename T> struct get_arity : get_arity<decltype(&T::operator())> {
};
template <typename R, typename... Args>
struct get_arity<R (*)(Args...)> : std::integral_constant<unsigned, sizeof...(Args)> {
};
template <typename R, typename C, typename... Args>
struct get_arity<R (C::*)(Args...)> : std::integral_constant<unsigned, sizeof...(Args)> {
};
template <typename R, typename C, typename... Args>
struct get_arity<R (C::*)(Args...) const> : std::integral_constant<unsigned, sizeof...(Args)> {
};
};

template <typename Fn> struct param_packet {
  constexpr param_packet() {}
  template <std::size_t N, std::size_t M, std::size_t L>
  constexpr param_packet(const char (&s)[N], const char (&s2)[M], const char (&s3)[L], const Accepts a = Accepts::czero)
      : param(s), abbr(s2), description(s3), acc(a), ptr(nullptr)
  {
  }
  template <std::size_t N, std::size_t M, std::size_t L>
  constexpr param_packet(const char (&s)[N], const char (&s2)[M], const char (&s3)[L], const Accepts a, Fn _ptr)
      : param(s), abbr(s2), description(s3), acc(a), ptr(_ptr)
  {
  }
  constexpr param_packet(const param_packet &o)
      : param(o.param), abbr(o.abbr), description(o.description), acc(o.acc), ptr(o.ptr)
  {
  }
  constexpr Accepts
  accepts() const
  {
    return acc;
  }
  constexpr param_packet &
  operator=(const param_packet &o)
  {
    param = o.param;
    abbr = o.abbr;
    description = o.description;
    acc = o.acc;
    ptr = o.ptr;
    return *this;
  }
  template <std::size_t N>
  param_packet &
  operator()(const packet<N> &p)
  {
    if constexpr ( std::is_pointer<Fn>::value ) {
      if ( ptr ) {
        if constexpr ( get_arity<Fn>() == 0 )
          ptr();
        if constexpr ( get_arity<Fn>() == 1 )
          ptr(p.key.data());
        if constexpr ( get_arity<Fn>() == 2 )
          ptr(p.key.data(), p.data.data());
      }
    }
    return *this;
  }
  // private:
  std::string_view param;
  std::string_view abbr;
  std::string_view description;
  Accepts acc;
  Fn ptr;
};

template <typename Fn = std::nullptr_t, std::size_t N, std::size_t M, std::size_t L>
inline constexpr param_packet<Fn>
make_packet(const char (&s)[N], const char (&s2)[M], const char (&s3)[L])
{
  return param_packet<Fn>(s, s2, s3);
}

template <typename Fn = std::nullptr_t, std::size_t N, std::size_t M, std::size_t L>
inline constexpr param_packet<Fn>
make_packet(const char (&s)[N], const char (&s2)[M], const char (&s3)[L], const Accepts s4)
{
  return param_packet<Fn>(s, s2, s3, s4);
}

template <typename Fn = std::nullptr_t, std::size_t N, std::size_t M, std::size_t L>
inline constexpr param_packet<Fn>
make_packet(const char (&s)[N], const char (&s2)[M], const char (&s3)[L], const Accepts s4, Fn ptr = nullptr)
{
  return param_packet<Fn>(s, s2, s3, s4, ptr);
}

template <std::size_t P_COUNT = 0, typename C = std::nullptr_t, gfcallback F = nullptr, char16_t FLAG = '-',
          int64_t MAX_LENGTH = 32, int64_t MAX_BUFFER = 127>
class parser
{
  static constexpr gfcallback global_callback = F;
  std::array<packet<MAX_BUFFER>, MAX_LENGTH> args;
  std::array<param_packet<C>, P_COUNT> parameters;

  inline void
  to_number(std::variant<long, long long, double, bool> &out, const char *str, const Accepts type) const
  {
    switch ( type ) {
    case Accepts::cint:
      out = std::stol(str);
      break;
    case Accepts::clong:
      out = std::stoll(str);
      break;
    case Accepts::cfloat:
      out = std::stof(str);
      break;
    case Accepts::cdouble:
      out = std::stod(str);
      break;
    }
  }
  inline std::variant<long, long long, double, bool>
  to_number(const char *str, const Accepts type) const
  {
    switch ( type ) {
    case Accepts::cint:
      return std::stol(str);
      break;
    case Accepts::clong:
      return std::stoll(str);
      break;
    case Accepts::cfloat:
      return std::stof(str);
      break;
    case Accepts::cdouble:
      return std::stod(str);
      break;
    }
    return false;
  }
  inline bool
  number(char *str) const
  {
    if ( str == nullptr )
      return false;
    while ( *str++ )
      if ( !std::isdigit(*str) && *str != '.' )
        return false;
    return true;
  }
  inline bool
  number(const inplace_string<MAX_BUFFER> &arg) const
  {
    for ( auto &c : arg )
      if ( !std::isdigit(c) && c != '.' )
        return false;
    return true;
  }
  inline bool
  empty(char *str) const
  {
    if ( str == nullptr )
      return true;
    while ( *str ) {
      if ( *str != FLAG && *str != 0 )
        return false;
      str++;
    }
    return true;
  }
  inline bool
  param(char *str) const
  {
    if ( str == nullptr )
      return false;
    return (*str == FLAG);
  }
  inline bool
  flag(char *ptr) const
  {
    char c = 0;
    while ( *ptr ) {
      if ( *ptr++ == FLAG )
        if ( c++ > 0 )
          return true;
    }
    return false;
  }
  inline char *
  trim(char *ptr) const
  {
    while ( *ptr ) {
      if ( *ptr == FLAG )
        ptr++;
      else
        break;
    }
    return ptr;
  }

  inline __attribute__((always_inline)) void
  callbacks_handler()
  {
    int c = 0;
    do {
      if ( args[c].type == Accepts::czero )
        break;
      for ( auto &n : parameters )
        if ( args[c].key == n.param || args[c].key == n.abbr )
          n(args[c]);
      if constexpr ( global_callback )
        global_callback(c, args[c].key.data(), args[c].data.data());
    } while ( ++c );
  }

  inline __attribute__((always_inline)) void
  general_parse(int argc, char **_args)
  {
    if ( argc > MAX_LENGTH )
#ifdef CARGS_EXC
      raise(SIGTERM);
#elif CARGS_STD_EXC
      throw std::runtime_error("Too many arguments present");
#else
      return;
#endif
    if ( argc == 1 )
      return;

    args[0].key += _args[0];
    args[0].type = Accepts::cpath;
    for ( int i = 1; i < argc; i++ ) {
      if ( strlen(_args[i]) > MAX_BUFFER )
#ifdef CARGS_EXC
        raise(SIGTERM);
#elif CARGS_STD_EXC
        throw std::runtime_error("Argument is too long");
#else
        continue;
#endif
      bool fl = flag(_args[i]);
      bool pm = (fl == true) ? false : param(_args[i]);

      auto *ch = trim(_args[i]);
      for ( auto &n : parameters ) {
        if ( n.param == ch && fl ) {
          args[i].key += ch;
          args[i].type = Accepts::ckey;
          goto data;
        } else if ( n.abbr == ch && pm ) {
          args[i].key += ch;
          args[i].type = Accepts::cparam;
          goto data;
        } else {
          args[i].data += _args[i];
          args[i].type = Accepts::coperand;
          goto data;
        }
        continue;
      data:
        if ( n.accepts() != Accepts::czero ) {
          if ( !(i + 1 < argc) )
            break;
          args[i].data += _args[i + 1];
          args[i].type = n.accepts();
        }
        break;
      }
    }
    callbacks_handler();
  }

  inline __attribute__((always_inline)) void
  general_parse_np(int argc, char **_args)
  {
    if ( argc > MAX_LENGTH )
#ifdef CARGS_EXC
      raise(SIGTERM);
#elif CARGS_STD_EXC
      throw std::runtime_error("Too many arguments present");
#else
      return;
#endif
    if ( argc == 1 )
      return;

    args[0].key += _args[0];
    args[0].type = Accepts::cpath;
    for ( int i = 1; i < argc; i++ ) {
      if ( strlen(_args[i]) > MAX_BUFFER )
#ifdef CARGS_EXC
        raise(SIGTERM);
#elif CARGS_STD_EXC
        throw std::runtime_error("Argument is too long");
#else
        continue;
#endif
      bool fl = flag(_args[i]);
      bool pm = (fl == true) ? false : param(_args[i]);
      bool emp = empty(_args[i]);

      if ( !emp ) {
        if ( fl ) {
          args[i].key += trim(_args[i]);
          args[i].type = Accepts::ckey;
          continue;
        } else if ( pm ) {
          args[i].key += trim(_args[i]);
          args[i].type = Accepts::cparam;
          continue;
        }
      }
      if ( args[i - 1].type == Accepts::ckey ) {
        args[i - 1].data += _args[i];
        args[i - 1].type = Accepts::cstring;
        args[i].type = Accepts::czero;
      } else {
        args[i].data += _args[i];
        args[i].type = Accepts::coperand;
      }
    }
    callbacks_handler();
  }

public:
  parser() = delete;
  template <class... Y> constexpr parser(Y &&...ls) : parameters{ ls... } {}
  explicit parser(int argc, char **_args) { general_parse_np(argc, _args); }
  parser(const parser &o) : args(o.args) {}
  parser(parser &&o) : args(std::move(o.args)) {}
  parser &
  operator=(const parser &o)
  {
    args = o.args;
    return *this;
  }
  parser &
  operator=(parser &&o)
  {
    args = std::move(o.args);
    return *this;
  }
  parser &
  operator()(int argc, char **argv)
  {
    general_parse(argc, argv);
    return *this;
  }
  const auto &
  get() const
  {
    return args;
  }
  template <typename Y = inplace_string<MAX_BUFFER>>
  std::vector<Y>
  list() const
  {
    std::vector<Y> tmp;
    for ( auto &n : args )
      if ( n.type >= Accepts::ckey )
        tmp.emplace_back(n.key);
    return tmp;
  }
  template <typename Y = inplace_string<MAX_BUFFER>>
  std::vector<Y>
  data() const
  {
    std::vector<Y> tmp;
    for ( auto &n : args )
      if ( n.type >= Accepts::ckey )
        tmp.emplace_back(n.data);
    return tmp;
  }
  template <typename Y = inplace_string<MAX_BUFFER>>
  std::vector<Y>
  params() const
  {
    std::vector<Y> tmp;
    for ( auto &n : args )
      if ( n.type == Accepts::cparam )
        tmp.emplace_back(n.key);
    return tmp;
  }
  template <typename Y = inplace_string<MAX_BUFFER>>
  std::vector<Y>
  operands() const
  {
    std::vector<Y> tmp;
    for ( auto &n : args )
      if ( n.type == Accepts::coperand )
        tmp.emplace_back(n.data);
    return tmp;
  }
  inline std::variant<long, long long, double, bool>
  get(const std::string &str) const
  {
    for ( auto &n : args )
      if ( str.compare(n.key.data()) == 0 )
        return to_number(n.data.data(), n.type);
    return false;
  }
  inline std::variant<long, long long, double, bool>
  get(const char *const str) const
  {
    for ( auto &n : args )
      if ( std::strcmp(str, n.key.data()) == 0 )
        return (to_number(n.data.data(), n.type));
    return false;
  }
  inline auto
  operator()(const std::string &str) const
  {
    for ( auto &n : args )
      if ( str.compare(n.key.data()) == 0 )
        return n.data;
    return inplace_string<MAX_BUFFER>();
  }
  inline auto
  operator()(const char *const str) const
  {
    for ( auto &n : args )
      if ( std::strcmp(str, n.key.data()) == 0 )
        return n.data;
    return inplace_string<MAX_BUFFER>();
  }
  // Does the argument exist?
  inline bool
  operator[](const char *const str) const
  {
    for ( auto &n : args )
      if ( std::strcmp(str, n.key.data()) == 0 )
        return true;
    return false;
  }
  bool
  operator[](const std::string &str) const
  {
    for ( auto &n : args )
      if ( str.compare(n.key.data()) == 0 )
        return true;
    return false;
  }
  bool
  operator[](std::initializer_list<const char *const> &&list) const
  {
    for ( auto &n : args )
      for ( auto &c : list )
        if ( std::strcmp(c, n.key.data()) == 0 )
          return true;
    return false;
  }
};
};
