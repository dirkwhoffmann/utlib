// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "main.h"
#include <utl/math.h>
#include <cfloat>
#include <cmath>
#include <thread>

namespace {

// Puts the thread-local environment into its default state
struct CleanEnv {

    static void reset() {

        FloatEnv::setRoundingMode(RoundingMode::NEAREST);
        FloatEnv::setPrecision(Precision::EXTENDED);
        FloatEnv::clearFlags();
    }

    CleanEnv() { reset(); }
    ~CleanEnv() { reset(); }
};

Float80 N(i64 value) { return Float80::fromInt(value); }

// Checks a result against its expected raw bits
#define CHECK_BITS(x, se, man) \
    do { CHECK_EQ((x).signExp(), u16(se)); CHECK_EQ((x).mantissa(), u64(man)); } while (0)

}

TEST_CASE("Float80: Creation") {

    CleanEnv env;

    SUBCASE("A default Float80 is +0") {

        Float80 x;
        CHECK_BITS(x, 0x0000, 0);
        CHECK(x.isZero());
        CHECK(!x.isNegative());
    }

    SUBCASE("Raw bits are preserved") {

        auto x = Float80::fromBits(0xC001, 0xA000000000000000ULL);
        CHECK_EQ(x.signExp(), 0xC001);
        CHECK_EQ(x.mantissa(), 0xA000000000000000ULL);
        CHECK(x.sign());
        CHECK_EQ(x.biasedExponent(), 0x4001);
        CHECK_EQ(x.exponent(), 2);
        CHECK(x.integerBit());
    }

    SUBCASE("Integers") {

        CHECK_BITS(N(0), 0x0000, 0);
        CHECK_BITS(N(1), 0x3FFF, 0x8000000000000000ULL);
        CHECK_BITS(N(-2), 0xC000, 0x8000000000000000ULL);
        CHECK_BITS(N(3), 0x4000, 0xC000000000000000ULL);
        CHECK_BITS(N(INT64_MAX), 0x403D, 0xFFFFFFFFFFFFFFFEULL);
        CHECK_BITS(N(INT64_MIN), 0xC03E, 0x8000000000000000ULL);
        CHECK_EQ(FloatEnv::flags(), 0);
    }

    SUBCASE("Floats and doubles are converted exactly") {

        CHECK_BITS(Float80::fromDouble(1.5), 0x3FFF, 0xC000000000000000ULL);
        CHECK_BITS(Float80::fromDouble(-0.25), 0xBFFD, 0x8000000000000000ULL);
        CHECK_BITS(Float80::fromFloat(0.5f), 0x3FFE, 0x8000000000000000ULL);

        for (double d : { 0.1, -3.14159265358979, 1e300, 5e-324, 123456789.0 }) {
            CHECK_EQ(Float80::fromDouble(d).toDouble(), d);
        }
        for (float f : { 0.1f, -3.14159f, 1e30f, 1e-45f }) {
            CHECK_EQ(Float80::fromFloat(f).toFloat(), f);
        }
        CHECK_EQ(FloatEnv::flags(), 0);
    }

    SUBCASE("Special values") {

        CHECK_BITS(Float80::zero(), 0x0000, 0);
        CHECK_BITS(Float80::zero(true), 0x8000, 0);
        CHECK_BITS(Float80::one(), 0x3FFF, 0x8000000000000000ULL);
        CHECK_BITS(Float80::one(true), 0xBFFF, 0x8000000000000000ULL);
        CHECK_BITS(Float80::infinity(), 0x7FFF, 0x8000000000000000ULL);
        CHECK_BITS(Float80::infinity(true), 0xFFFF, 0x8000000000000000ULL);
        CHECK(Float80::quietNaN().isQuietNaN());
        CHECK(Float80::signalingNaN().isSignalingNaN());
    }

    SUBCASE("Long double") {

        if constexpr (LDBL_MANT_DIG == 64) {

            // Exact on x87 hosts
            auto x = Float80::fromBits(0x3FFD, 0xAAAAAAAAAAAAAAABULL);
            CHECK(Float80::fromLongDouble(x.toLongDouble()).identical(x));
            CHECK_EQ(Float80::fromLongDouble(1.5L).toDouble(), 1.5);

        } else {

            CHECK_EQ(Float80::fromLongDouble(1.5L).toDouble(), 1.5);
            CHECK_EQ(Float80::one().toLongDouble(), 1.0L);
        }
    }

    SUBCASE("String representation") {

        CHECK_EQ(N(1).str(), "0x3FFF:8000000000000000");
        CHECK_EQ(N(-2).str(), "0xC000:8000000000000000");
    }
}

