# utlib

A lightweight C++20 utility library.

## About

utlib is a personal, utility-driven codebase born out of the specific requirements of my emulator development projects. It is not designed or packaged as a comprehensive, general-purpose library for the broader public. Instead, it serves as a curated collection of helpers, custom data structures, and foundational tools tailored to my own software architecture.

This repository acts as the master copy of the library. By keeping it in a dedicated, standalone space rather than embedding copies inside each individual project, I can centralize updates, maintain a single source of truth, and push improvements seamlessly across all dependent applications.

## Projects using utlib

The library is currently used by the following projects:

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/dirkwhoffmann/virtualc64">
        <img src="https://github.com/dirkwhoffmann/dirkwhoffmann.github.io/blob/main/images/ad-vc.png" width="80" height="80" alt="Virtual64 Icon">
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/dirkwhoffmann/peddle">
        <img src="https://github.com/dirkwhoffmann/dirkwhoffmann.github.io/blob/main/images/ad-pe.png" width="80" height="80" alt="Peddle Icon">
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/dirkwhoffmann/vamiga">
        <img src="https://github.com/dirkwhoffmann/dirkwhoffmann.github.io/blob/main/images/ad-va.png" width="80" height="80" alt="vAmiga Icon">
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/dirkwhoffmann/moira">
        <img src="https://github.com/dirkwhoffmann/dirkwhoffmann.github.io/blob/main/images/ad-mo.png" width="80" height="80" alt="Moira Icon">
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/dirkwhoffmann/retrovault">
        <img src="https://github.com/dirkwhoffmann/dirkwhoffmann.github.io/blob/main/images/ad-vault.png" width="80" height="80" alt="RetroVault Icon">
      </a>
    </td>
  </tr>
  <tr>
    <td align="center"><strong><a href="https://github.com/dirkwhoffmann/virtualc64">Virtual64</a></strong></td>
    <td align="center"><strong><a href="https://github.com/dirkwhoffmann/peddle">Peddle</a></strong></td>
    <td align="center"><strong><a href="https://github.com/dirkwhoffmann/vamiga">vAmiga</a></strong></td>
    <td align="center"><strong><a href="https://github.com/dirkwhoffmann/moira">Moira</a></strong></td>
    <td align="center"><strong><a href="https://github.com/dirkwhoffmann/retrovault">RetroVault</a></strong></td>
  </tr>
</table>

## Building

```bash
cmake -B build
cmake --build build
```

zlib is picked up automatically if present and enables the `gzip`/`gunzip`
codecs in `Compressible`. It is optional: without it those two throw and
everything else, `lz4` and `rle2` included, is unaffected.

## Running the tests

The test suite uses [doctest](https://github.com/doctest/doctest), vendored as a
single header in `unittests/`.

```bash
ctest --test-dir build --output-on-failure
```

Or run the binary directly, which accepts doctest's own options — for example
`--test-case="UUID"` to run one case, or `--list-test-cases`:

```bash
./build/unittests/unittester
```

## Code coverage

Coverage is measured with Clang's source-based instrumentation (or gcov under
GCC) and is off by default, since the instrumentation changes what the compiler
emits and slows the binary down.

```bash
cmake -B build-cov -DCMAKE_BUILD_TYPE=Debug -DUTLIB_COVERAGE=ON
cmake --build build-cov --target coverage
```

The target runs the test suite, prints a per-file summary, and writes a
browsable HTML report to `build-cov/coverage/html/index.html`. The tests
themselves and `thirdparty/` are excluded from the figures; override with
`-DUTLIB_COVERAGE_EXCLUDE=<regex>`.

Build `Debug`: an optimised build inlines and folds code away, so the line
counts stop matching the source. The target warns if the build type is anything
else.

Requires `llvm-profdata` and `llvm-cov` (found via `xcrun` on macOS, otherwise
on `PATH`), or `gcovr` when building with GCC.

## Using it in another project

The tests are only built when utlib is the top-level project, so it can be
embedded directly:

```cmake
add_subdirectory(utlib)
target_link_libraries(my_target PRIVATE utlib)
```

## License

utlib (excluding the code contained in the *thirdparty/* directory) is licensed 
under the Mozilla Public License v2.0. See [LICENSE](LICENSE) for details.

Third-party components located in *thirdparty/* retain their original licenses.

