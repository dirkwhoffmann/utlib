// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

/* Checks that the library's public surface is usable from the outside.
 *
 * Everything here is deliberately shallow. The point is not to exercise
 * behaviour -- the other files do that -- but to catch a header that only
 * compiles because of an include the library itself happened to pull in first,
 * which is exactly the breakage a consumer hits and the in-tree build does not.
 */

#include "main.h"

TEST_CASE("Umbrella headers are self-contained") {

    SUBCASE("Integer aliases have the advertised widths") {

        CHECK(sizeof(i8)  == 1);
        CHECK(sizeof(i16) == 2);
        CHECK(sizeof(i32) == 4);
        CHECK(sizeof(i64) == 8);

        CHECK(sizeof(u8)  == 1);
        CHECK(sizeof(u16) == 2);
        CHECK(sizeof(u32) == 4);
        CHECK(sizeof(u64) == 8);
    }

    SUBCASE("Integer aliases have the advertised signedness") {

        CHECK(std::is_signed_v<i8>);
        CHECK(std::is_signed_v<isize>);
        CHECK(std::is_unsigned_v<u8>);
        CHECK(std::is_unsigned_v<usize>);
    }

    SUBCASE("The limit constants bracket the types") {

        CHECK(MIN_I8 < 0);
        CHECK(MAX_I8 > 0);
        CHECK(MAX_U8 == 255);
        CHECK(MAX_U16 == 65535);
    }

    SUBCASE("fs is the standard filesystem namespace") {

        fs::path path = "a/b";
        CHECK(path.filename() == "b");
    }
}

TEST_CASE("Exceptions") {

    SUBCASE("IOError carries its payload and a message") {

        IOError error(IOError::FILE_NOT_FOUND, "somefile");

        CHECK(error.payload == IOError::FILE_NOT_FOUND);
        CHECK(string(error.errstr()) == "FILE_NOT_FOUND");
        CHECK(string(error.what()).find("somefile") != string::npos);
    }

    SUBCASE("IOError is catchable as a standard exception") {

        bool caught = false;

        try {
            throw IOError(IOError::DIR_NOT_FOUND, "somedir");
        } catch (const std::exception &e) {
            caught = true;
            CHECK(string(e.what()).find("somedir") != string::npos);
        }

        CHECK(caught);
    }
}
