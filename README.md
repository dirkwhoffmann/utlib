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

## Using it in another project

The tests are only built when utlib is the top-level project, so it can be
embedded directly:

```cmake
add_subdirectory(utlib)
target_link_libraries(my_target PRIVATE utlib)
```

## License

utlib is licensed under the Mozilla Public License v2. See [LICENSE](LICENSE).
