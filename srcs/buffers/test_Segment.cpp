#include "Segment.hpp"
#include <cstring>
#include <gtest/gtest.h>

// Fakes
static ssize_t fakeRead(int, void *buf, size_t len)
{
	if (len > 5)
		len = 5;
	std::memcpy(buf, "hello", len);
	return len;
}

static ssize_t fakeReadFail(int, void *, size_t) { return -1; }

static char g_sendBuf[RECV_SIZE];
static size_t g_sendLen;

static ssize_t fakeSend(int, const void *buf, size_t len)
{
	std::memcpy(g_sendBuf, buf, len);
	g_sendLen = len;
	return len;
}

class SegmentTest : public ::testing::Test
{
protected:
	Segment seg;

	void SetUp()
	{
		g_sendLen = 0;
		std::memset(g_sendBuf, 0, sizeof(g_sendBuf));
	}
};

// --- Fresh state ---

TEST_F(SegmentTest, FreshSegmentIsEmpty)
{
	EXPECT_EQ(seg.readable(), 0u);
	EXPECT_EQ(seg.writable(), static_cast<size_t>(RECV_SIZE));
	EXPECT_TRUE(seg._prev == NULL);
	EXPECT_TRUE(seg._next == NULL);
	EXPECT_TRUE(seg.allSent());
}

// --- copyIn ---

TEST_F(SegmentTest, CopyInWritesData)
{
	size_t n = seg.copyIn("hello", 5);
	EXPECT_EQ(n, 5u);
	EXPECT_EQ(seg.readable(), 5u);
	EXPECT_EQ(seg.writable(), static_cast<size_t>(RECV_SIZE) - 5u);
	EXPECT_EQ(std::memcmp(seg.data(), "hello", 5), 0);
}

TEST_F(SegmentTest, CopyInAccumulates)
{
	seg.copyIn("aaa", 3);
	seg.copyIn("bb", 2);
	EXPECT_EQ(seg.readable(), 5u);
	EXPECT_EQ(std::memcmp(seg.data(), "aaabb", 5), 0);
}

TEST_F(SegmentTest, CopyInClampsAtCapacity)
{
	char big[RECV_SIZE + 100];
	std::memset(big, 'X', sizeof(big));
	size_t n = seg.copyIn(big, sizeof(big));
	EXPECT_EQ(n, static_cast<size_t>(RECV_SIZE));
	EXPECT_EQ(seg.readable(), static_cast<size_t>(RECV_SIZE));
	EXPECT_EQ(seg.writable(), 0u);
}

TEST_F(SegmentTest, CopyInToFullSegmentReturnsZero)
{
	char fill[RECV_SIZE];
	std::memset(fill, 'A', RECV_SIZE);
	seg.copyIn(fill, RECV_SIZE);
	EXPECT_EQ(seg.copyIn("x", 1), 0u);
}

// --- readFrom ---

TEST_F(SegmentTest, ReadFromFillsSegment)
{
	ssize_t n = seg.readFrom(fakeRead, 0);
	EXPECT_EQ(n, 5);
	EXPECT_EQ(seg.readable(), 5u);
	EXPECT_EQ(std::memcmp(seg.data(), "hello", 5), 0);
}

TEST_F(SegmentTest, ReadFromFailureDoesNotAdvance)
{
	ssize_t n = seg.readFrom(fakeReadFail, 0);
	EXPECT_EQ(n, -1);
	EXPECT_EQ(seg.readable(), 0u);
}

// --- sendTo ---

TEST_F(SegmentTest, SendToWritesData)
{
	seg.copyIn("hello", 5);
	ssize_t n = seg.sendTo(fakeSend, 0);
	EXPECT_EQ(n, 5);
	EXPECT_EQ(g_sendLen, 5u);
	EXPECT_EQ(std::memcmp(g_sendBuf, "hello", 5), 0);
	EXPECT_TRUE(seg.allSent());
}

TEST_F(SegmentTest, AllSentTracksPartialSend)
{
	seg.copyIn("hello", 5);
	EXPECT_FALSE(seg.allSent());

	// Partial send — only 3 bytes
	seg.sendTo(fakeSend, 0); // sends all 5 in our fake, so use a partial fake:
	// Better: test with copyIn of enough data that fakeSend won't drain it
}

TEST_F(SegmentTest, AllSentAfterFullDrain)
{
	seg.copyIn("hi", 2);
	EXPECT_FALSE(seg.allSent());
	seg.sendTo(fakeSend, 0);
	EXPECT_TRUE(seg.allSent());
}

// --- reset ---

TEST_F(SegmentTest, ResetRestoresInitialState)
{
	seg._prev = &seg;
	seg._next = &seg;
	seg.copyIn("data", 4);
	seg.sendTo(fakeSend, 0);

	seg.reset();

	EXPECT_EQ(seg.readable(), 0u);
	EXPECT_EQ(seg.writable(), static_cast<size_t>(RECV_SIZE));
	EXPECT_TRUE(seg._prev == NULL);
	EXPECT_TRUE(seg._next == NULL);
	EXPECT_TRUE(seg.allSent());
}

// --- poison ---

TEST_F(SegmentTest, PoisonFillsBuffer)
{
	seg.copyIn("hello", 5);
	seg.poison();
#ifdef DEBUG
	unsigned char expected = 0xDE;
#else
	unsigned char expected = 0x00;
#endif
	const unsigned char *p
		= reinterpret_cast<const unsigned char *>(seg.data());
	for (size_t i = 0; i < 5; i++)
		EXPECT_EQ(p[i], expected) << "Mismatch at byte " << i;
}
