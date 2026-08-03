// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "main.h"

TEST_CASE("Strings: creating") {

    SUBCASE("createStr stops at the terminator") {

        const u8 buf[] = { 'a', 'b', 'c', 0, 'd' };

        CHECK(createStr(buf, 5) == "abc");
        CHECK(createStr(buf, 2) == "ab");
        CHECK(createStr(buf, 0) == "");
    }

    SUBCASE("createAscii replaces unprintable bytes") {

        const u8 buf[] = { 'a', 0x00, 'b', 0x1f, 'c' };

        CHECK(createAscii(buf, 5) == "a.b.c");
        CHECK(createAscii(buf, 5, '?') == "a?b?c");
    }

    SUBCASE("repeat") {

        CHECK(repeat("ab", 0) == "");
        CHECK(repeat("ab", 1) == "ab");
        CHECK(repeat("ab", 3) == "ababab");
        CHECK(repeat("", 5) == "");
    }
}

TEST_CASE("Strings: converting") {

    SUBCASE("parseHex accepts hexadecimal numbers") {

        isize result = -1;

        CHECK(parseHex("0", &result));
        CHECK(result == 0);

        CHECK(parseHex("ff", &result));
        CHECK(result == 255);

        CHECK(parseHex("FF", &result));
        CHECK(result == 255);

        CHECK(parseHex("100", &result));
        CHECK(result == 256);
    }

    SUBCASE("parseHex rejects anything else") {

        isize result = 0;

        CHECK_FALSE(parseHex("", &result));
        CHECK_FALSE(parseHex("xyz", &result));
        CHECK_FALSE(parseHex("12g4", &result));
    }

    SUBCASE("hexstr pads to the requested width") {

        CHECK(hexstr<2>(0) == "00");
        CHECK(hexstr<2>(255) == "ff");
        CHECK(hexstr<4>(255) == "00ff");
        CHECK(hexstr<8>(1) == "00000001");
    }
}

TEST_CASE("Strings: transforming") {

    SUBCASE("Case conversion leaves non-letters alone") {

        CHECK(lowercased("AbC-1") == "abc-1");
        CHECK(uppercased("AbC-1") == "ABC-1");
        CHECK(lowercased("") == "");
        CHECK(uppercased("") == "");
    }

    SUBCASE("makePrintable") {

        CHECK(makePrintable("abc") == "abc");
        CHECK(makePrintable(string("a\tb")) != "a\tb");
    }
}

TEST_CASE("Strings: stripping") {

    SUBCASE("trim removes from both ends") {

        CHECK(ltrim("  ab  ") == "ab  ");
        CHECK(rtrim("  ab  ") == "  ab");
        CHECK(trim("  ab  ") == "ab");
    }

    SUBCASE("trim accepts a custom character set") {

        CHECK(trim("xxabxx", "x") == "ab");
        CHECK(ltrim("--ab--", "-") == "ab--");
        CHECK(rtrim("--ab--", "-") == "--ab");
    }

    SUBCASE("Strings made entirely of trim characters collapse to empty") {

        CHECK(trim("    ") == "");
        CHECK(trim("") == "");
    }

    SUBCASE("unquote strips a matched pair of quotes") {

        CHECK(unquote("\"ab\"") == "ab");
        CHECK(unquote("ab") == "ab");
        CHECK(unquote("") == "");
    }

    SUBCASE("commonPrefix") {

        CHECK(commonPrefix("foobar", "foobaz") == "fooba");
        CHECK(commonPrefix("abc", "abc") == "abc");
        CHECK(commonPrefix("abc", "xyz") == "");
        CHECK(commonPrefix("", "abc") == "");

        CHECK(commonPrefix(std::vector<string> { "foobar", "foobaz", "food" }) == "foo");
        CHECK(commonPrefix(std::vector<string> { "abc" }) == "abc");
    }
}

TEST_CASE("Strings: padding, splitting, joining") {

    SUBCASE("padString") {

        CHECK(padString("ab", 4).size() == 4);
        CHECK(padString("abcd", 2) == "abcd");   // never truncates
    }

    SUBCASE("split") {

        auto parts = split("a,b,c", ',');
        REQUIRE(parts.size() == 3);
        CHECK(parts[0] == "a");
        CHECK(parts[1] == "b");
        CHECK(parts[2] == "c");

        /* An empty string splits into one empty field rather than nothing:
         * split() always emits the text after the last delimiter, and for ""
         * that is "". Same reason "a," yields two fields.
         */
        auto empty = split("", ',');
        REQUIRE(empty.size() == 1);
        CHECK(empty[0] == "");

        auto trailing = split("a,", ',');
        REQUIRE(trailing.size() == 2);
        CHECK(trailing[0] == "a");
        CHECK(trailing[1] == "");
    }

    SUBCASE("splitLast separates the tail") {

        auto [head, last] = splitLast(std::vector<string> { "a", "b", "c" });

        REQUIRE(head.size() == 2);
        CHECK(head[0] == "a");
        CHECK(head[1] == "b");
        CHECK(last == "c");
    }

    SUBCASE("concat") {

        CHECK(concat("a", "b", '/') == "a/b");
        CHECK(concat(std::vector<string> { "a", "b", "c" }, "-") == "a-b-c");
        CHECK(concat(std::vector<string> { }, "-") == "");
    }

    SUBCASE("split and concat round-trip") {

        const string original = "one/two/three";
        CHECK(concat(split(original, '/'), "/") == original);
    }
}

TEST_CASE("Strings: pretty printing") {

    SUBCASE("byteCountAsString") {

        // Only the shape is pinned down here -- the exact wording is cosmetic
        CHECK_FALSE(byteCountAsString(0).empty());
        CHECK_FALSE(byteCountAsString(1024).empty());
        CHECK_FALSE(byteCountAsString(1024 * 1024).empty());
    }

    SUBCASE("fillLevelAsString") {

        CHECK_FALSE(fillLevelAsString(0.0).empty());
        CHECK_FALSE(fillLevelAsString(0.5).empty());
        CHECK_FALSE(fillLevelAsString(1.0).empty());
    }
}
