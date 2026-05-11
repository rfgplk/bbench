
//          Copyright David Lucius Severus 2024-.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "../src/bench.hpp"
#include "../src/events.hpp"
#include "../src/format.hpp"
#include "../src/metrics.hpp"
#include "../src/options.hpp"
#include "../src/topdown.hpp"

#include <micron/io/stdout.hpp>
#include <micron/vector.hpp>
#include <micron/memory/cmemory.hpp>

namespace {

struct cli_opts {
  bbench::benchmark_opts bench_opts;
  usize n_runs = 1;
  bool verbose = false;
  bool table = false;
  bool topdown_only = false;
  char csv_sep = '\0';     // '\0' means: use human format
  const char *output_file = nullptr;
  const char *metrics_csv = nullptr;
  micron::vector<const char *> paths;
};

inline bool
parse_int(const char *s, long long &out) {
  if (!s || !*s) return false;
  long long v = 0; bool neg = false;
  while (*s == ' ') ++s;
  if (*s == '-') { neg = true; ++s; } else if (*s == '+') ++s;
  if (!(*s >= '0' && *s <= '9')) return false;
  while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); ++s; }
  out = neg ? -v : v;
  return true;
}

inline bool
arg_eq(const char *a, const char *b) {
  return micron::strcmp(a, b) == 0;
}

void print_usage(void) {
  micron::io::println("bbench [options] BINARY [BINARY...]");
  micron::io::println("  -n / -r N         repeat N times; print mean +- stddev (min/max)");
  micron::io::println("  -d / -dd / -ddd   detail level (default 1; 2 adds TLB+misses; 3 adds prefetch+faults)");
  micron::io::println("  -e EVENT...       custom event set by symbolic name");
  micron::io::println("  -D MS             delay measurement start by MS ms");
  micron::io::println("  --timeout MS      kill child after MS ms");
  micron::io::println("  --pre  CMD        run CMD before each measurement");
  micron::io::println("  --post CMD        run CMD after each measurement");
  micron::io::println("  --table           per-run table");
  micron::io::println("  -x SEP            CSV output with field separator SEP");
  micron::io::println("  -o FILE           output to FILE");
  micron::io::println("  -v / --verbose    show counter open errors");
  micron::io::println("  -M METRIC...      derived metrics (ipc, branch-miss-rate, cache-miss-rate, …)");
  micron::io::println("  --topdown         emit Intel Icelake+ top-down quadrant breakdown");
  micron::io::println("  --no-inherit      don't inherit counters to child threads");
  micron::io::println("  --no-scale        print raw counts, skip multiplex scaling");
  micron::io::println("  --all-user        restrict counters to user mode (default)");
  micron::io::println("  --all-kernel      restrict counters to kernel mode");
  micron::io::println("  --pinned          force counters pinned (fail-open instead of multiplexing)");
}

