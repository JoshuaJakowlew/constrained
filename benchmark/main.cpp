#include <chrono>
#include <cstdint>
#include <string>
#include <cstdlib>
#include <nanobench.h>
#include <constrained_type.hpp>

#include <non_null.hpp>

int main()
{
    auto bench_non_null = nb::Bench();
    bench_non_null
        .warmup(100)
        .epochs(100)
        .minEpochIterations(100'000)
        .run("manual nullable non_null", []{
            manual::nullable::non_null::run<int>();
        })
        .run("constrained nullable non_null", []{
            constrained::nullable::non_null::run<int>();
        })
        .minEpochIterations(10'000)
        .run("manual throwing non_null", []{
            manual::throwing::non_null::run<int>();
        })
        .run("constrained throwing non_null", []{
            constrained::throwing::non_null::run<int>();
        });
}
