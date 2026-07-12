#include "BufferManager.hpp"
#include "OutBuffer.hpp"
#include "StrView.hpp"
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

class OutBufferTest : public ::testing::Test
{
protected:
	BufferManager bm;
	int sockFd[2];

	void SetUp() { ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockFd), 0); }

	void TearDown()
	{
		if (sockFd[0] >= 0)
			close(sockFd[0]);
		if (sockFd[1] >= 0)
			close(sockFd[1]);
	}
};

TEST_F(OutBufferTest, WriteOutOnEmptyReturnsZero)
{
	OutBuffer buf(sockFd[1], bm);
	EXPECT_EQ(buf.writeOut(), 0);
}

TEST_F(OutBufferTest, WriteOutSendsCorrectData)
{
	OutBuffer buf(sockFd[1], bm);
	buf.append("hello", 5);
	buf.writeOut();

	char out[16] = {};
	ssize_t n = read(sockFd[0], out, sizeof(out));
	EXPECT_EQ(n, 5);
	EXPECT_TRUE(StrView(out, n) == "hello");
}

TEST_F(OutBufferTest, WriteOutPopsSegmentWhenDone)
{
	OutBuffer buf(sockFd[1], bm);
	buf.append("hello", 5);
	buf.writeOut();
	EXPECT_TRUE(buf.empty());
	EXPECT_EQ(buf.totalLen(), 0u);
}