TEST_CASE("Float80: Classification") {

    CleanEnv env;

    auto zero = Float80::zero();
    auto one = N(1);
    auto inf = Float80::infinity();
    auto qnan = Float80::quietNaN();
    auto snan = Float80::signalingNaN();
    auto denormal = Float80::fromBits(0x0000, 0x0000000000000001ULL);
    auto pseudoDenormal = Float80::fromBits(0x0000, 0x8000000000000000ULL);

    SUBCASE("Zero") {

        CHECK(zero.isZero());
        CHECK(Float80::zero(true).isZero());
        CHECK(!one.isZero());
        CHECK(!denormal.isZero());
    }

    SUBCASE("Infinity") {

        CHECK(inf.isInfinite());
        CHECK(Float80::infinity(true).isInfinite());
        CHECK(!inf.isNaN());
        CHECK(!inf.isFinite());
        CHECK(one.isFinite());
    }

    SUBCASE("NaN") {

        CHECK(qnan.isNaN());
        CHECK(qnan.isQuietNaN());
        CHECK(!qnan.isSignalingNaN());
        CHECK(snan.isNaN());
        CHECK(snan.isSignalingNaN());
        CHECK(!snan.isQuietNaN());
        CHECK(!inf.isSignalingNaN());
        CHECK(!one.isNaN());
    }

    SUBCASE("Normal and denormal numbers") {

        CHECK(one.isNormal());
        CHECK(!zero.isNormal());
        CHECK(!inf.isNormal());
        CHECK(!denormal.isNormal());
        CHECK(denormal.isDenormal());
        CHECK(!one.isDenormal());
        CHECK(!pseudoDenormal.isDenormal());
        CHECK(!pseudoDenormal.isNormal());
    }

    SUBCASE("Sign") {

        CHECK(!one.isNegative());
        CHECK(N(-1).isNegative());
        CHECK(Float80::zero(true).isNegative());
    }
}

