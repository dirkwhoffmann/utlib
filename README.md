# utlib

A lightweight C++20 utility library, extracted from the
[Silicium](https://www.dirkwhoffmann.de) emulator project and maintained here as
a standalone repository.

## What's in it

| Module | Header | Contents |
| --- | --- | --- |
| Types | `utl/types/...` | Fixed-width integer aliases, `UUID`, exception base classes |
| Support | `utl/support.h` | String helpers, bit manipulation macros, byte-order conversion |
| I/O | `utl/io.h` | File and directory helpers, directory syncing, header sniffing, parsing and formatting |
| Storage | `utl/storage.h` | `Buffer`, `RingBuffer`, `Mailbox`, and a ZIP archive wrapper |
| Chrono | `utl/chrono.h` | `Time` and `Clock` |
| Abilities | `utl/abilities.h` | Mixins: `Reflectable`, `Dumpable`, `Loggable`, `Compressible`, ... |
| Wrappers | `utl/wrappers.h` | `Backed`, `Sticky`, `Animated` value wrappers |
| Concurrency | `utl/concurrency.h` | Threading helpers |

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
