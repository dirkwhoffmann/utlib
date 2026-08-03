// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "main.h"

#include <cstring>

TEST_CASE("Bits: extraction macros") {

    SUBCASE("Nibbles") {

        CHECK(LO_NIBBLE(0xAB) == 0x0B);
        CHECK(HI_NIBBLE(0xAB) == 0x0A);
    }

    SUBCASE("Bytes of a word") {

        CHECK(LO_BYTE(0xABCD) == 0xCD);
        CHECK(HI_BYTE(0xABCD) == 0xAB);
    }

    SUBCASE("Words of a long") {

        CHECK(LO_WORD(0x12345678u) == 0x5678);
        CHECK(HI_WORD(0x12345678u) == 0x1234);
    }

    SUBCASE("Indexed byte access") {

        const u32 x = 0x12345678u;

        CHECK(BYTE0(x) == 0x78);
        CHECK(BYTE1(x) == 0x56);
        CHECK(BYTE2(x) == 0x34);
        CHECK(BYTE3(x) == 0x12);

        for (int i = 0; i < 4; i++) {
            CHECK(GET_BYTE(x, i) == ((x >> (8 * i)) & 0xFF));
        }
    }
}

TEST_CASE("Bits: composition macros") {

    SUBCASE("Little-endian composition") {

        CHECK(LO_HI(0xCDu, 0xABu) == 0xABCD);
        CHECK(LO_W_HI_W(0x5678u, 0x1234u) == 0x12345678u);
        CHECK(LO_LO_HI_HI(0x78u, 0x56u, 0x34u, 0x12u) == 0x12345678u);
    }

    SUBCASE("Big-endian composition") {

        CHECK(HI_LO(0xABu, 0xCDu) == 0xABCD);
        CHECK(HI_W_LO_W(0x1234u, 0x5678u) == 0x12345678u);
        CHECK(HI_HI_LO_LO(0x12u, 0x34u, 0x56u, 0x78u) == 0x12345678u);
    }

    SUBCASE("Composition inverts extraction") {

        const u16 x = 0xBEEF;
        CHECK(LO_HI(LO_BYTE(x), HI_BYTE(x)) == x);
    }
}

TEST_CASE("Bits: single-bit operations") {

    SUBCASE("GET_BIT reads the requested bit") {

        const u8 x = 0b1010'0001;

        CHECK(GET_BIT(x, 0));
        CHECK_FALSE(GET_BIT(x, 1));
        CHECK(GET_BIT(x, 5));
        CHECK(GET_BIT(x, 7));
    }

    SUBCASE("SET_BIT, CLR_BIT and TOGGLE_BIT") {

        u32 x = 0;

        SET_BIT(x, 3);
        CHECK(x == 0b1000);

        SET_BIT(x, 0);
        CHECK(x == 0b1001);

        CLR_BIT(x, 3);
        CHECK(x == 0b0001);

        TOGGLE_BIT(x, 1);
        CHECK(x == 0b0011);

        TOGGLE_BIT(x, 1);
        CHECK(x == 0b0001);
    }

    SUBCASE("REPLACE_BIT follows the value") {

        u32 x = 0;

        REPLACE_BIT(x, 2, true);
        CHECK(GET_BIT(x, 2));

        REPLACE_BIT(x, 2, false);
        CHECK_FALSE(GET_BIT(x, 2));
    }

    SUBCASE("Byte and word replacement") {

        CHECK(REPLACE_LO(0xABCDu, 0x12u) == 0xAB12);
        CHECK(REPLACE_HI(0xABCDu, 0x12u) == 0x12CD);
        CHECK(REPLACE_LO_WORD(0x12345678u, 0xABCDu) == 0x1234ABCDu);
    }
}

TEST_CASE("Bits: masks") {

    SUBCASE("ALL_CLR and ALL_SET") {

        const u8 x = 0b1100'0011;

        CHECK(ALL_SET(x, 0b1100'0000));
        CHECK(ALL_SET(x, 0b0000'0011));
        CHECK_FALSE(ALL_SET(x, 0b0001'0000));

        CHECK(ALL_CLR(x, 0b0011'1100));
        CHECK_FALSE(ALL_CLR(x, 0b0000'0001));
    }
}

TEST_CASE("Bits: byte order") {

    SUBCASE("bigEndian swaps the byte order") {

        CHECK(bigEndian(u16(0x1234)) == 0x3412);
        CHECK(bigEndian(u32(0x12345678u)) == 0x78563412u);
        CHECK(bigEndian(u64(0x0123456789ABCDEFull)) == 0xEFCDAB8967452301ull);
    }

    SUBCASE("bigEndian is its own inverse") {

        CHECK(bigEndian(bigEndian(u16(0xBEEF))) == 0xBEEF);
        CHECK(bigEndian(bigEndian(u32(0xDEADBEEFu))) == 0xDEADBEEFu);
    }
}

TEST_CASE("Bits: buffer helpers") {

    SUBCASE("isZero") {

        const u8 zeros[4] = { 0, 0, 0, 0 };
        const u8 mixed[4] = { 0, 0, 1, 0 };

        CHECK(isZero(zeros, 4));
        CHECK_FALSE(isZero(mixed, 4));

        // Only the requested range is inspected
        CHECK(isZero(mixed, 2));
    }

    SUBCASE("replace substitutes a byte sequence") {

        char buf[] = "aXXb";
        const char seq[] = "XX";
        const char sub[] = "YY";

        replace(buf, isize(sizeof(buf)), seq, sub);

        CHECK(std::string(buf) == "aYYb");
    }
}
