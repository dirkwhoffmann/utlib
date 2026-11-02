// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "utl/math/Float80.h"
#include "softfloat/softfloat.h"
#include <bit>
#include <cfloat>
#include <cstring>

namespace utl {

namespace sf = ::softfloat;

namespace {

// Conversion between Float80 and the SoftFloat representation
inline sf::floatx80
toSF(const Float80 &x)
{
    sf::floatx80 result;
    result.high = x.signExp();
    result.low = x.mantissa();
    return result;
}

inline Float80
fromSF(const sf::floatx80 &x)
{
    return Float80::fromBits(x.high, x.low);
}

}


//
// FloatEnv
//

RoundingMode
FloatEnv::roundingMode()
{
    switch (sf::float_rounding_mode) {

        case sf::float_round_to_zero:   return RoundingMode::ZERO;
        case sf::float_round_down:      return RoundingMode::DOWN;
        case sf::float_round_up:        return RoundingMode::UP;
        default:                        return RoundingMode::NEAREST;
    }
}

void
FloatEnv::setRoundingMode(RoundingMode mode)
{
    switch (mode) {

        case RoundingMode::NEAREST: sf::float_rounding_mode = sf::float_round_nearest_even; break;
        case RoundingMode::ZERO:    sf::float_rounding_mode = sf::float_round_to_zero; break;
        case RoundingMode::DOWN:    sf::float_rounding_mode = sf::float_round_down; break;
        case RoundingMode::UP:      sf::float_rounding_mode = sf::float_round_up; break;
    }
}

Precision
FloatEnv::precision()
{
    switch (sf::floatx80_rounding_precision) {

        case 32:    return Precision::SINGLE;
        case 64:    return Precision::DOUBLE;
        default:    return Precision::EXTENDED;
    }
}

void
FloatEnv::setPrecision(Precision precision)
{
    sf::floatx80_rounding_precision = (sf::int8)precision;
}

u8
FloatEnv::flags()
{
    return (u8)sf::float_exception_flags;
}

void
FloatEnv::clearFlags(u8 mask)
{
    sf::float_exception_flags &= (sf::int8)~mask;
}

void
FloatEnv::raiseFlags(u8 mask)
{
    sf::float_exception_flags |= (sf::int8)(mask & FloatFlags::ALL);
}

FloatEnv::Scope::Scope(bool restoreFlags) : restoreFlags(restoreFlags)
{
    savedMode = FloatEnv::roundingMode();
    savedPrecision = FloatEnv::precision();
    savedFlags = FloatEnv::flags();
}

FloatEnv::Scope::~Scope()
{
    FloatEnv::setRoundingMode(savedMode);
    FloatEnv::setPrecision(savedPrecision);

    if (restoreFlags) {
        FloatEnv::clearFlags();
        FloatEnv::raiseFlags(savedFlags);
    }
}


//
// Float80: Creating numbers
//

Float80
Float80::fromInt(i64 value)
{
    // The conversion is always exact
    return fromSF(sf::int64_to_floatx80(value));
}

Float80
Float80::fromFloat(float value)
{
    // The conversion is always exact. Signaling NaNs raise the invalid flag.
    return fromSF(sf::float32_to_floatx80(std::bit_cast<u32>(value)));
}

Float80
Float80::fromDouble(double value)
{
    // The conversion is always exact. Signaling NaNs raise the invalid flag.
    return fromSF(sf::float64_to_floatx80(std::bit_cast<u64>(value)));
}

Float80
Float80::fromLongDouble(long double value)
{
    if constexpr (LDBL_MANT_DIG == 64 && std::endian::native == std::endian::little) {

        // x87 extended format: 8 bytes mantissa followed by 2 bytes sign and exponent
        u64 m;
        u16 e;
        std::memcpy(&m, &value, sizeof(m));
        std::memcpy(&e, reinterpret_cast<const char *>(&value) + sizeof(m), sizeof(e));
        return fromBits(e, m);

    } else {

        return fromDouble((double)value);
    }
}


//
// Float80: Converting to other number formats
//

i32
Float80::toInt32() const
{
    return sf::floatx80_to_int32(toSF(*this));
}

i32
Float80::toInt32Truncated() const
{
    return sf::floatx80_to_int32_round_to_zero(toSF(*this));
}

i64
Float80::toInt64() const
{
    return sf::floatx80_to_int64(toSF(*this));
}

i64
Float80::toInt64Truncated() const
{
    return sf::floatx80_to_int64_round_to_zero(toSF(*this));
}

float
Float80::toFloat() const
{
    return std::bit_cast<float>((u32)sf::floatx80_to_float32(toSF(*this)));
}

double
Float80::toDouble() const
{
    return std::bit_cast<double>((u64)sf::floatx80_to_float64(toSF(*this)));
}

long double
Float80::toLongDouble() const
{
    if constexpr (LDBL_MANT_DIG == 64 && std::endian::native == std::endian::little) {

        long double result = 0;
        std::memcpy(&result, &man, sizeof(man));
        std::memcpy(reinterpret_cast<char *>(&result) + sizeof(man), &se, sizeof(se));
        return result;

    } else {

        return (long double)toDouble();
    }
}

string
Float80::str() const
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "0x%04X:%016llX", (unsigned)se, (unsigned long long)man);
    return buf;
}


