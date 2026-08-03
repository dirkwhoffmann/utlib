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

using Algo = Hashable::HashAlgorithm;

namespace {

// A minimal Hashable, so the instance-side wrappers can be exercised
struct HashableBuffer : Hashable {

    std::vector<u8> data;

    explicit HashableBuffer(const std::string &s) : data(s.begin(), s.end()) { }

    u64 hash(Algo algorithm) const override {
        return Hashable::hash(data.data(), isize(data.size()), algorithm);
    }
};

// A minimal Dumpable over a fixed buffer
struct DumpableBuffer : Dumpable {

    std::vector<u8> data;

    explicit DumpableBuffer(std::vector<u8> d) : data(std::move(d)) { }

    DataProvider dataProvider() const override {
        return Dumpable::dataProvider(data.data(), isize(data.size()));
    }
};

std::vector<u8> ascending(isize n) {
    std::vector<u8> v;
    for (isize i = 0; i < n; i++) v.push_back(u8(i));
    return v;
}

}

TEST_CASE("Hashable: FNV-1a") {

    SUBCASE("The seeds are the published FNV-1a offset bases") {

        CHECK(Hashable::fnvInit32() == 0x811c9dc5);
        CHECK(Hashable::fnvInit64() == 0xcbf29ce484222325);
    }

    SUBCASE("A single iteration is deterministic and mixes the input") {

        auto a = Hashable::fnvIt32(Hashable::fnvInit32(), 0x41);
        auto b = Hashable::fnvIt32(Hashable::fnvInit32(), 0x41);
        auto c = Hashable::fnvIt32(Hashable::fnvInit32(), 0x42);

        CHECK(a == b);
        CHECK(a != c);
        CHECK(a != Hashable::fnvInit32());

        auto a64 = Hashable::fnvIt64(Hashable::fnvInit64(), 0x41);
        CHECK(a64 == Hashable::fnvIt64(Hashable::fnvInit64(), 0x41));
        CHECK(a64 != Hashable::fnvIt64(Hashable::fnvInit64(), 0x42));
    }

    SUBCASE("Hashing a buffer is deterministic") {

        const u8 data[] = { 'u', 't', 'l', 'i', 'b' };

        CHECK(Hashable::fnv32(data, 5) == Hashable::fnv32(data, 5));
        CHECK(Hashable::fnv64(data, 5) == Hashable::fnv64(data, 5));
    }

    SUBCASE("Different inputs hash differently") {

        const u8 a[] = { 1, 2, 3 };
        const u8 b[] = { 1, 2, 4 };

        CHECK(Hashable::fnv32(a, 3) != Hashable::fnv32(b, 3));
        CHECK(Hashable::fnv64(a, 3) != Hashable::fnv64(b, 3));
    }

    SUBCASE("Length is part of the input") {

        const u8 data[] = { 1, 2, 3, 4 };

        CHECK(Hashable::fnv32(data, 3) != Hashable::fnv32(data, 4));
    }

    SUBCASE("An empty or null buffer hashes to zero, not to the seed") {

        /* A deliberate early return rather than the FNV convention, which
         * would leave the offset basis untouched. Pinned down because callers
         * treating 0 as "nothing to hash" depend on it.
         */
        const u8 data[] = { 0 };

        CHECK(Hashable::fnv32(data, 0) == 0);
        CHECK(Hashable::fnv64(data, 0) == 0);
        CHECK(Hashable::fnv32(nullptr, 4) == 0);
        CHECK(Hashable::fnv64(nullptr, 4) == 0);
    }
}

TEST_CASE("Hashable: CRC") {

    SUBCASE("CRC32 matches the known value for \"123456789\"") {

        // The standard check value for CRC-32/ISO-HDLC
        const u8 data[] = { '1','2','3','4','5','6','7','8','9' };

        CHECK(Hashable::crc32(data, 9) == 0xCBF43926);
    }

    SUBCASE("CRC is deterministic and input-sensitive") {

        const u8 a[] = { 'a', 'b', 'c' };
        const u8 b[] = { 'a', 'b', 'd' };

        CHECK(Hashable::crc16(a, 3) == Hashable::crc16(a, 3));
        CHECK(Hashable::crc32(a, 3) == Hashable::crc32(a, 3));
        CHECK(Hashable::crc16(a, 3) != Hashable::crc16(b, 3));
        CHECK(Hashable::crc32(a, 3) != Hashable::crc32(b, 3));
    }

    SUBCASE("CRC16 stays inside 16 bits") {

        const u8 data[] = { 'x', 'y', 'z' };

        CHECK(Hashable::crc16(data, 3) <= 0xFFFF);
    }
}

