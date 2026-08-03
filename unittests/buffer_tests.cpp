// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "main.h"

#include <sstream>

TEST_CASE("Buffer: sizing") {

    SUBCASE("alloc and dealloc") {

        Buffer<u8> buf;
        CHECK(buf.empty());
        CHECK_FALSE(bool(buf));

        buf.alloc(10);
        CHECK(buf.size == 10);
        CHECK_FALSE(buf.empty());
        CHECK(bool(buf));

        buf.dealloc();
        CHECK(buf.empty());
        CHECK(buf.ptr == nullptr);
    }

    SUBCASE("bytesize accounts for the element type") {

        Buffer<u8> bytes(8);
        Buffer<u32> words(8);

        CHECK(bytes.bytesize() == 8);
        CHECK(words.bytesize() == 32);
    }

    SUBCASE("resize keeps the leading elements") {

        Buffer<u8> buf(4, u8(7));

        buf.resize(8);
        REQUIRE(buf.size == 8);
        for (isize i = 0; i < 4; i++) CHECK(buf[i] == 7);

        buf.resize(2);
        REQUIRE(buf.size == 2);
        CHECK(buf[0] == 7);
        CHECK(buf[1] == 7);
    }

    SUBCASE("resize can pad the new elements") {

        Buffer<u8> buf(2, u8(1));
        buf.resize(5, u8(9));

        REQUIRE(buf.size == 5);
        CHECK(buf[0] == 1);
        CHECK(buf[1] == 1);
        CHECK(buf[2] == 9);
        CHECK(buf[4] == 9);
    }

    SUBCASE("strip removes from the front") {

        Buffer<u8> buf(std::vector<u8> { 1, 2, 3, 4, 5 });

        buf.strip(2);

        REQUIRE(buf.size == 3);
        CHECK(buf[0] == 3);
        CHECK(buf[2] == 5);
    }
}

TEST_CASE("Buffer: contents") {

    SUBCASE("clear fills the whole buffer by default") {

        Buffer<u8> buf(4, u8(1));
        buf.clear(u8(0));

        for (isize i = 0; i < 4; i++) CHECK(buf[i] == 0);
    }

    SUBCASE("clear honours an offset") {

        Buffer<u8> buf(4, u8(1));
        buf.clear(u8(0), 2);

        CHECK(buf[0] == 1);
        CHECK(buf[1] == 1);
        CHECK(buf[2] == 0);
        CHECK(buf[3] == 0);
    }

    SUBCASE("clear honours an offset and a length") {

        Buffer<u8> buf(5, u8(1));
        buf.clear(u8(9), 1, 2);

        CHECK(buf[0] == 1);
        CHECK(buf[1] == 9);
        CHECK(buf[2] == 9);
        CHECK(buf[3] == 1);
    }

    SUBCASE("copy writes the contents out") {

        Buffer<u8> buf(std::vector<u8> { 1, 2, 3, 4 });
        u8 out[4] = { 0, 0, 0, 0 };

        buf.copy(out);

        CHECK(out[0] == 1);
        CHECK(out[3] == 4);
    }

    SUBCASE("copy honours an offset and a length") {

        Buffer<u8> buf(std::vector<u8> { 1, 2, 3, 4 });
        u8 out[2] = { 0, 0 };

        buf.copy(out, 1, 2);

        CHECK(out[0] == 2);
        CHECK(out[1] == 3);
    }

    SUBCASE("Elements are writable through operator[]") {

        Buffer<u8> buf(3, u8(0));
        buf[1] = 42;

        CHECK(buf[0] == 0);
        CHECK(buf[1] == 42);
    }

    SUBCASE("patch substitutes a byte sequence") {

        Buffer<u8> buf(std::string("aXXb"));

        buf.patch("XX", "YY");

        CHECK(buf[0] == 'a');
        CHECK(buf[1] == 'Y');
        CHECK(buf[2] == 'Y');
        CHECK(buf[3] == 'b');
    }
}

