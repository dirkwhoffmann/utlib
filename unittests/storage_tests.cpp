// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "main.h"

#include <algorithm>

TEST_CASE("Buffer") {

    SUBCASE("A default buffer is empty") {

        Buffer<u8> buf;

        CHECK(buf.size == 0);
        CHECK(buf.ptr == nullptr);
    }

    SUBCASE("Sized construction") {

        Buffer<u8> buf(16);

        CHECK(buf.size == 16);
        REQUIRE(buf.ptr != nullptr);
    }

    SUBCASE("Construction with a fill value") {

        Buffer<u8> buf(8, u8(0xAB));

        REQUIRE(buf.size == 8);
        for (isize i = 0; i < buf.size; i++) CHECK(buf[i] == 0xAB);
    }

    SUBCASE("Construction from a string") {

        Buffer<u8> buf(string("hello"));

        REQUIRE(buf.size == 5);
        CHECK(buf[0] == 'h');
        CHECK(buf[4] == 'o');
    }

    SUBCASE("Construction from raw memory") {

        const u8 src[] = { 1, 2, 3 };
        Buffer<u8> buf(src, 3);

        REQUIRE(buf.size == 3);
        CHECK(buf[0] == 1);
        CHECK(buf[2] == 3);
    }

    SUBCASE("Construction from a vector") {

        std::vector<u8> vec { 9, 8, 7 };
        Buffer<u8> buf(vec);

        REQUIRE(buf.size == 3);
        CHECK(buf[0] == 9);
        CHECK(buf[2] == 7);
    }

    SUBCASE("Copies are independent") {

        Buffer<u8> a(4, u8(1));
        Buffer<u8> b(a);

        REQUIRE(b.size == 4);
        b[0] = 42;

        CHECK(a[0] == 1);
        CHECK(b[0] == 42);
    }

    SUBCASE("Reading a file into a buffer") {

        TempDir dir("buffer");
        auto path = dir / "data.bin";
        writeFile(path, "abcdef");

        Buffer<u8> buf(path);

        REQUIRE(buf.size == 6);
        CHECK(buf[0] == 'a');
        CHECK(buf[5] == 'f');
    }
}

TEST_CASE("RingBuffer / Array") {

    SUBCASE("A fresh array is empty") {

        Array<int, 4> array;

        CHECK(array.isEmpty());
        CHECK_FALSE(array.isFull());
        CHECK(array.count() == 0);
        CHECK(array.cap() == 4);
        CHECK(array.free() == 4);
    }

    SUBCASE("Writing advances the count") {

        Array<int, 4> array;

        array.write(10);
        array.write(20);

        CHECK(array.count() == 2);
        CHECK(array.free() == 2);
        CHECK_FALSE(array.isEmpty());
        CHECK(array[0] == 10);
        CHECK(array[1] == 20);
    }

    SUBCASE("Filling to capacity") {

        Array<int, 3> array;

        array.write(1);
        array.write(2);
        array.write(3);

        CHECK(array.isFull());
        CHECK(array.free() == 0);
        CHECK(array.fillLevel() == doctest::Approx(1.0));
    }

    SUBCASE("clear resets the write position") {

        Array<int, 4> array;

        array.write(1);
        array.write(2);
        array.clear();

        CHECK(array.isEmpty());
        CHECK(array.count() == 0);
    }

    SUBCASE("Elements are assignable through operator[]") {

        Array<int, 4> array;

        array.write(1);
        array[0] = 99;

        CHECK(array[0] == 99);
    }

    SUBCASE("fillLevel tracks occupancy") {

        Array<int, 4> array;

        CHECK(array.fillLevel() == doctest::Approx(0.0));
        array.write(1);
        array.write(2);
        CHECK(array.fillLevel() == doctest::Approx(0.5));
    }
}

TEST_CASE("ZipArchive") {

    TempDir dir("zip");
    auto root = dir / "tree";
    auto archive = dir / "test.zip";

    writeFile(root / "a.txt", "alpha");
    writeFile(root / "sub" / "b.txt", "beta");

    SUBCASE("Writing and listing") {

        {
            ZipArchive zip(archive, 'w');
            zip.write(std::vector<fs::path> { root / "a.txt", root / "sub" / "b.txt" }, root);
        }

        REQUIRE(fs::exists(archive));

        ZipArchive zip(archive);
        auto items = zip.listFiles();

        CHECK(items.size() == 2);
        CHECK(std::find(items.begin(), items.end(), "a.txt") != items.end());
        CHECK(std::find(items.begin(), items.end(), "sub/b.txt") != items.end());
    }

    SUBCASE("Uncompressing a single entry into memory") {

        {
            ZipArchive zip(archive, 'w');
            zip.write(std::vector<fs::path> { root / "a.txt" }, root);
        }

        ZipArchive zip(archive);
        auto data = zip.uncompress("a.txt");

        CHECK(string(data.begin(), data.end()) == "alpha");
    }

    SUBCASE("Uncompressing the whole archive to disk") {

        {
            ZipArchive zip(archive, 'w');
            zip.write(std::vector<fs::path> { root / "a.txt", root / "sub" / "b.txt" }, root);
        }

        auto out = dir / "out";
        fs::create_directories(out);

        ZipArchive zip(archive);
        zip.uncompressAll(out);

        CHECK(readFile(out / "a.txt") == "alpha");
        CHECK(readFile(out / "sub" / "b.txt") == "beta");
    }

    SUBCASE("A round trip preserves content exactly") {

        const string payload = "The quick brown fox\n\0with an embedded NUL";

        writeFile(root / "payload.bin", payload);

        {
            ZipArchive zip(archive, 'w');
            zip.write(std::vector<fs::path> { root / "payload.bin" }, root);
        }

        ZipArchive zip(archive);
        auto data = zip.uncompress("payload.bin");

        CHECK(string(data.begin(), data.end()) == payload);
    }

    SUBCASE("Opening a non-existent archive fails") {

        CHECK_THROWS(ZipArchive(dir / "missing.zip"));
    }
}