TEST_CASE("Float80: Arithmetic") {

    CleanEnv env;

    SUBCASE("Exact results") {

        CHECK_BITS(N(1) + N(1), 0x4000, 0x8000000000000000ULL);
        CHECK_BITS(N(5) - N(7), 0xC000, 0x8000000000000000ULL);
        CHECK_BITS(N(3) * N(4), 0x4002, 0xC000000000000000ULL);
        CHECK_BITS(N(12) / N(4), 0x4000, 0xC000000000000000ULL);
        CHECK_EQ(FloatEnv::flags(), 0);
    }

    SUBCASE("Compound assignment") {

        auto x = N(2);
        x += N(3);
        CHECK(x == N(5));
        x -= N(1);
        CHECK(x == N(4));
        x *= N(3);
        CHECK(x == N(12));
        x /= N(6);
        CHECK(x == N(2));
    }

    SUBCASE("Negation and absolute value") {

        CHECK_BITS(-N(1), 0xBFFF, 0x8000000000000000ULL);
        CHECK_BITS(-(-N(1)), 0x3FFF, 0x8000000000000000ULL);
        CHECK_BITS(N(-3).abs(), 0x4000, 0xC000000000000000ULL);
        CHECK_BITS(-Float80::zero(), 0x8000, 0);

        // Neither operation raises an exception, not even for signaling NaNs
        auto r = -Float80::signalingNaN();
        auto s = Float80::signalingNaN().abs();
        CHECK(r.isSignalingNaN());
        CHECK(s.isSignalingNaN());
        CHECK_EQ(FloatEnv::flags(), 0);
    }

    SUBCASE("Inexact results") {

        auto third = N(1) / N(3);
        CHECK_BITS(third, 0x3FFD, 0xAAAAAAAAAAAAAAABULL);
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INEXACT);
    }

    SUBCASE("Flags are sticky") {

        N(1) / N(3);
        N(1) + N(1);
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INEXACT);

        FloatEnv::clearFlags();
        CHECK_EQ(FloatEnv::flags(), 0);
    }

    SUBCASE("Division by zero") {

        auto x = N(1) / Float80::zero();
        CHECK_BITS(x, 0x7FFF, 0x8000000000000000ULL);
        CHECK_EQ(FloatEnv::flags(), FloatFlags::DIV_BY_ZERO);

        FloatEnv::clearFlags();
        auto y = N(-1) / Float80::zero();
        CHECK_BITS(y, 0xFFFF, 0x8000000000000000ULL);
        CHECK_EQ(FloatEnv::flags(), FloatFlags::DIV_BY_ZERO);
    }

    SUBCASE("Invalid operations") {

        auto x = Float80::zero() / Float80::zero();
        CHECK(x.isNaN());
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INVALID);

        FloatEnv::clearFlags();
        auto y = Float80::infinity() - Float80::infinity();
        CHECK(y.isNaN());
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INVALID);

        FloatEnv::clearFlags();
        auto z = N(-1).sqrt();
        CHECK(z.isNaN());
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INVALID);
    }

    SUBCASE("NaN propagation") {

        auto x = Float80::quietNaN() + N(1);
        CHECK(x.isQuietNaN());
        CHECK_EQ(FloatEnv::flags(), 0);

        auto y = Float80::signalingNaN() + N(1);
        CHECK(y.isQuietNaN());
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INVALID);
    }

    SUBCASE("Overflow") {

        auto max = Float80::fromBits(0x7FFE, 0xFFFFFFFFFFFFFFFFULL);
        auto x = max * N(2);
        CHECK_BITS(x, 0x7FFF, 0x8000000000000000ULL);
        CHECK_EQ(FloatEnv::flags(), FloatFlags::OVFL | FloatFlags::INEXACT);

        // Rounding toward zero saturates at the largest finite number
        FloatEnv::clearFlags();
        FloatEnv::setRoundingMode(RoundingMode::ZERO);
        auto y = max * N(2);
        CHECK(y.identical(max));
    }

    SUBCASE("Underflow") {

        auto min = Float80::fromBits(0x0001, 0x8000000000000000ULL);
        auto x = min / N(3);
        CHECK(x.isDenormal());
        CHECK((FloatEnv::flags() & FloatFlags::UNFL) != 0);
        CHECK((FloatEnv::flags() & FloatFlags::INEXACT) != 0);
    }

    SUBCASE("Square root") {

        CHECK(N(4).sqrt() == N(2));
        CHECK(N(0).sqrt().isZero());
        CHECK_EQ(FloatEnv::flags(), 0);

        // Both results are correctly rounded
        CHECK_EQ(N(2).sqrt().toDouble(), std::sqrt(2.0));
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INEXACT);
    }

    SUBCASE("IEEE remainder") {

        CHECK(N(5).rem(N(3)) == N(-1));
        CHECK(N(7).rem(N(3)) == N(1));
        CHECK(N(-7).rem(N(3)) == N(-1));
        CHECK_EQ(FloatEnv::flags(), 0);

        CHECK(N(1).rem(Float80::zero()).isNaN());
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INVALID);
    }

    SUBCASE("Scaling") {

        CHECK(N(3).scale(N(4)) == N(48));
        CHECK(N(48).scale(N(-4)) == N(3));
        CHECK(N(3).scale(Float80::fromDouble(2.9)) == N(12));
        CHECK_EQ(FloatEnv::flags(), 0);
    }
}

