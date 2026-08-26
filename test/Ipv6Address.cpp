#include <qsox/Ipv4Address.hpp>
#include <qsox/Ipv6Address.hpp>
#include <gtest/gtest.h>

using namespace qsox;

static Ipv6Address mustParse(std::string_view str) {
    auto result = Ipv6Address::parse(str);
    EXPECT_TRUE(result.isOk()) << "Expected \"" << str << "\" to parse";
    return result.isOk() ? result.unwrap() : Ipv6Address{};
}

static void expectFail(std::string_view str) {
    auto result = Ipv6Address::parse(str);
    EXPECT_TRUE(result.isErr()) << "Expected \"" << str << "\" to fail parsing";
}

TEST(Ipv6Parse, FullForm) {
    EXPECT_EQ(mustParse("1:2:3:4:5:6:7:8"), Ipv6Address(1, 2, 3, 4, 5, 6, 7, 8));
}

TEST(Ipv6Parse, Unspecified) {
    EXPECT_EQ(mustParse("::"), Ipv6Address::UNSPECIFIED);
    EXPECT_EQ(mustParse("0:0:0:0:0:0:0:0"), Ipv6Address::UNSPECIFIED);
    EXPECT_EQ(mustParse("0000::0000"), Ipv6Address::UNSPECIFIED);
    EXPECT_EQ(mustParse("0::0"), Ipv6Address::UNSPECIFIED);
    EXPECT_EQ(mustParse("0::"), Ipv6Address::UNSPECIFIED);
    EXPECT_EQ(mustParse("::0"), Ipv6Address::UNSPECIFIED);
}

TEST(Ipv6Parse, Loopback) {
    EXPECT_EQ(mustParse("::1"), Ipv6Address::LOCALHOST);
    EXPECT_EQ(mustParse("0:0:0:0:0:0:0:1"), Ipv6Address::LOCALHOST);
}

TEST(Ipv6Parse, Compression) {
    EXPECT_EQ(mustParse("2001:db8::1"), Ipv6Address(0x2001, 0x0db8, 0, 0, 0, 0, 0, 1));
    EXPECT_EQ(mustParse("2001:db8::"), Ipv6Address(0x2001, 0x0db8, 0, 0, 0, 0, 0, 0));
    EXPECT_EQ(mustParse("::1:2:3:4:5:6:7"), Ipv6Address(0, 1, 2, 3, 4, 5, 6, 7));
    EXPECT_EQ(mustParse("1:2:3:4:5:6:7::"), Ipv6Address(1, 2, 3, 4, 5, 6, 7, 0));
}

TEST(Ipv6Parse, NonCanonical) {
    // uncompressed / leading-zero / uppercase forms that normalize to the same address
    EXPECT_EQ(mustParse("2001:db8:0:1:0:0:0:1"), Ipv6Address(0x2001, 0x0db8, 0, 1, 0, 0, 0, 1));
    EXPECT_EQ(mustParse("2001:0db8:0000:0001:0000:0000:0000:0001"), Ipv6Address(0x2001, 0x0db8, 0, 1, 0, 0, 0, 1));
    EXPECT_EQ(mustParse("2001:DB8:0:0001::1"), Ipv6Address(0x2001, 0x0db8, 0, 1, 0, 0, 0, 1));
}

TEST(Ipv6Parse, Ipv4Mapped) {
    Ipv4Address v4(192, 168, 1, 1);

    Ipv6Address addr = mustParse("::ffff:192.168.1.1");
    EXPECT_EQ(addr, Ipv6Address::fromIpv4Mapped(v4));

    auto mapped = addr.toIpv4Mapped();
    ASSERT_TRUE(mapped.has_value());
    EXPECT_EQ(*mapped, v4);
}

TEST(Ipv6Parse, Ipv4Compatible) {
    // deprecated form, but inet_pton accepts it
    EXPECT_EQ(mustParse("::192.168.1.1"), Ipv6Address(0, 0, 0, 0, 0, 0, 0xc0a8, 0x0101));
}

TEST(Ipv6Parse, EmbeddedIpv4) {
    EXPECT_EQ(mustParse("64:ff9b::192.0.2.33"), Ipv6Address(0x0064, 0xff9b, 0, 0, 0, 0, 0xc000, 0x0221));
    EXPECT_EQ(mustParse("2001:db8::192.0.2.33"), Ipv6Address(0x2001, 0x0db8, 0, 0, 0, 0, 0xc000, 0x0221));
}

TEST(Ipv6Parse, Invalid) {
    expectFail("");
    expectFail(":");
    expectFail(":::");
    expectFail("::1:");
    expectFail(":1::");
    expectFail("1:2:3:4:5:6:7");
    expectFail("1:2:3:4:5:6:7:8:9");
    expectFail("12345::");
    expectFail("1::2::3");
    expectFail("g::1");
    expectFail("2001:db8:::1");
    expectFail("::ffff:192.168.1");
    expectFail("::ffff:192.168.1.256");
    expectFail("::ffff:1.2.3.4.5");
    expectFail("fe80::1%eth0");
}
