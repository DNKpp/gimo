//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/Pipeline.hpp"
#include "gimo/algorithm/AndThen.hpp"
#include "gimo_ext/StdOptional.hpp"

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

namespace
{
    auto make_setup(std::string prefix, unsigned const seed)
    {
        std::optional<int> opt =
            seed % 2 == 0 ? std::nullopt : std::optional<int>{1337};
        prefix += " - with ";
        prefix += !opt
                    ? "nullopt"
                    : "optional{1337}";

        return std::make_tuple(std::move(opt), std::move(prefix));
    }

    void StdOptionalAndThenChain(ankerl::nanobench::Bench& bench, unsigned const seed)
    {
        auto const [opt, name] = make_setup("std::optional::and_then", seed);

        bench.run(
            name.data(),
            [&] {
                auto r = opt.and_then([](auto const& x) { return std::optional{static_cast<float>(x) + 1}; })
                             .and_then([](float const x) { return std::optional{static_cast<short>(x) + 1}; })
                             .and_then([](short const x) { return std::optional{static_cast<float>(x) + 1}; });

                ankerl::nanobench::doNotOptimizeAway(r);
            });
    }

    void GimoAndThenChain(ankerl::nanobench::Bench& bench, unsigned const seed)
    {
        auto const [opt, name] = make_setup("gimo::and_then", seed);

        bench.run(
            name.data(),
            [&] {
                auto const r = gimo::apply(
                    opt,
                    gimo::and_then([](auto const& x) { return std::optional{static_cast<float>(x) + 1}; })
                        | gimo::and_then([](float const x) { return std::optional{static_cast<short>(x) + 1}; })
                        | gimo::and_then([](short const x) { return std::optional{static_cast<float>(x) + 1}; }));

                ankerl::nanobench::doNotOptimizeAway(r);
            });
    }
}

int main()
{
    [[maybe_unused]] auto const seed = std::random_device{}();

    ankerl::nanobench::Bench bench{};
    bench.relative(true)
        .warmup(1000)
        .minEpochIterations(100'000'000)
        .performanceCounters(true);

    StdOptionalAndThenChain(bench, 1);
    GimoAndThenChain(bench, 1);
    StdOptionalAndThenChain(bench, 2);
    GimoAndThenChain(bench, 2);
}
