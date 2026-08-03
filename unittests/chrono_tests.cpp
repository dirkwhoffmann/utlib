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

    /* The epoch of now() is deliberately not asserted here.
     *
     * It differs per platform: mach_absolute_time() and CLOCK_MONOTONIC both
     * count from boot, while the generic implementation counts from the first
     * call to now() -- so on Windows the very first call returns exactly 0.
     * All three are valid monotonic clocks; only differences between two
     * readings are meaningful, and that is all these check.
     */
    SUBCASE("now() never goes backwards") {

        auto t1 = Time::now();
        auto t2 = Time::now();

        CHECK(t2 >= t1);
        CHECK(t1.asNanoseconds() >= 0);
    }

    SUBCASE("sleep() waits roughly the requested time") {

        auto t1 = Time::now();
        Time::milliseconds(20).sleep();
        auto t2 = Time::now();

        /* An upper bound as well as a lower one, because the failure this
         * guards against is sleeping far too *long*: the macOS path used to
         * mix nanoseconds into a mach-time deadline and hung outright. Kept
         * loose enough that a loaded CI runner will not trip it.
         */
        CHECK((t2 - t1).asMilliseconds() < 5000);

        CHECK(t2 > t1);

        /* A generous lower bound. Sleep may overshoot by a lot on a loaded CI
         * runner, so only the floor is checked -- and that floor is under the
         * requested 20ms, because a coarse system clock can report slightly
         * less than the time actually slept.
         */
        CHECK((t2 - t1).asMilliseconds() >= 5);
    }

    SUBCASE("buildTime produces a non-empty stamp") {

        CHECK_FALSE(Time::buildTime().empty());
    }
}
