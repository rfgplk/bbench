<div align="center">
  <img src="https://github.com/user-attachments/assets/9d07b433-174a-4a63-a5f7-404c0224b0f4" alt="bbench_logo_384" width="300"/>
  
# bbench
### a lightweight Linux performance event benchmarking library
</div>

bbench is a *lightweight*, **intuitive**, and ***logical*** benchmarking library, written in C++20. 

The goal of this library is to provide simplistic yet useful functions for *timing and profiling* performance **critical** code. The library uses specific performance monitoring facilities (via the kernel) to extract all the relevant information you would ever need without being overbearing. Most bbench code is evaluated and instantiated at compile time, meaning this is practically the *lightest (and smallest) possible implementation* of benchmarking functionality. Has minimal (almost non-existent) runtime overhead. It can be used either as a library or a compiled binary (in case you would like to benchmark precompiled code). 

To compile from source run `ninja bbench` or `ninja btime`. 


bbench is specifically designed for Linux, as such other operating systems and kernels are entirely unsupported for the time being.

## General Usage
```cpp
// Primary functions
benchmark_t  bbench::benchmark                 (Func, Arguments...);
benchmark_t  bbench::benchmark<time_unit>      (Func, Arguments...);
benchmark_t  bbench::benchmark_bin             (path_to_binary, argv...);
     double  bbench::bench                     (Func, Arguments...);
     double  bbench::bench<time_unit>          (Func, Arguments...);
  long long  bbench::cpu_bench                 (Func, Arguments...);
  long long  bbench::cpu_bench<target_type>    (Func, Arguments...);
     double  bbench::bench_bin                 (path_to_binary, argv...);
     double  bbench::bench_bin<time_unit>      (path_to_binary, argv...);
  long long  bbench::cpu_bench_bin             (path_to_binary, argv...);
  long long  bbench::cpu_bench_bin<target_type>(path_to_binary, argv...);
// path_to_binary can be either a const char* or any string-like object, same as argv
// Func can be any function, Arguments can be of any type, ... denotes a variadic argument
```
## Specific Usage

```cpp
std::vector<double> bbench::bench<time_unit> (Funcs...);
std::vector<double> bbench::bench_repeat<N>  (Func, Arguments...);
```

### Example 0
```cpp
// call benchmark with a desired function and given time resolution to collect traced data
benchmark_t b = bbench::benchmark<bbench::time_resolution::us>  (parse_strings, "test", "hello", "world");
// benchmark can be called with no template argument, in which case the resolution is assumed to be in us

// benchmark primarily returns struct benchmark_t which holds all the traced data
struct benchmark_t {
  double time;     // heh
  long long cycles;
  long long instructions;
  long long cache_misses;
  long long total_branches;
  long long branch_misses;
  long long total_cycles;
  long long cpu_time;
  long long context_switches;
  long long migrations;

  long long l1_cache;
  long long l1t_cache;
  long long ll_cache;
  long long access;
  long long bpu;
};

```


### Example A
```cpp
#include "src/bench.hpp"

auto bmx =   bbench::benchmark<bbench::time_resolution::ns>(my_function, arg1, arg2, arg3);
std::cout << per_cycle(bmx) << std::endl;
std::cout << per_instruction(bmx) << std::endl;
std::cout << miss_percent(bmx) << std::endl;
std::cout << cycles_per_instruction(bmx) << std::endl;
```


### Example B
```cpp
#include "src/bench.hpp"

// Exhaustive list of bench targets
// Each call collects information for that target only
// bbench provides k_* and a_* prefixed targets for kernel tracing but depending 
// on perf_event_paranoid you may need root capabilities
// check sysctl kernel.perf_event_paranoid and capabilities(7)
auto s = bbench::cpu_bench<bbench::hardware_cycles>       (my_function, arg1, arg2, arg3);
auto s = bbench::cpu_bench<bbench::hardware_instructions> (my_function, arg1, arg2, arg3);
auto s = bbench::cpu_bench<bbench::cache_misses>          (my_function, arg1, arg2, arg3);
auto s = bbench::cpu_bench<bbench::branches>              (my_function, arg1, arg2, arg3);
auto s = bbench::cpu_bench<bbench::branch_misses>         (my_function, arg1, arg2, arg3);
auto s = bbench::cpu_bench<bbench::total_cycles>          (my_function, arg1, arg2, arg3);
auto s = bbench::cpu_bench<bbench::cpu_time>              (my_function, arg1, arg2, arg3);
auto s = bbench::cpu_bench<bbench::context_switches>      (my_function, arg1, arg2, arg3);
auto s = bbench::cpu_bench<bbench::proc_migrations>       (my_function, arg1, arg2, arg3);
auto s = bbench::cpu_bench<bbench::level1d>               (my_function, arg1, arg2, arg3);
auto s = bbench::cpu_bench<bbench::level1t>               (my_function, arg1, arg2, arg3);
auto s = bbench::cpu_bench<bbench::llcache>               (my_function, arg1, arg2, arg3);
auto s = bbench::cpu_bench<bbench::cache_node>            (my_function, arg1, arg2, arg3);

// get cycles spent in kernel space
auto s = bbench::cpu_bench<bbench::k_hardware_cycles>     (my_function, arg1, arg2, arg3); 
```

