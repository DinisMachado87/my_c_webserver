#include "BufferManager.hpp"
#include "IBuffer.hpp"
#include "Segment.hpp"
#include "webServ.hpp"
#include <cstring>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <unistd.h>

class IBufferTest : public ::testing::Test
{
protected:
	BufferManager _pool;
	int _fds[2];

	void SetUp()
	{
		ASSERT_EQ(pipe(_fds), OK);
		ASSERT_EQ(fcntl(_fds[0], F_SETFL, O_NONBLOCK), OK);
	}

	void TearDown()
	{
		closeFd(_fds[0]);
		closeFd(_fds[1]);
	}

	static void closeFd(int &fd)
	{
		if (fd != -1)
			close(fd);
		fd = -1;
	}

	void feed(const char *src, size_t n)
	{
		// Assert that error does not come from write.
		ASSERT_EQ(write(_fds[1], src, n), static_cast<ssize_t>(n));
	}

	void closeInput() { closeFd(_fds[1]); }

	// IBuffer has no public content getter — subclass to expose the chain
	// for assertions via the same operator<< the SegmentList tests use.
	class TestableIbuffer : public IBuffer
	{
	public:
		TestableIbuffer(int fd, BufferManager &pool) :
			IBuffer(fd, Reader::FILE, pool)
		{
		}

		std::string drained()
		{
			std::ostringstream os;
			os << _segList;
			return os.str();
		}
	};
};

/* readIn */

TEST_F(IBufferTest, ReadInFillsFromFd)
{
	TestableIbuffer buf(_fds[0], _pool);
	feed("hello", 5);
	EXPECT_EQ(buf.readIn(), 5);
	EXPECT_EQ(buf.drained(), "hello");
}

TEST_F(IBufferTest, ReadInReturnsBytesRead)
{
	TestableIbuffer buf(_fds[0], _pool);
	feed("abcd", 4);
	EXPECT_EQ(buf.readIn(), 4);
}

/* EOF handling */

TEST_F(IBufferTest, EofSetsInClosed)
{
	TestableIbuffer buf(_fds[0], _pool);
	closeInput();
	EXPECT_EQ(buf.readIn(), 0);
	EXPECT_TRUE(buf.inClosed());
}

TEST_F(IBufferTest, NotClosedBeforeEof)
{
	TestableIbuffer buf(_fds[0], _pool);
	feed("x", 1);
	buf.readIn();
	EXPECT_FALSE(buf.inClosed());
}

/* done() */

TEST_F(IBufferTest, NotDoneWithBufferedBytes)
{
	TestableIbuffer buf(_fds[0], _pool);
	feed("data", 4);
	buf.readIn();
	closeInput();
	buf.readIn(); // EOF, but chain still holds "data"
	EXPECT_TRUE(buf.inClosed());
	EXPECT_FALSE(buf.done());
}

TEST_F(IBufferTest, DoneWhenClosedAndEmpty)
{
	TestableIbuffer buf(_fds[0], _pool);
	closeInput();
	buf.readIn(); // EOF, nothing ever buffered
	EXPECT_TRUE(buf.done());
}

/* Multi-call accumulation — one segment per readIn, appended in order */

TEST_F(IBufferTest, SuccessiveReadsAppend)
{
	TestableIbuffer buf(_fds[0], _pool);
	feed("aaa", 3);
	buf.readIn();
	feed("bbb", 3);
	buf.readIn();
	EXPECT_EQ(buf.drained(), "aaabbb");
}

/* Segment rollover — fill past RECV_SIZE spans multiple segments */

TEST_F(IBufferTest, FillPastSegmentSpansChain)
{
	TestableIbuffer buf(_fds[0], _pool);
	std::string big(RECV_SIZE + 100, 'z');
	feed(big.data(), big.size());

	// Pipe may not deliver all at once; drain until inClosed.
	ssize_t bytesRead;
	do {
		bytesRead = buf.readIn();
	} while (bytesRead > 0);
	closeInput();

	EXPECT_EQ(buf.drained().size(), big.size());
}

/* Error path — closed read fd returns negative, no EOF semantics */

TEST_F(IBufferTest, ClosedFdReturnsNegative)
{
	closeFd(_fds[0]);
	TestableIbuffer buf(_fds[0], _pool);
	EXPECT_LT(buf.readIn(), 0);
	EXPECT_FALSE(buf.inClosed()); // <0 is error, not EOF
}
