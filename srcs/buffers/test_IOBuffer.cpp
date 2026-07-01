#include "BufferManager.hpp"
#include "Exception.hpp"
#include "IOBuffer.hpp"
#include <cstring>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

class IOBufferTest : public ::testing::Test
{
protected:
	BufferManager bm;
	int _inFds[2];
	int _outFds[2];

	void SetUp()
	{
		_inFds[0] = _inFds[1] = -1;
		_outFds[0] = _outFds[1] = -1;
		createPairs();
	}

	void TearDown()
	{
		closePair(_inFds);
		closePair(_outFds);
	}

	void closePair(int *fds)
	{
		if (fds[0] >= 0)
			close(fds[0]);
		if (fds[1] >= 0)
			close(fds[1]);
		fds[0] = fds[1] = -1;
	}

	void createPairs()
	{
		ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, _inFds), 0);
		ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, _outFds), 0);
	}

	void feedInput(const char *data, size_t len)
	{
		ASSERT_EQ(write(_inFds[1], data, len), static_cast<ssize_t>(len));
	}

	ssize_t readOutput(char *buf, size_t len)
	{
		return read(_outFds[0], buf, len);
	}
};

// --- readIn ---

TEST_F(IOBufferTest, ReadInReturnsDataAsStrView)
{
	IOBuffer buf(IOBuffer::SOCK_TO_FILE, _inFds[0], _outFds[1], bm);

	feedInput("hello", 5);
	StrView sv = buf.readIn();

	EXPECT_EQ(sv.size(), 5u);
	EXPECT_EQ(std::memcmp(sv.data(), "hello", 5), 0);
}

TEST_F(IOBufferTest, ReadInUpdatesTotalLen)
{
	IOBuffer buf(IOBuffer::SOCK_TO_FILE, _inFds[0], _outFds[1], bm);

	feedInput("hello", 5);
	buf.readIn();

	EXPECT_EQ(buf.totalLen(), 5u);
}

TEST_F(IOBufferTest, MultipleReadInsAccumulate)
{
	IOBuffer buf(IOBuffer::SOCK_TO_FILE, _inFds[0], _outFds[1], bm);

	feedInput("abc", 3);
	buf.readIn();
	feedInput("defgh", 5);
	buf.readIn();

	EXPECT_EQ(buf.totalLen(), 8u);
}

TEST_F(IOBufferTest, ReadInThrowsOnPeerClose)
{
	IOBuffer buf(IOBuffer::SOCK_TO_FILE, _inFds[0], _outFds[1], bm);

	close(_inFds[1]);
	_inFds[1] = -1;

	EXPECT_THROW(buf.readIn(), ClientClosed);
}

// --- writeOut ---

TEST_F(IOBufferTest, WriteOutOnEmptyReturnsZero)
{
	IOBuffer buf(IOBuffer::SOCK_TO_FILE, _inFds[0], _outFds[1], bm);

	EXPECT_EQ(buf.writeOut(), 0);
}

TEST_F(IOBufferTest, WriteOutSendsCorrectData)
{
	IOBuffer buf(IOBuffer::SOCK_TO_FILE, _inFds[0], _outFds[1], bm);

	feedInput("hello", 5);
	buf.readIn();

	ssize_t n = buf.writeOut();
	EXPECT_EQ(n, 5);

	char out[16];
	ssize_t r = readOutput(out, sizeof(out));
	EXPECT_EQ(r, 5);
	EXPECT_EQ(std::memcmp(out, "hello", 5), 0);
}

TEST_F(IOBufferTest, WriteOutPopsSegmentWhenDone)
{
	IOBuffer buf(IOBuffer::SOCK_TO_FILE, _inFds[0], _outFds[1], bm);

	feedInput("hello", 5);
	buf.readIn();
	buf.writeOut();

	EXPECT_TRUE(buf.empty());
	EXPECT_EQ(buf.totalLen(), 0u);
}

// --- ensureSpace ---

TEST_F(IOBufferTest, EnsureSpaceAllocatesWhenSegmentHalfFull)
{
	IOBuffer buf(IOBuffer::SOCK_TO_FILE, _inFds[0], _outFds[1], bm);

	// Fill first segment past half capacity
	size_t halfPlus = RECV_SIZE / 2 + 1;
	char big[RECV_SIZE / 2 + 1];
	std::memset(big, 'A', halfPlus);
	feedInput(big, halfPlus);
	buf.readIn();

	// Next readIn should trigger a new segment
	feedInput("more", 4);
	StrView sv = buf.readIn();

	EXPECT_EQ(buf.totalLen(), halfPlus + 4);
	EXPECT_EQ(sv.size(), 4u);
}

// --- full cycle ---

TEST_F(IOBufferTest, FullReadWriteCycle)
{
	IOBuffer buf(IOBuffer::SOCK_TO_FILE, _inFds[0], _outFds[1], bm);

	feedInput("hello world", 11);
	buf.readIn();

	EXPECT_FALSE(buf.empty());
	EXPECT_EQ(buf.totalLen(), 11u);

	buf.writeOut();

	EXPECT_TRUE(buf.empty());
	EXPECT_EQ(buf.totalLen(), 0u);

	char out[16];
	ssize_t r = readOutput(out, sizeof(out));
	EXPECT_EQ(r, 11);
	EXPECT_EQ(std::memcmp(out, "hello world", 11), 0);
}

// --- Type wiring ---

TEST_F(IOBufferTest, FileToSockReadsAndSends)
{
	IOBuffer buf(IOBuffer::FILE_TO_SOCK, _inFds[0], _outFds[1], bm);

	feedInput("payload", 7);
	buf.readIn();
	buf.writeOut();

	char out[16];
	ssize_t r = readOutput(out, sizeof(out));
	EXPECT_EQ(r, 7);
	EXPECT_EQ(std::memcmp(out, "payload", 7), 0);
}

TEST_F(IOBufferTest, RequestTypeCanRead)
{
	IOBuffer buf(IOBuffer::REQUEST, _inFds[0], -1, bm);

	feedInput("GET / HTTP/1.1\r\n", 16);
	StrView sv = buf.readIn();

	EXPECT_EQ(sv.size(), 16u);
	EXPECT_EQ(buf.totalLen(), 16u);
}