bool parse_argv(int argc, char **argv, cli_opts &out) {
  for (int i = 1; i < argc; ++i) {
    const char *a = argv[i];
    auto need_value = [&](const char *flag, const char *&dst) -> bool {
      if (i + 1 >= argc) {
        bbench::format::sink err = bbench::format::sink::stderr_sink();
        err.emit("bbench: "); err.emit(flag); err.emit(" requires a value\n");
        return false;
      }
      dst = argv[++i];
      return true;
    };
    auto need_int = [&](const char *flag, long long &dst) -> bool {
      const char *v = nullptr;
      if (!need_value(flag, v)) return false;
      if (!parse_int(v, dst)) {
        bbench::format::sink err = bbench::format::sink::stderr_sink();
        err.emit("bbench: "); err.emit(flag); err.emit(" requires an integer\n");
        return false;
      }
      return true;
    };

    if (arg_eq(a, "-n") || arg_eq(a, "-r")) {
      long long v; if (!need_int(a, v) || v < 1) return false;
      out.n_runs = static_cast<usize>(v);
    } else if (arg_eq(a, "-d")) {
      out.bench_opts.detail = 1;
    } else if (arg_eq(a, "-dd")) {
      out.bench_opts.detail = 2;
    } else if (arg_eq(a, "-ddd")) {
      out.bench_opts.detail = 3;
    } else if (arg_eq(a, "-e")) {
      const char *v = nullptr;
      if (!need_value(a, v)) return false;
      out.bench_opts.event_csv = v;
    } else if (arg_eq(a, "-D")) {
      long long v; if (!need_int(a, v) || v < 0) return false;
      out.bench_opts.delay_ms = static_cast<u32>(v);
    } else if (arg_eq(a, "--timeout")) {
      long long v; if (!need_int(a, v) || v < 0) return false;
      out.bench_opts.timeout_ms = static_cast<u32>(v);
    } else if (arg_eq(a, "--pre")) {
      if (!need_value(a, out.bench_opts.pre)) return false;
    } else if (arg_eq(a, "--post")) {
      if (!need_value(a, out.bench_opts.post)) return false;
    } else if (arg_eq(a, "--table")) {
      out.table = true;
    } else if (arg_eq(a, "-x")) {
      const char *v = nullptr;
      if (!need_value(a, v)) return false;
      out.csv_sep = v[0] ? v[0] : ',';
    } else if (arg_eq(a, "-o")) {
      if (!need_value(a, out.output_file)) return false;
    } else if (arg_eq(a, "-v") || arg_eq(a, "--verbose")) {
      out.verbose = true;
    } else if (arg_eq(a, "-M")) {
      if (!need_value(a, out.metrics_csv)) return false;
    } else if (arg_eq(a, "--topdown")) {
      out.topdown_only = true;
    } else if (arg_eq(a, "--no-inherit")) {
      out.bench_opts.inherit = false;
    } else if (arg_eq(a, "--no-scale")) {
      out.bench_opts.scale = false;
    } else if (arg_eq(a, "--all-user")) {
      out.bench_opts.excl_kernel = true;
      out.bench_opts.excl_user = false;
    } else if (arg_eq(a, "--all-kernel")) {
      out.bench_opts.excl_kernel = false;
      out.bench_opts.excl_user = true;
    } else if (arg_eq(a, "--pinned")) {
      out.bench_opts.pinned = true;
    } else if (arg_eq(a, "-h") || arg_eq(a, "--help")) {
      print_usage();
      return false;
    } else if (a[0] == '-' && a[1] != '\0') {
      bbench::format::sink err = bbench::format::sink::stderr_sink();
      err.emit("bbench: unknown flag: "); err.emit(a); err.newline();
      return false;
    } else {
      out.paths.push_back(a);
    }
  }
  if (out.paths.size() == 0) {
    print_usage();
    return false;
  }
  return true;
}

inline long long
avg_int(const micron::vector<bbench::benchmark_t> &v, long long bbench::benchmark_t::*f) {
  long long sum = 0;
  for (const auto &b : v) sum += b.*f;
  return sum / static_cast<long long>(v.size());
}

inline double
avg_double(const micron::vector<bbench::benchmark_t> &v, double bbench::benchmark_t::*f) {
  double sum = 0.0;
  for (const auto &b : v) sum += b.*f;
  return sum / static_cast<double>(v.size());
}