TEST_CASE("Float80: Rounding mode") {

    CleanEnv env;

    auto third = N(1) / N(3);
    auto nthird = N(-1) / N(3);

    SUBCASE("Division") {

        FloatEnv::setRoundingMode(RoundingMode::NEAREST);
        CHECK_BITS(N(1) / N(3), 0x3FFD, 0xAAAAAAAAAAAAAAABULL);

        FloatEnv::setRoundingMode(RoundingMode::ZERO);
        CHECK_BITS(N(1) / N(3), 0x3FFD, 0xAAAAAAAAAAAAAAAAULL);
        CHECK_BITS(N(-1) / N(3), 0xBFFD, 0xAAAAAAAAAAAAAAAAULL);

        FloatEnv::setRoundingMode(RoundingMode::DOWN);
        CHECK_BITS(N(1) / N(3), 0x3FFD, 0xAAAAAAAAAAAAAAAAULL);
        CHECK_BITS(N(-1) / N(3), 0xBFFD, 0xAAAAAAAAAAAAAAABULL);

        FloatEnv::setRoundingMode(RoundingMode::UP);
        CHECK_BITS(N(1) / N(3), 0x3FFD, 0xAAAAAAAAAAAAAAABULL);
        CHECK_BITS(N(-1) / N(3), 0xBFFD, 0xAAAAAAAAAAAAAAAAULL);
    }

    SUBCASE("The mode can be read back") {

        for (auto mode : { RoundingMode::NEAREST, RoundingMode::ZERO, RoundingMode::DOWN, RoundingMode::UP }) {
            FloatEnv::setRoundingMode(mode);
            CHECK(FloatEnv::roundingMode() == mode);
        }
    }

    SUBCASE("roundToInt") {

        auto check = [&](RoundingMode mode, double in, double out) {
            FloatEnv::setRoundingMode(mode);
            CHECK_EQ(Float80::fromDouble(in).roundToInt().toDouble(), out);
        };

        check(RoundingMode::NEAREST,  2.5,  2.0);
        check(RoundingMode::NEAREST,  3.5,  4.0);
        check(RoundingMode::NEAREST, -2.5, -2.0);
        check(RoundingMode::ZERO,     2.7,  2.0);
        check(RoundingMode::ZERO,    -2.7, -2.0);
        check(RoundingMode::DOWN,     2.5,  2.0);
        check(RoundingMode::DOWN,    -2.5, -3.0);
        check(RoundingMode::UP,       2.5,  3.0);
        check(RoundingMode::UP,      -2.5, -2.0);
    }

    SUBCASE("Integer conversion") {

        auto x = Float80::fromDouble(2.5);
        auto y = Float80::fromDouble(3.5);

        CHECK_EQ(x.toInt32(), 2);
        CHECK_EQ(y.toInt32(), 4);
        CHECK_EQ(x.toInt64(), 2);
        CHECK_EQ(y.toInt64(), 4);
        CHECK_EQ(y.toInt32Truncated(), 3);
        CHECK_EQ(y.toInt64Truncated(), 3);
        CHECK_EQ(Float80::fromDouble(-3.5).toInt32Truncated(), -3);

        FloatEnv::setRoundingMode(RoundingMode::UP);
        CHECK_EQ(x.toInt32(), 3);
        FloatEnv::setRoundingMode(RoundingMode::DOWN);
        CHECK_EQ(Float80::fromDouble(-2.5).toInt32(), -3);
    }

    SUBCASE("Out-of-range integer conversion") {

        /* The value that is returned depends on the NaN / overflow
         * specialization of SoftFloat (currently x87 semantics, which return
         * the "integer indefinite" value). The invalid flag is raised in any case.
         */
        auto extreme32 = [](i32 v) { return v == INT32_MAX || v == INT32_MIN; };
        auto extreme64 = [](i64 v) { return v == INT64_MAX || v == INT64_MIN; };

        FloatEnv::clearFlags();
        CHECK(extreme32(Float80::fromDouble(1e20).toInt32()));
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INVALID);

        FloatEnv::clearFlags();
        CHECK(extreme32(Float80::fromDouble(-1e20).toInt32()));
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INVALID);

        FloatEnv::clearFlags();
        CHECK(extreme64(Float80::fromDouble(1e30).toInt64()));
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INVALID);
    }

    SUBCASE("Narrowing conversions") {

        // 1/3 rounded to float and double
        CHECK_EQ(third.toFloat(), 1.0f / 3.0f);
        CHECK_EQ(third.toDouble(), 1.0 / 3.0);
        CHECK_EQ(nthird.toDouble(), -1.0 / 3.0);

        // The directed modes pick the neighbor of 1/3 in the requested direction
        auto exact = Float80::fromBits(0x3FFD, 0xAAAAAAAAAAAAAAABULL);
        auto nexact = -exact;
        double d = 1.0 / 3.0;

        FloatEnv::setRoundingMode(RoundingMode::ZERO);
        CHECK_EQ(exact.toDouble(), d);
        CHECK_EQ(nexact.toDouble(), -d);

        FloatEnv::setRoundingMode(RoundingMode::DOWN);
        CHECK_EQ(exact.toDouble(), d);
        CHECK_EQ(nexact.toDouble(), std::nextafter(-d, -1.0));

        FloatEnv::setRoundingMode(RoundingMode::UP);
        CHECK_EQ(exact.toDouble(), std::nextafter(d, 1.0));
        CHECK_EQ(nexact.toDouble(), -d);
    }
}