### Example C
```cpp
#include "src/bench.hpp"

// Time Resolutions
// sec, ds, ms, us, ns
// long names are equivalent, ie seconds, deciseconds, milliseconds...
double d = bbench::bench<bbench::time_resolution::ns>     (my_function, arg1, arg2, arg3); 
// get time in ns
double d = bbench::bench<bbench::time_resolution::ms>     (my_function, arg1, arg2, arg3); 
// get time in ms
```

### Example D
```cpp
#include "src/bench.hpp"

// To bench a precompiled binary simply call any *_bin suffixed function
double f = bbench::bench_bin("/bin/gcc", "a.cc"); // equivalent to typing /bin/gcc a.cc into a shell
double f = bbench::bench_bin("/bin/whoami");
double f = bbench::bench_bin("/bin/top");
double f = bbench::bench_bin("/bin/cal");
// path can be absolute or relative, strings following the first argument correspond to argv which the binary will receive
// NOTE: if a relative path is provided, the binary must be present at that exact location
// bbench DOES NOT check your local path, NOR does it invoke a shell
```

## Comparison with perf stat
Tested against perf, sample output for both executables.
```
Performance counter stats for './bin/bbench /bin/whoami':

              4.40 msec task-clock:u                     #    0.949 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
               211      page-faults:u                    #   47.939 K/sec                     
         2,687,746      cycles:u                         #    0.611 GHz                         (95.77%)
         3,712,006      instructions:u                   #    1.38  insn per cycle              (76.46%)
           500,349      branches:u                       #  113.679 M/sec                     
            15,581      branch-misses:u                  #    3.11% of all branches           

       0.004640276 seconds time elapsed

       0.001053000 seconds user
       0.003764000 seconds sys


 Performance counter stats for 'perf stat whoami':

             11.79 msec task-clock:u                     #    0.952 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
             1,066      page-faults:u                    #   90.430 K/sec                     
        12,829,132      cycles:u                         #    1.088 GHz                       
        15,285,441      instructions:u                   #    1.19  insn per cycle            
         3,170,034      branches:u                       #  268.917 M/sec                     
           109,362      branch-misses:u                  #    3.45% of all branches           

       0.012379547 seconds time elapsed

       0.005142000 seconds user
       0.007240000 seconds sys

// bbench output
./bin/bbench /bin/whoami
Total time elapsed:   1241
Cycles Spent:         480439
Total Instructions:   384592
Total Branches:       88495
Branch Misses:        5397
Total CPU Cycles:     840030
Total CPU Time Spent: 1096229
Context Switches:     0
Core Migrations:      0
Cache Misses:         2901
L1 Cache:             0
L1T Cache:            0
Last Level Cache:     0
Cache Accesses:       0
Branch Predictions:   0

Performance counter stats for '/bin/whoami':

              0.76 msec task-clock:u                     #    0.654 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
                70      page-faults:u                    #   91.829 K/sec                     
           574,814      cycles:u                         #    0.754 GHz                       
           386,162      instructions:u                   #    0.67  insn per cycle            
            88,976      branches:u                       #  116.722 M/sec                     
             5,315      branch-misses:u                  #    5.97% of all branches           

       0.001164855 seconds time elapsed

       0.001215000 seconds user
       0.000000000 seconds sys


```



## Installation

bbench is a header only library. Just copy all files from `src/` and include `src/bench.hpp` into your project.To install bbench and btime simply run `ninja btime` and `ninja bbench` and copy the files to your desired location.


## TODO
- [ ] add direct __rdtsc functionality
- [ ] develop benchmarking suites
- [ ] write per core and per socket specific tracing core
- [ ] develop benchmarking endpoints for all perf_event code (currently in bbench, but inaccessible easily)
- [ ] write go \& python wrappers
- [ ] C bindings

## Misc
no external dependencies, requires C++20, tested with g++.
Requires at least Linux 2.6.
bbench needs the following headers (and associated .so) to compile properly (normally are distributed with every standard Linux/C++ distribution).

```cpp
#include <array>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <linux/perf_event.h>
#include <memory>
#include <queue>
#include <sched.h>
#include <signal.h>
#include <spawn.h>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <type_traits>
#include <unistd.h>
#include <utility>
#include <variant>
#include <vector>
```

## License
Licensed under the Boost Software License.
