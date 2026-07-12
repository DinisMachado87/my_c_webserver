#include "Hex.hpp"
#include <cstring>
#include <gtest/gtest.h>

/* helpers */
static void expectNumToHex(size_t val, const char *expected, size_t expectedLen)
{
	Hex hex;
	hex.parseFromNum(val);
	EXPECT_EQ(hex.value(), val);
	EXPECT_EQ(hex.strLen(), expectedLen);
	EXPECT_EQ(std::memcmp(hex.str(), expected, expectedLen), 0);
}

static void expectParseHex(const char *str, size_t len, size_t expected)
{
	Hex hex;
	EXPECT_TRUE(hex.parseFromStr(str, len));
	EXPECT_EQ(hex.value(), expected);
}

static void expectParseFails(const char *str, size_t len)
{
	Hex hex;
	EXPECT_FALSE(hex.parseFromStr(str, len));
}

/* numToHex */
TEST(HexTest, NumToHexZero) { expectNumToHex(0, "0", 1); }
TEST(HexTest, NumToHexSingleDigit) { expectNumToHex(15, "f", 1); }
TEST(HexTest, NumToHexMultiDigit) { expectNumToHex(255, "ff", 2); }
TEST(HexTest, NumToHexLargeValue) { expectNumToHex(65536, "10000", 5); }
TEST(HexTest, NumToHexUsesLowercase) { expectNumToHex(0xabcdef, "abcdef", 6); }

/* parseHex */
TEST(HexTest, ParseHexSingleDigit) { expectParseHex("a", 1, 10); }
TEST(HexTest, ParseHexMultiDigit) { expectParseHex("1a3", 3, 0x1a3); }
TEST(HexTest, ParseHexUppercase) { expectParseHex("FF", 2, 255); }
TEST(HexTest, ParseHexMixedCase) { expectParseHex("aB", 2, 0xab); }
TEST(HexTest, ParseHexZero) { expectParseHex("0", 1, 0); }
TEST(HexTest, ParseHexLeadingZeros) { expectParseHex("00ff", 4, 255); }

/* parseHex errors */
TEST(HexTest, ParseHexEmptyFails) { expectParseFails("", 0); }
TEST(HexTest, ParseHexTooLongFails)
{
	expectParseFails("12345678901234567", 17);
}
TEST(HexTest, ParseHexInvalidCharFails) { expectParseFails("1g", 2); }
TEST(HexTest, ParseHexSpaceFails) { expectParseFails("1 ", 2); }
TEST(HexTest, ParseHexNegativeSignFails) { expectParseFails("-1", 2); }

/* unterminated buffer */
TEST(HexTest, ParseHexRespectsLen)
{
	const char buf[] = "ffZZZZ";
	expectParseHex(buf, 2, 255);
}

/* roundtrip */
static void expecNumToStrToNum(size_t val)
{
	Hex a;
	a.parseFromNum(val);
	Hex b;
	ASSERT_TRUE(b.parseFromStr(a.str(), a.strLen()));
	EXPECT_EQ(b.value(), val);
}

TEST(HexTest, RoundtripSmall) { expecNumToStrToNum(42); }
TEST(HexTest, RoundtripLarge) { expecNumToStrToNum(1048576); }
TEST(HexTest, RoundtripMax32Bits) { expecNumToStrToNum(0xffffffff); }
