// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "main.h"

TEST_CASE("Time") {

    SUBCASE("A default Time is zero") {

        CHECK(Time().asNanoseconds() == 0);
    }

    SUBCASE("The unit factories agree with each other") {

        CHECK(Time::microseconds(1).asNanoseconds() == 1000);
        CHECK(Time::milliseconds(1).asNanoseconds() == 1000000);
        CHECK(Time::seconds(i64(1)).asNanoseconds() == 1000000000);

        CHECK(Time::seconds(i64(2)).asMilliseconds() == 2000);
        CHECK(Time::milliseconds(2500).asMicroseconds() == 2500000);
    }

    SUBCASE("Conversion to smaller units truncates") {

        CHECK(Time::nanoseconds(1999).asMicroseconds() == 1);
        CHECK(Time::nanoseconds(999).asMicroseconds() == 0);
    }

    SUBCASE("asSeconds converts to floating point") {

        CHECK(Time::milliseconds(500).asSeconds() == doctest::Approx(0.5f));
        CHECK(Time::seconds(i64(3)).asSeconds() == doctest::Approx(3.0f));
    }

    SUBCASE("Comparison operators") {

        auto a = Time::milliseconds(100);
        auto b = Time::milliseconds(200);
        auto c = Time::milliseconds(100);

        CHECK(a == c);
        CHECK(a != b);
        CHECK(a < b);
        CHECK(b > a);
        CHECK(a <= c);
        CHECK(a >= c);
    }

    SUBCASE("Negative durations are representable") {

        CHECK(Time::milliseconds(-5).asNanoseconds() == -5000000);
        CHECK(Time::milliseconds(-5) < Time());
    }

    SUBCASE("now() advances") {

        auto t1 = Time::now();
        auto t2 = Time::now();

        CHECK(t2 >= t1);
        CHECK(t1.asNanoseconds() > 0);
    }

    SUBCASE("buildTime produces a non-empty stamp") {

        CHECK_FALSE(Time::buildTime().empty());
    }
}
