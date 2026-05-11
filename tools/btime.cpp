
//          Copyright David Lucius Severus 2024-.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "../src/bench.hpp"
#include "../src/format.hpp"

#include <micron/io/stdout.hpp>
#include <micron/vector.hpp>
#include <micron/memory/cmemory.hpp>

namespace {

inline bool
parse_int(const char *s, long long &out) {
  if (!s || !*s) return false;
  long long v = 0; bool neg = false;
  if (*s == '-') { neg = true; ++s; } else if (*s == '+') ++s;
  if (!(*s >= '0' && *s <= '9')) return false;
  while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); ++s; }
  out = neg ? -v : v;
  return true;
}

void usage(void) {
  micron::io::println("btime [-r N] [-x SEP] [-q] BINARY");
  micron::io::println("  -r N    repeat N times; print mean / stddev / min / max");
  micron::io::println("  -x SEP  CSV output, one row per run + final summary");
  micron::io::println("  -q      quiet — print only the number(s), no labels");
}

};

int
main(int argc, char **argv) {
  if (argc < 2) {
    usage();
    return -1;
  }

  usize n_runs = 1;
  char csv_sep = '\0';
  bool quiet = false;
  const char *path = nullptr;

  for (int i = 1; i < argc; ++i) {
    const char *a = argv[i];
    if (micron::strcmp(a, "-r") == 0) {
      if (i + 1 >= argc) { usage(); return -1; }
      long long v;
      if (!parse_int(argv[++i], v) || v < 1) { usage(); return -1; }
      n_runs = static_cast<usize>(v);
    } else if (micron::strcmp(a, "-x") == 0) {
      if (i + 1 >= argc) { usage(); return -1; }
      const char *s = argv[++i];
      csv_sep = s[0] ? s[0] : ',';
    } else if (micron::strcmp(a, "-q") == 0) {
      quiet = true;
    } else if (micron::strcmp(a, "-h") == 0 || micron::strcmp(a, "--help") == 0) {
      usage();
      return 0;
    } else if (a[0] == '-' && a[1] != '\0') {
      usage();
      return -1;
    } else {
      path = a;
      break;
    }
  }
  if (!path) { usage(); return -1; }

  micron::vector<double> times;
  times.reserve(n_runs);
  for (usize i = 0; i < n_runs; ++i)
    times.push_back(bbench::bench_bin<bbench::time_resolution::us>(path));

  bbench::format::sink out = bbench::format::sink::stdout_sink();

  if (csv_sep != '\0') {
    const char s[2] = { csv_sep, '\0' };
    if (!quiet) {
      out.emit("run"); out.emit(s); out.emit("time_us"); out.newline();
    }
    for (usize i = 0; i < times.size(); ++i) {
      out.emit_int(static_cast<long long>(i + 1));
      out.emit(s);
      out.emit_double(times[i]);
      out.newline();
    }
    if (!quiet && times.size() > 1) {
      auto stats = bbench::format::compute_stats(times, [](double t) { return t; });
      out.emit("mean"); out.emit(s); out.emit_double(stats.mean); out.newline();
      out.emit("stddev"); out.emit(s); out.emit_double(stats.stddev); out.newline();
      out.emit("min"); out.emit(s); out.emit_double(stats.mn); out.newline();
      out.emit("max"); out.emit(s); out.emit_double(stats.mx); out.newline();
    }
    return 0;
  }

  // Non-CSV path
  if (n_runs == 1) {
    if (quiet) {
      out.emit_double(times[0]);
      out.newline();
    } else {
      out.emit("time(us): ");
      out.emit_double(times[0]);
      out.newline();
    }
    return 0;
  }

  auto stats = bbench::format::compute_stats(times, [](double t) { return t; });
  if (quiet) {
    out.emit_double(stats.mean); out.emit(" ");
    out.emit_double(stats.stddev); out.emit(" ");
    out.emit_double(stats.mn); out.emit(" ");
    out.emit_double(stats.mx); out.newline();
  } else {
    out.emit("time(us): mean="); out.emit_double(stats.mean);
    out.emit("  stddev=");      out.emit_double(stats.stddev);
    out.emit("  min=");         out.emit_double(stats.mn);
    out.emit("  max=");         out.emit_double(stats.mx);
    out.newline();
  }
  return 0;
}
