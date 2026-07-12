#include "BufferManager.hpp"
#include "ClientClosed.hpp"
#include "InBuffer.hpp"
#include "StrView.hpp"
#include <gtest/gtest.h>
#include <unistd.h>

class InBufferTest : public ::testing::Test
{
protected:
	BufferManager bm;
	int pipeFd[2];

	void SetUp() { ASSERT_EQ(pipe(pipeFd), 0); }

	void TearDown()
	{
		if (pipeFd[0] >= 0)
			close(pipeFd[0]);
		if (pipeFd[1] >= 0)
			close(pipeFd[1]);
	}
};

TEST_F(InBufferTest, ReadInReturnsDataAsStrView)
{
	InBuffer buf(pipeFd[0], ::read, bm);
	write(pipeFd[1], "hello", 5);
	StrView sv = buf.readIn();
	EXPECT_EQ(sv.size(), 5u);
	EXPECT_TRUE(sv == "hello");
}

TEST_F(InBufferTest, ReadInUpdatesTotalLen)
{
	InBuffer buf(pipeFd[0], ::read, bm);
	write(pipeFd[1], "hello", 5);
	buf.readIn();
	EXPECT_EQ(buf.totalLen(), 5u);
}

TEST_F(InBufferTest, MultipleReadInsAccumulate)
{
	InBuffer buf(pipeFd[0], ::read, bm);
	write(pipeFd[1], "aaa", 3);
	buf.readIn();
	write(pipeFd[1], "bb", 2);
	buf.readIn();
	EXPECT_EQ(buf.totalLen(), 5u);
}

TEST_F(InBufferTest, ReadInThrowsOnPeerClose)
{
	InBuffer buf(pipeFd[0], ::read, bm);
	close(pipeFd[1]);
	pipeFd[1] = -1;
	EXPECT_THROW(buf.readIn(), ClientClosed);
}

TEST_F(InBufferTest, PeekFrontReturnsData)
{
	InBuffer buf(pipeFd[0], ::read, bm);
	write(pipeFd[1], "hello", 5);
	buf.readIn();
	const Segment *seg = buf.peekFront();
	ASSERT_TRUE(seg != NULL);
	EXPECT_EQ(seg->readable(), 5u);
}

TEST_F(InBufferTest, DiscardFrontClearsTotalLen)
{
	InBuffer buf(pipeFd[0], ::read, bm);
	write(pipeFd[1], "hello", 5);
	buf.readIn();
	buf.discardFront();
	EXPECT_TRUE(buf.empty());
	EXPECT_EQ(buf.totalLen(), 0u);
}
