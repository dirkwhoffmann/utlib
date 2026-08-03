// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "main.h"

#include <set>
#include <unordered_set>

TEST_CASE("UUID") {

    SUBCASE("Default construction yields the nil UUID") {

        UUID uuid;

        CHECK(uuid.isZero());
        CHECK_FALSE(bool(uuid));
        CHECK(uuid == UUID(0, 0));
    }

    SUBCASE("A non-zero UUID converts to true") {

        CHECK(bool(UUID(1, 0)));
        CHECK(bool(UUID(0, 1)));
    }

    SUBCASE("v4 produces distinct, non-zero values") {

        std::set<UUID> seen;

        for (int i = 0; i < 1000; i++) {

            auto uuid = UUID::v4();
            CHECK_FALSE(uuid.isZero());
            CHECK(seen.insert(uuid).second);
        }
    }

    SUBCASE("toString has the canonical 8-4-4-4-12 shape") {

        auto str = UUID::v4().toString();

        REQUIRE(str.size() == 36);
        CHECK(str[8]  == '-');
        CHECK(str[13] == '-');
        CHECK(str[18] == '-');
        CHECK(str[23] == '-');

        for (size_t i = 0; i < str.size(); i++) {
            if (i == 8 || i == 13 || i == 18 || i == 23) continue;
            CHECK(std::isxdigit(static_cast<unsigned char>(str[i])) != 0);
        }
    }

    SUBCASE("v4 sets the version and variant bits") {

        // RFC 4122: version nibble is 4, variant nibble is one of 8, 9, a, b
        for (int i = 0; i < 100; i++) {

            auto str = UUID::v4().toString();

            CHECK(str[14] == '4');
            CHECK((str[19] == '8' || str[19] == '9' ||
                   str[19] == 'a' || str[19] == 'b'));
        }
    }

    SUBCASE("String conversion round-trips") {

        for (int i = 0; i < 100; i++) {

            auto uuid = UUID::v4();
            CHECK(UUID::fromString(uuid.toString()) == uuid);
        }
    }

    SUBCASE("The nil UUID round-trips too") {

        UUID nil;
        CHECK(nil.toString() == "00000000-0000-0000-0000-000000000000");
        CHECK(UUID::fromString(nil.toString()) == nil);
    }

    SUBCASE("Ordering is by high word, then low") {

        CHECK(UUID(0, 1) < UUID(0, 2));
        CHECK(UUID(0, 9) < UUID(1, 0));
        CHECK(UUID(1, 0) > UUID(0, 9));
        CHECK(UUID(3, 4) == UUID(3, 4));
    }

    SUBCASE("Usable as a hash key") {

        std::unordered_set<UUID> set;

        auto a = UUID::v4();
        CHECK(set.insert(a).second);
        CHECK_FALSE(set.insert(a).second);
        CHECK(set.count(a) == 1);
    }
}