TEST_CASE("Float80: Rounding precision") {

    CleanEnv env;

    SUBCASE("Extended precision keeps 64 bits") {

        CHECK_BITS(N(1) / N(3), 0x3FFD, 0xAAAAAAAAAAAAAAABULL);
    }

    SUBCASE("Double precision keeps 53 bits") {

        FloatEnv::setPrecision(Precision::DOUBLE);
        CHECK_BITS(N(1) / N(3), 0x3FFD, 0xAAAAAAAAAAAAA800ULL);
        CHECK(FloatEnv::precision() == Precision::DOUBLE);
    }

    SUBCASE("Single precision keeps 24 bits") {

        FloatEnv::setPrecision(Precision::SINGLE);
        auto x = N(1) / N(3);
        CHECK_BITS(x, 0x3FFD, 0xAAAAAB0000000000ULL);
        CHECK_EQ(x.toFloat(), 1.0f / 3.0f);
        CHECK(FloatEnv::precision() == Precision::SINGLE);
    }

    SUBCASE("Precision does not narrow the exponent range") {

        FloatEnv::setPrecision(Precision::SINGLE);
        auto big = Float80::fromDouble(1e300) * Float80::fromDouble(1e300);
        CHECK(big.isFinite());
        CHECK(big.toDouble() > 1e300);
    }
}