bbench::benchmark_t
collapse_runs(const micron::vector<bbench::benchmark_t> &runs) {
  bbench::benchmark_t out{};
  if (runs.size() == 0) return out;
  out.name = runs.front().name;
  out.time             = avg_double(runs, &bbench::benchmark_t::time);
  out.cycles           = avg_int   (runs, &bbench::benchmark_t::cycles);
  out.instructions     = avg_int   (runs, &bbench::benchmark_t::instructions);
  out.cache_misses     = avg_int   (runs, &bbench::benchmark_t::cache_misses);
  out.total_branches   = avg_int   (runs, &bbench::benchmark_t::total_branches);
  out.branch_misses    = avg_int   (runs, &bbench::benchmark_t::branch_misses);
  out.total_cycles     = avg_int   (runs, &bbench::benchmark_t::total_cycles);
  out.cpu_time         = avg_int   (runs, &bbench::benchmark_t::cpu_time);
  out.context_switches = avg_int   (runs, &bbench::benchmark_t::context_switches);
  out.migrations       = avg_int   (runs, &bbench::benchmark_t::migrations);
  out.l1_cache         = avg_int   (runs, &bbench::benchmark_t::l1_cache);
  out.l1t_cache        = avg_int   (runs, &bbench::benchmark_t::l1t_cache);
  out.ll_cache         = avg_int   (runs, &bbench::benchmark_t::ll_cache);
  out.access           = avg_int   (runs, &bbench::benchmark_t::access);
  out.bpu              = avg_int   (runs, &bbench::benchmark_t::bpu);
  out.page_faults      = avg_int   (runs, &bbench::benchmark_t::page_faults);
  out.bus_cycles       = avg_int   (runs, &bbench::benchmark_t::bus_cycles);
  out.stalled_front    = avg_int   (runs, &bbench::benchmark_t::stalled_front);
  out.stalled_back     = avg_int   (runs, &bbench::benchmark_t::stalled_back);
  out.dtlb_access      = avg_int   (runs, &bbench::benchmark_t::dtlb_access);
  out.dtlb_miss        = avg_int   (runs, &bbench::benchmark_t::dtlb_miss);
  out.itlb_access      = avg_int   (runs, &bbench::benchmark_t::itlb_access);
  out.itlb_miss        = avg_int   (runs, &bbench::benchmark_t::itlb_miss);
  out.l1d_miss         = avg_int   (runs, &bbench::benchmark_t::l1d_miss);
  out.l1t_miss         = avg_int   (runs, &bbench::benchmark_t::l1t_miss);
  out.llcache_miss     = avg_int   (runs, &bbench::benchmark_t::llcache_miss);
  out.l1d_prefetch     = avg_int   (runs, &bbench::benchmark_t::l1d_prefetch);
  out.l1d_prefetch_miss = avg_int  (runs, &bbench::benchmark_t::l1d_prefetch_miss);
  out.minor_faults     = avg_int   (runs, &bbench::benchmark_t::minor_faults);
  out.major_faults     = avg_int   (runs, &bbench::benchmark_t::major_faults);
  out.alignment_faults = avg_int   (runs, &bbench::benchmark_t::alignment_faults);
  out.emulation_faults = avg_int   (runs, &bbench::benchmark_t::emulation_faults);
  return out;
}

void
sort_results(micron::vector<micron::vector<bbench::benchmark_t>> &v) {
  for (usize i = 0; i + 1 < v.size(); ++i) {
    usize mn = i;
    for (usize j = i + 1; j < v.size(); ++j)
      if (collapse_runs(v[j]).time < collapse_runs(v[mn]).time) mn = j;
    if (mn != i) {
      auto tmp = micron::move(v[i]);
      v[i] = micron::move(v[mn]);
      v[mn] = micron::move(tmp);
    }
  }
}

void
emit_stats(const bbench::format::sink &out,
           const micron::vector<bbench::benchmark_t> &runs, bool color) {
  using bbench::format::compute_stats;
  auto s_time = compute_stats(runs, [](const bbench::benchmark_t &b) { return b.time; });
  auto s_cyc  = compute_stats(runs, [](const bbench::benchmark_t &b) { return static_cast<double>(b.cycles); });
  auto s_ins  = compute_stats(runs, [](const bbench::benchmark_t &b) { return static_cast<double>(b.instructions); });

  if (color) out.emit("\033[34m", 5);
  out.emit("time (us):     mean=");
  if (color) out.emit("\033[0m", 4);
  out.emit_double(s_time.mean);
  out.emit("  stddev=");  out.emit_double(s_time.stddev);
  out.emit("  min=");     out.emit_double(s_time.mn);
  out.emit("  max=");     out.emit_double(s_time.mx);
  out.newline();

  if (color) out.emit("\033[34m", 5);
  out.emit("cycles:        mean=");
  if (color) out.emit("\033[0m", 4);
  out.emit_double(s_cyc.mean);
  out.emit("  stddev=");  out.emit_double(s_cyc.stddev);
  out.newline();

  if (color) out.emit("\033[34m", 5);
  out.emit("instructions:  mean=");
  if (color) out.emit("\033[0m", 4);
  out.emit_double(s_ins.mean);
  out.emit("  stddev=");  out.emit_double(s_ins.stddev);
  out.newline();
}