TEST_CASE("Hashable: dispatch and instance wrappers") {

    const u8 data[] = { 'u', 't', 'l' };

    SUBCASE("hash() dispatches to the requested algorithm") {

        CHECK(Hashable::hash(data, 3, Algo::FNV32) == Hashable::fnv32(data, 3));
        CHECK(Hashable::hash(data, 3, Algo::FNV64) == Hashable::fnv64(data, 3));
        CHECK(Hashable::hash(data, 3, Algo::CRC16) == Hashable::crc16(data, 3));
        CHECK(Hashable::hash(data, 3, Algo::CRC32) == Hashable::crc32(data, 3));
    }

    SUBCASE("The instance wrappers agree with the static functions") {

        HashableBuffer buf("utl");

        CHECK(buf.fnv32() == Hashable::fnv32(data, 3));
        CHECK(buf.fnv64() == Hashable::fnv64(data, 3));
        CHECK(buf.crc16() == Hashable::crc16(data, 3));
        CHECK(buf.crc32() == Hashable::crc32(data, 3));
    }
}

TEST_CASE("Dumpable: data providers") {

    SUBCASE("A provider reads back what it was given") {

        auto data = ascending(4);
        auto provider = Dumpable::dataProvider(data.data(), 4);

        // (offset, size) -> value, and nothing past the end
        CHECK(provider(0, 1) == 0);
        CHECK(provider(3, 1) == 3);
        CHECK_FALSE(provider(4, 1).has_value());
    }

    SUBCASE("The span overload behaves the same") {

        auto data = ascending(4);
        auto provider = Dumpable::dataProvider(std::span<const u8>(data));

        CHECK(provider(0, 1) == 0);
        CHECK(provider(3, 1) == 3);
        CHECK_FALSE(provider(4, 1).has_value());
    }

    SUBCASE("An empty buffer yields nothing") {

        auto provider = Dumpable::dataProvider(nullptr, 0);
        CHECK_FALSE(provider(0, 1).has_value());
    }
}

TEST_CASE("Dumpable: rendering") {

    DumpableBuffer buf(ascending(32));

    SUBCASE("hexDump writes hex and honours the column count") {

        std::ostringstream os;
        buf.hexDump(os);
        auto text = os.str();

        CHECK_FALSE(text.empty());
        CHECK(text.find("00") != std::string::npos);
        CHECK(text.find("1f") != std::string::npos);   // last byte, 31

        // 32 bytes at 16 columns is two lines
        CHECK(std::count(text.begin(), text.end(), '\n') == 2);
    }

    SUBCASE("memDump adds an ASCII column, hexDump does not") {

        DumpableBuffer text({ 'A', 'B', 'C', 'D' });

        std::ostringstream hex, mem;
        text.hexDump(hex);
        text.memDump(mem);

        CHECK(mem.str().find("ABCD") != std::string::npos);
        CHECK(hex.str().find("ABCD") == std::string::npos);
    }

    SUBCASE("ascDump renders the characters") {

        DumpableBuffer text({ 'h', 'e', 'l', 'l', 'o' });

        std::ostringstream os;
        text.ascDump(os);

        CHECK(os.str().find("hello") != std::string::npos);
    }

    SUBCASE("txtDump renders the characters too") {

        DumpableBuffer text({ 'w', 'o', 'r', 'l', 'd' });

        std::ostringstream os;
        text.txtDump(os);

        CHECK(os.str().find("world") != std::string::npos);
    }

    SUBCASE("An empty buffer produces no rows") {

        DumpableBuffer empty({});

        std::ostringstream os;
        empty.hexDump(os);

        CHECK(os.str().empty());
    }

    SUBCASE("The explicit option/format overload is usable") {

        std::ostringstream narrow, wide;

        buf.dump(narrow, DumpOpt { .base = 16 },
                 DumpFmt { .size = 'b', .columns = 8, .groups = 1,
                           .nr = false, .offset = true, .ascii = false });
        buf.dump(wide, DumpOpt { .base = 16 },
                 DumpFmt { .size = 'b', .columns = 16, .groups = 1,
                           .nr = false, .offset = true, .ascii = false });

        /* Compared against each other rather than against an absolute line
         * count: halving the column count has to lengthen the dump, and that
         * relationship is what 'columns' means. Both must still show the last
         * byte, so nothing is dropped either way.
         */
        CHECK(narrow.str().size() > wide.str().size());
        CHECK(narrow.str().find("1f") != std::string::npos);
        CHECK(wide.str().find("1f") != std::string::npos);

        // The offset column advances by the column count
        CHECK(narrow.str().find("00000008") != std::string::npos);
        CHECK(wide.str().find("00000010") != std::string::npos);
    }

    SUBCASE("DumpFmt::fmt translates the options to a format string") {

        DumpFmt f { .size = 'b', .columns = 4, .groups = 0,
                    .nr = false, .offset = true, .ascii = true };

        CHECK_FALSE(f.fmt().empty());
    }
}