TEST_CASE("Float80: Comparison") {

    CleanEnv env;

    auto qnan = Float80::quietNaN();
    auto snan = Float80::signalingNaN();

    SUBCASE("Ordered values") {

        CHECK(N(1) == N(1));
        CHECK(N(1) != N(2));
        CHECK(N(1) < N(2));
        CHECK(N(1) <= N(2));
        CHECK(N(1) <= N(1));
        CHECK(N(2) > N(1));
        CHECK(N(2) >= N(1));
        CHECK(!(N(2) < N(1)));
        CHECK(N(-2) < N(-1));
        CHECK(N(-1) < N(1));
        CHECK(Float80::infinity(true) < N(-1000000));
        CHECK(N(1000000) < Float80::infinity());
        CHECK_EQ(FloatEnv::flags(), 0);
    }

    SUBCASE("Zeros compare equal, but are not identical") {

        auto pz = Float80::zero();
        auto nz = Float80::zero(true);
        CHECK(pz == nz);
        CHECK(!(nz < pz));
        CHECK(!pz.identical(nz));
        CHECK(pz.identical(Float80()));
    }

    SUBCASE("NaN is unordered") {

        CHECK(!(qnan == qnan));
        CHECK(qnan != qnan);
        CHECK(!(qnan < N(1)));
        CHECK(!(qnan <= N(1)));
        CHECK(!(qnan > N(1)));
        CHECK(!(qnan >= N(1)));
        CHECK(qnan.unordered(N(1)));
        CHECK(N(1).unordered(qnan));
        CHECK(!N(1).unordered(N(2)));
        CHECK(qnan.identical(qnan));
    }

    SUBCASE("Exceptions") {

        // Equality is quiet for quiet NaNs, but not for signaling ones
        FloatEnv::clearFlags();
        CHECK(!(qnan == N(1)));
        CHECK_EQ(FloatEnv::flags(), 0);
        CHECK(!(snan == N(1)));
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INVALID);

        // Ordering comparisons signal for all NaNs
        FloatEnv::clearFlags();
        CHECK(!(qnan < N(1)));
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INVALID);
    }
}

TEST_CASE("FloatEnv") {

    CleanEnv env;

    SUBCASE("Defaults") {

        CHECK(FloatEnv::roundingMode() == RoundingMode::NEAREST);
        CHECK(FloatEnv::precision() == Precision::EXTENDED);
        CHECK_EQ(FloatEnv::flags(), 0);
    }

    SUBCASE("Raising and clearing flags") {

        FloatEnv::raiseFlags(FloatFlags::INVALID | FloatFlags::INEXACT);
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INVALID | FloatFlags::INEXACT);

        FloatEnv::clearFlags(FloatFlags::INVALID);
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INEXACT);

        FloatEnv::clearFlags();
        CHECK_EQ(FloatEnv::flags(), 0);
    }

    SUBCASE("Scope restores mode and precision") {

        {
            FloatEnv::Scope scope;
            FloatEnv::setRoundingMode(RoundingMode::UP);
            FloatEnv::setPrecision(Precision::SINGLE);
            N(1) / N(3);
            CHECK(FloatEnv::roundingMode() == RoundingMode::UP);
        }
        CHECK(FloatEnv::roundingMode() == RoundingMode::NEAREST);
        CHECK(FloatEnv::precision() == Precision::EXTENDED);

        // Flags raised inside the scope are kept by default
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INEXACT);
    }

    SUBCASE("Scope can discard flags") {

        FloatEnv::raiseFlags(FloatFlags::UNFL);
        {
            FloatEnv::Scope scope(true);
            N(1) / N(3);
        }
        CHECK_EQ(FloatEnv::flags(), FloatFlags::UNFL);
    }

    SUBCASE("The environment is thread-local") {

        FloatEnv::setRoundingMode(RoundingMode::ZERO);
        FloatEnv::setPrecision(Precision::SINGLE);
        FloatEnv::raiseFlags(FloatFlags::INVALID);

        RoundingMode mode = RoundingMode::UP;
        Precision prec = Precision::SINGLE;
        u8 flags = 0xFF;
        Float80 result;

        std::thread t([&] {

            mode = FloatEnv::roundingMode();
            prec = FloatEnv::precision();
            flags = FloatEnv::flags();
            result = N(1) / N(3);
        });
        t.join();

        // The new thread starts out with a pristine environment
        CHECK(mode == RoundingMode::NEAREST);
        CHECK(prec == Precision::EXTENDED);
        CHECK_EQ(flags, 0);
        CHECK_BITS(result, 0x3FFD, 0xAAAAAAAAAAAAAAABULL);

        // Our own environment is untouched
        CHECK(FloatEnv::roundingMode() == RoundingMode::ZERO);
        CHECK(FloatEnv::precision() == Precision::SINGLE);
        CHECK_EQ(FloatEnv::flags(), FloatFlags::INVALID);
    }
}
