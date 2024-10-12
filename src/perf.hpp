
//          Copyright David Lucius Severus 2024-.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>
#include <cstring>
#include <linux/perf_event.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include <concepts>
#include <type_traits>

#include "time.hpp"

static long
perf_event(struct perf_event_attr &event, pid_t pid, int cpu, int group_fd, unsigned long flags)
{
  return syscall(SYS_perf_event_open, &event, pid, cpu, group_fd, flags);
};

static long
perf_event_this(struct perf_event_attr &event)
{
  return syscall(SYS_perf_event_open, &event, 0, -1, -1, 0);
};

static long
perf_event_pid(struct perf_event_attr &event, pid_t pid)
{
  return syscall(SYS_perf_event_open, &event, pid, -1, -1, 0);
};

namespace bbench
{

enum class kernel_clock_types : uint64_t {
  hardware = PERF_TYPE_HARDWARE,
  software = PERF_TYPE_SOFTWARE,
  tracepoint = PERF_TYPE_TRACEPOINT,
  cache = PERF_TYPE_HW_CACHE
};

namespace options
{

enum class hardware : uint64_t {
  cpu_cycles = PERF_COUNT_HW_CPU_CYCLES,
  total_inst = PERF_COUNT_HW_INSTRUCTIONS,
  cache_misses = PERF_COUNT_HW_CACHE_MISSES,
  branches = PERF_COUNT_HW_BRANCH_INSTRUCTIONS,
  branch_misses = PERF_COUNT_HW_BRANCH_MISSES,
  bus_cycles = PERF_COUNT_HW_BUS_CYCLES,
  stalled_init = PERF_COUNT_HW_STALLED_CYCLES_FRONTEND,
  stalled_retirement = PERF_COUNT_HW_STALLED_CYCLES_BACKEND,
  total_cycles = PERF_COUNT_HW_REF_CPU_CYCLES,
  none
};

enum class software : uint64_t {
  cpu_clock = PERF_COUNT_SW_CPU_CLOCK,
  tasks = PERF_COUNT_SW_TASK_CLOCK,
  page_faults = PERF_COUNT_SW_PAGE_FAULTS,
  cntx_swtch = PERF_COUNT_SW_CONTEXT_SWITCHES,
  cpu_migrations = PERF_COUNT_SW_CPU_MIGRATIONS,
  minor_page_flt = PERF_COUNT_SW_PAGE_FAULTS_MIN,
  major_page_flt = PERF_COUNT_SW_PAGE_FAULTS_MAJ,
  alignment_faults = PERF_COUNT_SW_ALIGNMENT_FAULTS,
  emulation_faults = PERF_COUNT_SW_EMULATION_FAULTS,
  nothing = PERF_COUNT_SW_DUMMY,
  sample = PERF_COUNT_SW_BPF_OUTPUT,
  cgroup_swtch = PERF_COUNT_SW_CGROUP_SWITCHES,
  none
};

enum class cache : uint64_t {
  level1d = PERF_COUNT_HW_CACHE_L1D,
  level1t = PERF_COUNT_HW_CACHE_L1I,
  last_level = PERF_COUNT_HW_CACHE_LL,
  data_tlb = PERF_COUNT_HW_CACHE_DTLB,
  instr_tlb = PERF_COUNT_HW_CACHE_ITLB,
  branch = PERF_COUNT_HW_CACHE_BPU,
  local_access = PERF_COUNT_HW_CACHE_NODE,
  none
};
};     // namespace options

// struct perf_event_attr {
//   __u32 type;   /* Type of event */
//   __u32 size;   /* Size of attribute structure */
//    __u64 config; /* Type-specific configuration */

//   union {
//   __u64 sample_period; /* Period of sampling */
//     __u64 sample_freq;   /* Frequency of sampling */
//   };
//
//   __u64 sample_type; /* Specifies values included in sample */
//   __u64 read_format; /* Specifies values returned in read */
//
//   __u64 disabled : 1,     /* off by default */
//       inherit : 1,        /* children inherit it */
//       pinned : 1,         /* must always be on PMU */
//       exclusive : 1,      /* only group on PMU */
//       exclude_user : 1,   /* don't count user */
//       exclude_kernel : 1, /* don't count kernel */
//       exclude_hv : 1,     /* don't count hypervisor */
//       exclude_idle : 1,   /* don't count when idle */
//      mmap : 1,           /* include mmap data */
//      comm : 1,           /* include comm data */
//    freq : 1,           /* use freq, not period */
//       inherit_stat : 1,   /* per task counts */
//    enable_on_exec : 1, /* next exec enables */
//       task : 1,           /* trace fork/exit */
//    watermark : 1,      /* wakeup_watermark */
//       precise_ip : 2,     /* skid constraint */
//    mmap_data : 1,      /* non-exec mmap data */
//   sample_id_all : 1,  /* sample_type all events */
//      exclude_host : 1,   /* don't count in host */
//      exclude_guest : 1,  /* don't count in guest */
//    exclude_callchain_kernel : 1,
//      /* exclude kernel callchains */
//    exclude_callchain_user : 1,
//    /* exclude user callchains */
//   mmap2 : 1,          /* include mmap with inode data */
//       comm_exec : 1,      /* flag comm events that are
//                            due to exec */
//     use_clockid : 1,    /* use clockid for time fields */
//       context_switch : 1, /* context switch data */
//       write_backward : 1, /* Write ring buffer from end
//                           to beginning */
//       namespaces : 1,     /* include namespaces data */
//   ksymbol : 1,        /* include ksymbol events */
//       bpf_event : 1,      /* include bpf events */
//    aux_output : 1,     /* generate AUX records
//                              instead of events */
//     cgroup : 1,         /* include cgroup events */
//       text_poke : 1,      /* include text poke events */
//    build_id : 1,       /* use build id in mmap2 events */
//       inherit_thread : 1, /* children only inherit */
//                        /* if cloned with CLONE_THREAD */
//       remove_on_exec : 1, /* event is removed from task
//                        on exec */
//    sigtrap : 1,        /* send synchronous SIGTRAP
//                     on event */
//
//    __reserved_1 : 26;
//
//   union {
//   __u32 wakeup_events;    /* wakeup every n events */
//     __u32 wakeup_watermark; /* bytes before wakeup */
//   };
//     __u32     bp_type;          /* breakpoint type */
//
//               union {
//                  __u64 bp_addr;          /* breakpoint address */
//                   __u64 kprobe_func;      /* for perf_kprobe */
//                   __u64 uprobe_path;      /* for perf_uprobe */
//                   __u64 config1;          /* extension of config */
//               };
//
//             union {
//               __u64 bp_len;           /* breakpoint length */
//             __u64 kprobe_addr;      /* with kprobe_func == NULL */
//           __u64 probe_offset;     /* for perf_[k,u]probe */
//         __u64 config2;          /* extension of config1 */
//   };
//               __u64 branch_sample_type;   /* enum perf_branch_sample_type */
//             __u64 sample_regs_user;     /* user regs to dump on samples */
//           __u32 sample_stack_user;    /* size of stack to dump on
//                                        samples */
//       __s32 clockid;              /* clock to use for time fields */
//     __u64 sample_regs_intr;     /* regs to dump on samples */
//   __u32 aux_watermark;        /* aux bytes before wakeup */
//               __u16 sample_max_stack;     /* max frames in callchain */
//             __u16 __reserved_2;         /* align to u64 */
//           __u32 aux_sample_size;      /* max aux sample size */
//         __u32 __reserved_3;         /* align to u64 */
//       __u64 sig_data;             /* user data for sigtrap */

struct time_userland {
};
struct time_kernelland {
};
struct time_vmland {
};
struct time_everyland {
};

template <class T, kernel_clock_types R> struct kernel_clock {
  int e_fd;
  struct perf_event_attr event;
  ~kernel_clock(void) { ::close(e_fd); }

  template <class P>
    requires(std::same_as<P, options::hardware> or std::same_as<P, options::software> or std::same_as<P, options::cache>)
  kernel_clock(P e_type)
  {
    std::memset(&event, 0, sizeof(event));

    if constexpr ( std::same_as<T, time_everyland> ) {
      event.size = sizeof(event);
      if constexpr ( R == kernel_clock_types::hardware ) {
        event.type = PERF_TYPE_HARDWARE;
        event.disabled = 1;
        event.pinned = 1;

        if constexpr ( std::is_same_v<P, options::hardware> )
          event.config = static_cast<__u64>(e_type);
      }
      if constexpr ( R == kernel_clock_types::software ) {
        event.type = PERF_TYPE_SOFTWARE;
        event.disabled = 1;
        event.pinned = 1;

        event.exclude_hv = 1;

        if constexpr ( std::is_same_v<P, options::software> )
          event.config = static_cast<__u64>(e_type);
      }
      if constexpr ( R == kernel_clock_types::tracepoint ) {
        event.type = PERF_TYPE_TRACEPOINT;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_hv = 1;
      }
      if constexpr ( R == kernel_clock_types::cache ) {
        event.type = PERF_TYPE_HW_CACHE;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_hv = 1;
        if constexpr ( std::is_same_v<P, options::cache> )
          event.config = static_cast<__u64>(e_type);
      }
    }

    if constexpr ( std::same_as<T, time_kernelland> ) {
      event.size = sizeof(event);
      if constexpr ( R == kernel_clock_types::hardware ) {
        event.type = PERF_TYPE_HARDWARE;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_user = 1;
        event.exclude_hv = 1;

        if constexpr ( std::is_same_v<P, options::hardware> )
          event.config = static_cast<__u64>(e_type);
      }
      if constexpr ( R == kernel_clock_types::software ) {
        event.type = PERF_TYPE_SOFTWARE;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_user = 1;
        event.exclude_hv = 1;
        if constexpr ( std::is_same_v<P, options::software> )
          event.config = static_cast<__u64>(e_type);
      }
      if constexpr ( R == kernel_clock_types::tracepoint ) {
        event.type = PERF_TYPE_TRACEPOINT;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_user = 1;
        event.exclude_hv = 1;
      }
      if constexpr ( R == kernel_clock_types::cache ) {
        event.type = PERF_TYPE_HW_CACHE;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_user = 1;
        event.exclude_hv = 1;
        if constexpr ( std::is_same_v<P, options::cache> )
          event.config = static_cast<__u64>(e_type);
      }
    }

    if constexpr ( std::same_as<T, time_userland> ) {
      event.size = sizeof(event);
      if constexpr ( R == kernel_clock_types::hardware ) {
        event.type = PERF_TYPE_HARDWARE;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_kernel = 1;
        event.exclude_hv = 1;

        if constexpr ( std::is_same_v<P, options::hardware> )
          event.config = static_cast<__u64>(e_type);
      }
      if constexpr ( R == kernel_clock_types::software ) {
        event.type = PERF_TYPE_SOFTWARE;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_kernel = 1;
        event.exclude_hv = 1;
        if constexpr ( std::is_same_v<P, options::software> )
          event.config = static_cast<__u64>(e_type);
      }
      if constexpr ( R == kernel_clock_types::tracepoint ) {
        event.type = PERF_TYPE_TRACEPOINT;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_kernel = 1;
        event.exclude_hv = 1;
      }
      if constexpr ( R == kernel_clock_types::cache ) {
        event.type = PERF_TYPE_HW_CACHE;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_kernel = 1;
        event.exclude_hv = 1;
        if constexpr ( std::is_same_v<P, options::cache> )
          event.config = static_cast<__u64>(e_type);
      }
    }

    if constexpr ( std::same_as<T, time_vmland> ) {
      event.size = sizeof(event);
      if constexpr ( R == kernel_clock_types::hardware ) {
        event.type = PERF_TYPE_HARDWARE;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_user = 1;
        event.exclude_kernel = 1;

        if constexpr ( std::is_same_v<P, options::hardware> )
          event.config = static_cast<__u64>(e_type);
      }
      if constexpr ( R == kernel_clock_types::software ) {
        event.type = PERF_TYPE_SOFTWARE;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_user = 1;
        event.exclude_kernel = 1;
        if constexpr ( std::is_same_v<P, options::software> )
          event.config = static_cast<__u64>(e_type);
      }
      if constexpr ( R == kernel_clock_types::tracepoint ) {
        event.type = PERF_TYPE_TRACEPOINT;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_user = 1;
        event.exclude_kernel = 1;
      }
      if constexpr ( R == kernel_clock_types::cache ) {
        event.type = PERF_TYPE_HW_CACHE;
        event.disabled = 1;
        event.pinned = 1;
        event.exclude_user = 1;
        event.exclude_kernel = 1;
        if constexpr ( std::is_same_v<P, options::cache> )
          event.config = static_cast<__u64>(e_type);
      }
    }
  }

  int
  reopen(pid_t pid)
  {
    ::close(e_fd);
    e_fd = static_cast<int>(perf_event_pid(event, pid));     // stop the complaining
    if ( e_fd == -1 )
      return -1;     // ERROR
    return e_fd;
  }
  int
  reopen(void)
  {
    ::close(e_fd);
    e_fd = static_cast<int>(perf_event_this(event));     // stop the complaining
    if ( e_fd == -1 )
      return -1;     // ERROR
    return e_fd;
  }
  int
  open(pid_t pid)
  {
    e_fd = static_cast<int>(perf_event_pid(event, pid));     // stop the complaining
    if ( e_fd == -1 )
      return -1;     // ERROR
    return e_fd;
  }
  int
  open(void)
  {
    e_fd = static_cast<int>(perf_event_this(event));     // stop the complaining
    if ( e_fd == -1 )
      return -1;     // ERROR
    return e_fd;
  }
  template <typename X = T>
  inline __attribute__((always_inline)) void
  start(void)
  {
    ::ioctl(e_fd, PERF_EVENT_IOC_RESET, 0);
    ::ioctl(e_fd, PERF_EVENT_IOC_ENABLE, 0);
  }

  template <typename X = T>
  inline __attribute__((always_inline)) void
  stop(void)
  {
    ::ioctl(e_fd, PERF_EVENT_IOC_DISABLE, 0);
  }
  long long
  read(void)
  {
    long long x = 0;
    ::read(e_fd, &x, sizeof(x));
    return x;
  }
};

enum class system_clocks : clockid_t {
  realtime_set = CLOCK_REALTIME,
  realtime = CLOCK_REALTIME_ALARM,
  realtime_coarse = CLOCK_REALTIME_COARSE,
  taitime = CLOCK_TAI,
  monotonic = CLOCK_MONOTONIC,
  monotonic_coarse = CLOCK_MONOTONIC_COARSE,
  monotonic_raw = CLOCK_MONOTONIC_RAW,
  since_boot = CLOCK_BOOTTIME,
  cputime = CLOCK_PROCESS_CPUTIME_ID,
  cputime_this = CLOCK_THREAD_CPUTIME_ID
};

// clock meant to get general time
template <system_clocks C = system_clocks::realtime> struct system_clock {
  struct timespec time_begin;
  struct timespec time_end;
  system_clock(options::hardware)
  {
    std::memset(&time_begin, 0x0, sizeof(timespec));
    std::memset(&time_end, 0x0, sizeof(timespec));
    if ( ::clock_gettime((clockid_t)C, &time_begin) == -1 )
      throw std::runtime_error("bbench clock failed to get time");
  }
  system_clock()
  {
    std::memset(&time_begin, 0x0, sizeof(timespec));
    std::memset(&time_end, 0x0, sizeof(timespec));
    if ( ::clock_gettime((clockid_t)C, &time_begin) == -1 )
      throw std::runtime_error("bbench clock failed to get time");
  }
  inline __attribute__((always_inline)) void
  start(void)
  {
    if ( ::clock_gettime((clockid_t)C, &time_begin) == -1 )
      throw std::runtime_error("bbench clock failed to get time");
  }
  inline __attribute__((always_inline)) auto
  start_get(void) -> timespec
  {
    if ( ::clock_gettime((clockid_t)C, &time_begin) == -1 )
      throw std::runtime_error("bbench clock failed to get time");
    return time_begin;
  }
  inline __attribute__((always_inline)) void
  stop(void)
  {
    if ( ::clock_gettime((clockid_t)C, &time_end) == -1 )
      throw std::runtime_error("bbench clock failed to get time");
  }
  inline __attribute__((always_inline)) auto
  stop_get(void) -> timespec
  {
    if ( ::clock_gettime((clockid_t)C, &time_end) == -1 )
      throw std::runtime_error("bbench clock failed to get time");
    return time_end;
  }
  inline __attribute__((always_inline)) static auto
  now(void) -> double
  {
    struct timespec t;
    if ( ::clock_gettime((clockid_t)C, &t) == -1 )
      throw std::runtime_error("bbench clock failed to get time");
    auto sec = t.tv_sec;
    auto msec = (t.tv_nsec) / 1000000000;
    return static_cast<double>(sec) + static_cast<double>(msec);
  }
  auto
  read(const timespec &t) -> double
  {
    auto sec = t.tv_sec - time_begin.tv_sec;
    auto msec = (t.tv_nsec - time_begin.tv_nsec) / 1000000000;
    return static_cast<double>(sec) + static_cast<double>(msec);
  }
  auto
  read_ms(const timespec &t) -> double
  {
    auto sec = t.tv_sec - time_begin.tv_sec;
    auto msec = (t.tv_nsec - time_begin.tv_nsec) / 1000000;
    return chrono::milliseconds(static_cast<double>(sec)) + static_cast<double>(msec);
  }
  auto
  read(const timespec &t, const timespec &ts) -> double
  {
    auto sec = t.tv_sec - ts.tv_sec;
    auto msec = (t.tv_nsec - ts.tv_nsec) / 1000000000;
    return static_cast<double>(sec) + static_cast<double>(msec);
  }
  auto
  read_ms(const timespec &t, const timespec &ts) -> double
  {
    auto sec = t.tv_sec - ts.tv_sec;
    auto msec = (t.tv_nsec - ts.tv_nsec) / 1000000;
    return chrono::milliseconds(static_cast<double>(sec)) + static_cast<double>(msec);
  }
  auto
  read(void) -> double
  {
    auto sec = time_end.tv_sec - time_begin.tv_sec;
    auto msec = (time_end.tv_nsec - time_begin.tv_nsec) / 1000000000;
    return static_cast<double>(sec) + static_cast<double>(msec);
  }
  auto
  read_ds(void) -> double
  {
    auto sec = (time_end.tv_sec - time_begin.tv_sec) / 10;
    auto msec = (time_end.tv_nsec - time_begin.tv_nsec) / 100000000;
    return static_cast<double>(sec) + static_cast<double>(msec);
  }
  auto
  read_ms(void) -> double
  {
    auto sec = time_end.tv_sec - time_begin.tv_sec;
    auto msec = (time_end.tv_nsec - time_begin.tv_nsec) / 1000000;
    return chrono::milliseconds(static_cast<double>(sec)) + static_cast<double>(msec);
  }
  auto
  read_us(void) -> double
  {
    auto sec = time_end.tv_sec - time_begin.tv_sec;
    auto msec = (time_end.tv_nsec - time_begin.tv_nsec) / 1000;
    return chrono::microseconds(static_cast<double>(sec)) + static_cast<double>(msec);
  }
  auto
  read_ns(void) -> double
  {
    auto sec = time_end.tv_sec - time_begin.tv_sec;
    auto msec = (time_end.tv_nsec - time_begin.tv_nsec);
    return chrono::nanoseconds(static_cast<double>(sec)) + static_cast<double>(msec);
  }
};

};