//
// Float80: Arithmetic
//

Float80
Float80::operator-() const
{
    // Flipping the sign is exact and raises no exception
    return fromBits(u16(se ^ 0x8000), man);
}

Float80
Float80::abs() const
{
    return fromBits(u16(se & 0x7FFF), man);
}

Float80
Float80::operator+(const Float80 &rhs) const
{
    return fromSF(sf::floatx80_add(toSF(*this), toSF(rhs)));
}

Float80
Float80::operator-(const Float80 &rhs) const
{
    return fromSF(sf::floatx80_sub(toSF(*this), toSF(rhs)));
}

Float80
Float80::operator*(const Float80 &rhs) const
{
    return fromSF(sf::floatx80_mul(toSF(*this), toSF(rhs)));
}

Float80
Float80::operator/(const Float80 &rhs) const
{
    return fromSF(sf::floatx80_div(toSF(*this), toSF(rhs)));
}

Float80
Float80::sqrt() const
{
    return fromSF(sf::floatx80_sqrt(toSF(*this)));
}

Float80
Float80::rem(const Float80 &rhs) const
{
    return fromSF(sf::floatx80_rem(toSF(*this), toSF(rhs)));
}

Float80
Float80::roundToInt() const
{
    return fromSF(sf::floatx80_round_to_int(toSF(*this)));
}

Float80
Float80::scale(const Float80 &rhs) const
{
    // NaNs propagate (and signaling NaNs raise the invalid flag)
    if (isNaN() || rhs.isNaN()) return *this + rhs;

    // Scaling by an infinite factor is undefined
    if (rhs.isInfinite()) {
        FloatEnv::raiseFlags(FloatFlags::INVALID);
        return quietNaN();
    }

    // Zeros and infinities are not changed
    if (isZero() || isInfinite()) return *this;

    // Determine the scale factor. Factors beyond the exponent range of the
    // format are clamped, because they overflow or underflow anyway.
    i32 n;
    if (rhs.exponent() >= 20) {
        n = rhs.sign() ? -0x100000 : 0x100000;
    } else {
        // Truncating discards the fraction without being inexact
        auto saved = FloatEnv::flags();
        n = rhs.toInt32Truncated();
        FloatEnv::clearFlags();
        FloatEnv::raiseFlags(saved);
    }

    // Normalize the mantissa. Denormals have the exponent of the smallest
    // normal number.
    i32 e = biasedExponent() ? biasedExponent() : 1;
    u64 m = man;
    int shift = std::countl_zero(m);
    m <<= shift;
    e -= shift;

    // Let SoftFloat do the rounding and the exception handling
    return fromSF(sf::roundAndPackFloatx80(sf::floatx80_rounding_precision,
                                           sign(), e + n, m, 0));
}


//
// Float80: Comparing
//

bool
Float80::operator==(const Float80 &rhs) const
{
    return sf::floatx80_eq(toSF(*this), toSF(rhs));
}

bool
Float80::operator<(const Float80 &rhs) const
{
    return sf::floatx80_lt(toSF(*this), toSF(rhs));
}

bool
Float80::operator<=(const Float80 &rhs) const
{
    return sf::floatx80_le(toSF(*this), toSF(rhs));
}

}