TEST_CASE("Buffer: mixed-in abilities") {

    Buffer<u8> buf(std::string("utlib"));

    SUBCASE("It hashes like the raw bytes") {

        const u8 raw[] = { 'u', 't', 'l', 'i', 'b' };

        CHECK(buf.fnv32() == Hashable::fnv32(raw, 5));
        CHECK(buf.crc32() == Hashable::crc32(raw, 5));
    }

    SUBCASE("It dumps its own contents") {

        std::ostringstream os;
        buf.memDump(os);

        CHECK(os.str().find("utlib") != std::string::npos);
    }

    SUBCASE("byteView exposes the same bytes") {

        auto view = buf.byteView();
        CHECK(view.size() == 5);
    }
}

TEST_CASE("Buffer: compression") {

    // Repetitive data, so every codec has something to work with
    std::string payload;
    for (int i = 0; i < 200; i++) payload += "utlib-";

#ifdef USE_ZLIB
    SUBCASE("gzip round-trips") {

        Buffer<u8> buf(payload);
        auto original = buf.size;

        buf.gzip();
        CHECK(buf.size != original);

        buf.gunzip(0, original);
        REQUIRE(buf.size == original);
        CHECK(std::string((char *)buf.ptr, size_t(buf.size)) == payload);
    }
#else
    SUBCASE("gzip reports that the build has no zlib") {

        /* Compressible::gzip is compiled out unless USE_ZLIB is defined, and
         * this project does not define it -- Silicium does, which is why the
         * codec works there. Pinned down so the difference is visible rather
         * than surfacing as a puzzling exception.
         */
        Buffer<u8> buf(payload);
        CHECK_THROWS(buf.gzip());
    }
#endif

    SUBCASE("lz4 round-trips") {

        Buffer<u8> buf(payload);
        auto original = buf.size;

        buf.lz4();
        buf.unlz4(0, original);

        REQUIRE(buf.size == original);
        CHECK(std::string((char *)buf.ptr, size_t(buf.size)) == payload);
    }

    SUBCASE("rle2 round-trips") {

        // Long runs, which is what run-length encoding is for
        Buffer<u8> buf(std::string(500, 'x'));
        auto original = buf.size;

        buf.rle2();
        buf.unrle2(0, original);

        REQUIRE(buf.size == original);
        CHECK(buf[0] == 'x');
        CHECK(buf[499] == 'x');
    }
}

TEST_CASE("ZipError") {

    SUBCASE("Each fault has a name") {

        CHECK(string(ZipError(ZipError::INVALID_ZIP_MODE).errstr()) == "INVALID_ZIP_MODE");
        CHECK(string(ZipError(ZipError::INVALID_ARCHIVE).errstr())  == "INVALID_ARCHIVE");
        CHECK(string(ZipError(ZipError::NOT_A_FILE).errstr())       == "NOT_A_FILE");
        CHECK(string(ZipError(ZipError::NOT_A_DIRECTORY).errstr())  == "NOT_A_DIRECTORY");
        CHECK(string(ZipError(ZipError::KUBA_ZIP_ERROR).errstr())   == "KUBA_ZIP_ERROR");
    }

    SUBCASE("An unknown fault falls back rather than misreporting") {

        CHECK(string(ZipError(9999).errstr()) == "UNKNOWN");
    }

    SUBCASE("The message carries the detail it was given") {

        /* Both spellings must agree. A literal selects the const char*
         * constructor, which routes through setError(), while a std::string
         * uses the inline one -- they used to produce different messages.
         */
        ZipError literal(ZipError::NOT_A_FILE, "somefile");
        ZipError str(ZipError::NOT_A_FILE, std::string("somefile"));

        CHECK(literal.payload == ZipError::NOT_A_FILE);
        CHECK(string(literal.what()).find("somefile") != string::npos);
        CHECK(string(literal.what()) == string(str.what()));

        ZipError dir(ZipError::NOT_A_DIRECTORY, "somedir");
        CHECK(string(dir.what()).find("somedir") != string::npos);
    }

    SUBCASE("It is catchable as a standard exception") {

        bool caught = false;

        try {
            throw ZipError(ZipError::INVALID_ARCHIVE, "broken.zip");
        } catch (const std::exception &e) {
            caught = true;
            CHECK(string(e.what()).find("broken.zip") != string::npos);
        }

        CHECK(caught);
    }
}
