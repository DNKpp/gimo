```
   █████████  █████ ██████   ██████    ███████   
  ███▒▒▒▒▒███▒▒███ ▒▒██████ ██████   ███▒▒▒▒▒███ 
 ███     ▒▒▒  ▒███  ▒███▒█████▒███  ███     ▒▒███
▒███          ▒███  ▒███▒▒███ ▒███ ▒███      ▒███
▒███    █████ ▒███  ▒███ ▒▒▒  ▒███ ▒███      ▒███
▒▒███  ▒▒███  ▒███  ▒███      ▒███ ▒▒███     ███ 
 ▒▒█████████  █████ █████     █████ ▒▒▒███████▒  
  ▒▒▒▒▒▒▒▒▒  ▒▒▒▒▒ ▒▒▒▒▒     ▒▒▒▒▒    ▒▒▒▒▒▒▒   
```

<b>G</b>eneric <b>I</b>nterchangeable <b>M</b>onadic <b>O</b>perations

---

## Table of Contents

* [Introduction](#introduction)
    * [Genericity](#genericity)
    * [Interchangeability](#interchangeability)
    * [Zero Cost Abstraction](#zero-cost-abstraction)
* [Integration](#integration)
    * [Optional Extensions](#optional-extensions)
    * [Portability](#portability)
    * [CMake](#cmake)
    * [Single-Header](#single-header)

---

<a name="introduction"></a>
## Introduction

*gimo* is a small C++20 library that provides reusable monadic operations as free functions.
Although C++23 introduces monadic operations for std::optional (and std::expected) as member functions,
their real-world usability is still quite limited.

In the following sections, I’ll explain the general idea behind *gimo*.

<a name="genericity"></a>
### Genericity

The C++ standard library is well known for its generic algorithms,
which decouple concrete container implementations from the operations performed on them.
For example, there is no `std::vector::find`,
because `std::find` (and, since C++20, `std::ranges::find`) already solves the problem generically.

There is no need to reimplement the same algorithm for each container
if the actual goal is simply to perform a linear search.
Doing so would only result in more code, without any real benefit.

This is exactly what I’d like to have for monadic operations.
Therefore, gimo provides the following algorithms for all *nullable* types out of the box:

- `gimo::and_then`
- `gimo::or_else`
- `gimo::transform`

Additionally, for *expected-like* types, *gimo* offers `gimo::transform_error`.

Providing these operations as free functions also enables customization.
Users can add their own algorithms where needed, without being constrained by member functions.

<a name="interchangeability"></a>
### Interchangeability

C++ has a long history, and many codebases already use their own optional- or expected-like types.
Each of these typically has to implement monadic operations on its own.
Even when such operations exist, it is currently impossible to mix different vocabulary types within a single pipeline.

*gimo* makes it possible to build pipelines that involve different closure types.
Consider a case where we want to use both [std::optional](https://en.cppreference.com/w/cpp/utility/optional.html) and
[nonstd-lite/optional-lite](https://github.com/nonstd-lite/optional-lite) in the same pipeline.
Assuming that integration for both types is in place, the following just works:

```cpp
auto const result = gimo::apply(
    std::optional{1337},
    gimo::and_then([](int const v) { 
        return nonstd::optional<std::string>{std::to_string(v)};
    }));
```

See the full example on [godbolt.org](https://godbolt.org/z/ETWxbnhce).

<a name="zero-cost-abstraction"></a>
### Zero Cost Abstraction

This section describes more of a nice side effect,
but it’s something I like to point out in discussions,
because it supports the idea that monadic operations should be free functions.

Consider the following snippet, where several transform operations are composed in a single pipeline:
```cpp
int const result = std::optional{1337}
    .transform([](int const v) { return static_cast<float>(v); })
    .transform([](float const v) { return std::to_string(v); });
```

The final result is not particularly surprising.
However, if we look a bit closer at what happens under the hood, some inefficiencies become apparent.

We start with a non-empty `std::optional` and call transform.
This operation checks whether the optional contains a value and then applies the function.
The second transform has to perform the same emptiness check again,
even though the first transform can never change that invariant.
Each operation is fully isolated, and no information is propagated between them.

Now let’s look at the same example using gimo:
```cpp
int const result = gimo::apply(
    std::optional{1337},
    gimo::transform([](int const v) { return static_cast<float>(v); })
    | gimo::transform([](float const v) { return std::to_string(v); }));
```

Here, the first `transform` still needs to evaluate emptiness.
However, once that information is known, it can be forwarded to the next operation.

In fact, this closely resembles the following rewritten code,
which users may have in mind when constructing the pipeline.

```cpp
std::optional opt{1337};
if (opt)
{
    auto const f = static_cast<float>(*opt);
    return std::to_string(f);
}

return std::nullopt;
```

Note that these optimizations depend on the specific algorithm, as each has different semantic properties.
For instance, `or_else` preserves the emptiness state when the nullable holds a value,
allowing this information to be propagated.
Since the fallback action is not guaranteed to produce a non-empty nullable,
the next operation must still perform its own check.

<a name="integration"></a>

## Integration

*gimo* is a header-only library, allowing users to easily access all features by simply including the `gimo.hpp` header.

<a name="optional_extensions"></a>

### Optional Extensions

The *gimo* core is a type-agnostic framework for free-standing monadic operations.
Support for specific types is available via an opt-in model through the `gimo_ext` directory.
These headers provide the necessary trait specializations to adapt existing types to the gimo pipeline.
For instance, to enable support for `std::optional`, simply include the corresponding adapter:

```cpp
#include <gimo_ext/StdOptional.hpp>
```

Currently, the following extensions are provided:

- [std::optional](https://github.com/DNKpp/gimo/blob/main/include/gimo_ext/StdOptional.hpp)
- [std::expected](https://github.com/DNKpp/gimo/blob/main/include/gimo_ext/StdExpected.hpp)
- [raw-pointers](https://github.com/DNKpp/gimo/blob/main/include/gimo_ext/RawPointer.hpp)
- [std::unique_ptr](https://github.com/DNKpp/gimo/blob/main/include/gimo_ext/StdUniquePtr.hpp)
- [std::shared_ptr](https://github.com/DNKpp/gimo/blob/main/include/gimo_ext/StdSharedPtr.hpp)

<a name="portability"></a>

### Portability

The *gimo* framework is architected for broad compatibility with any C++20-conforming compiler,
maintaining strict independence from underlying hardware architectures or operating systems.
The library has been verified across various environments, including Windows, macOS,
and major Linux distributions (Ubuntu, Debian) on both `x86_64` and `x86_32` platforms.

#### Minimum Toolchain Requirements

The library is verified to work with the following compiler versions.
While older versions supporting C++20 may work, these represent the minimums confirmed by the current test suite:

- **GCC:** 10.2+
- **Clang:** 16+
- **MSVC:** v143+ (Visual Studio 2022)

<a name="cmake"></a>

### CMake

The integration into a CMake project is straight-forward.

```cmake
target_link_libraries(<your_target_name> PUBLIC gimo::gimo)
```

Users can either select a commit in the **main** branch or a version tag and utilize the CMake ``FetchContent`` module:

```cmake
include(FetchContent)

FetchContent_Declare(gimo
    VERSION 0.1.0 # or GIT_TAG <commit_hash> 
    GIT_REPOSITORY https://github.com/DNKpp/gimo
)

FetchContent_MakeAvailable(gimo)
# do not forget linking via target_link_libraries as shown above
```

As an alternative, I recommend using [CPM](https://github.com/cpm-cmake/CPM.cmake),
which is a convenient wrapper based on the ``FetchContent`` feature:

```cmake
include(CPM.cmake) # or include(get_cpm.cmake)

CPMAddPackage("gh:DNKpp/gimo@0.1.0") # or gh:DNKpp/gimo#<commit_hash>
# do not forget linking via target_link_libraries as shown above
```

<a name="single-header"></a>

### Single-Header

For convenience, an amalgamated version is available
via [gimo-amalgamate.hpp](https://github.com/DNKpp/gimo/blob/amalgamate/gimo-amalgamate.hpp).
This file tracks the current state of the main branch as a single, self-contained header.
Please note that extensions found in `gimo_ext` are excluded from this file and must be included separately.