void
emit_metrics(const bbench::format::sink &out, const bbench::benchmark_t &b,
             const char *csv, bool color) {
  if (!csv) return;
  char buf[64]; usize bi = 0;
  for (const char *p = csv;; ++p) {
    if (*p == ',' || *p == '\0') {
      if (bi > 0) {
        buf[bi] = '\0';
        const auto *m = bbench::metric::lookup_metric(buf);
        if (m) {
          if (color) out.emit("\033[34m", 5);
          out.emit(m->name);
          out.emit(": ");
          if (color) out.emit("\033[0m", 4);
          out.emit_double(m->fn(b));
          out.newline();
        } else {
          out.emit("metric '");
          out.emit(buf);
          out.emit("' not known\n");
        }
        bi = 0;
      }
      if (*p == '\0') break;
    } else if (bi < sizeof(buf) - 1) {
      buf[bi++] = *p;
    }
  }
}

void
emit_topdown(const bbench::format::sink &out, const bbench::topdown::topdown_t &td, bool color) {
  if (!td.supported) {
    out.emit("topdown: not supported on this CPU/kernel (Intel Icelake+ required)\n");
    return;
  }
  if (color) out.emit("\033[34mtopdown:\033[0m\n", 21);
  else       out.emit("topdown:\n", 9);
  out.emit("  retiring:    "); out.emit_double(td.retiring); out.newline();
  out.emit("  bad-spec:    "); out.emit_double(td.bad_spec); out.newline();
  out.emit("  fe-bound:    "); out.emit_double(td.frontend); out.newline();
  out.emit("  be-bound:    "); out.emit_double(td.backend);  out.newline();
}

} // anonymous namespace

int
main(int argc, char **argv) {
  if (argc < 2) {
    print_usage();
    return -1;
  }
  cli_opts cli;
  if (!parse_argv(argc, argv, cli)) return -1;

  // open output sink (stdout or file)
  bbench::format::sink out = cli.output_file
      ? bbench::format::sink::file_sink(cli.output_file)
      : bbench::format::sink::stdout_sink();
  const bool color = cli.output_file == nullptr && cli.csv_sep == '\0';

  if (cli.bench_opts.event_csv) {
    micron::vector<bbench::event_def> events;
    if (!bbench::parse_event_list(cli.bench_opts.event_csv, events)) {
      bbench::format::sink err = bbench::format::sink::stderr_sink();
      err.emit("bbench: one or more event names in -e were not recognized\n");
    }
    for (const char *path : cli.paths) {
      bbench::dynamic_result_t res{};
      for (usize r = 0; r < cli.n_runs; ++r)
        res = bbench::benchmark_bin_dynamic(path, events, cli.bench_opts);
      out.emit(path); out.emit(": time(us)="); out.emit_double(res.time); out.newline();
      for (const auto &row : res.rows) {
        out.emit("  ");
        out.emit(row.name);
        out.emit(": ");
        out.emit_int(row.value);
        if (cli.verbose && row.err) {
          out.emit("  [open failed: errno=");
          out.emit_int(row.err);
          out.emit("]");
        }
        out.newline();
      }
    }
    return 0;
  }

  micron::vector<micron::vector<bbench::benchmark_t>> all_results;
  for (const char *path : cli.paths) {
    micron::vector<bbench::benchmark_t> runs;
    for (usize r = 0; r < cli.n_runs; ++r) {
      runs.emplace_back(bbench::benchmark_bin(path, cli.bench_opts));
    }
    all_results.push_back(micron::move(runs));
  }

  sort_results(all_results);

  if (cli.csv_sep != '\0') bbench::format::emit_csv_header(out, cli.bench_opts.detail, cli.csv_sep);

  bool first = true;
  for (auto &runs : all_results) {
    bbench::benchmark_t agg = collapse_runs(runs);
    if (cli.csv_sep != '\0') {
      bbench::format::emit_csv_one(out, agg, cli.bench_opts.detail, cli.csv_sep);
    } else {
      if (!first) out.newline();
      first = false;
      bbench::format::emit_human_one(out, agg, cli.bench_opts.detail, color, cli.n_runs);
      if (cli.n_runs > 1)
        emit_stats(out, runs, color);
      if (cli.table)
        bbench::format::emit_table(out, runs);
      if (cli.metrics_csv)
        emit_metrics(out, agg, cli.metrics_csv, color);
      if (cli.topdown_only) {
        bbench::topdown::topdown_t td = bbench::topdown::measure_bin(runs.front().name.c_str(), cli.bench_opts);
        emit_topdown(out, td, color);
      }
    }
    if (cli.verbose && cli.n_runs == 1) {
    }
  }
  return 0;
}
