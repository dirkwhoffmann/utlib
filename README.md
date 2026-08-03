# utlib

A lightweight C++20 utility library, extracted from the
[Silicium](https://www.dirkwhoffmann.de) emulator project and maintained here as
a standalone repository.

## Projects using utlib

The library is currently used by the following projects:

<a href="https://github.com/dirkwhoffmann/virtualc64" class="image left"><img src="https://github.com/dirkwhoffmann/dirkwhoffmann.github.io/blob/main/images/ad-vc.png" width="100" height="100" alt="Icon" align="middle"></a>&nbsp;&nbsp; **Virtual64**

<a href="https://github.com/dirkwhoffmann/peddle" class="image left"><img src="https://github.com/dirkwhoffmann/dirkwhoffmann.github.io/blob/main/images/ad-pe.png" width="100" height="100" alt="Icon" align="middle"></a>&nbsp;&nbsp; **Peddle**

<a href="https://github.com/dirkwhoffmann/vamiga" class="image left"><img src="https://github.com/dirkwhoffmann/dirkwhoffmann.github.io/blob/main/images/ad-va.png" width="100" height="100" alt="Icon" align="middle"></a>&nbsp;&nbsp; **vAmiga**

<a href="https://github.com/dirkwhoffmann/moira" class="image left"><img src="https://github.com/dirkwhoffmann/dirkwhoffmann.github.io/blob/main/images/ad-mo.png" width="100" height="100" alt="Icon" align="middle"></a>&nbsp;&nbsp; **Moira**
  
<a href="https://github.com/dirkwhoffmann/retrovault" class="image left"><img src="https://github.com/dirkwhoffmann/dirkwhoffmann.github.io/blob/main/images/ad-vault.png" width="100" height="100" alt="Icon" align="middle"></a>&nbsp;&nbsp; **RetroVault**

## Note

Third-party code lives in `thirdparty/` and keeps its own licensing
(`kuba-zip`/`miniz`, `lz4`, `nlohmann/json`, `cpp-httplib`).

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
