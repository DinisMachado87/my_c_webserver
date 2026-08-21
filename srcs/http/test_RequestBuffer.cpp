#include "BufferManager.hpp"
#include "RequestBuffer.hpp"
#include "Segment.hpp"
#include "webServ.hpp"
#include <cstring>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <unistd.h>

class RequestBufferTest : public ::testing::Test
{
protected:
	BufferManager _pool;
	int _fds[2];

	void SetUp()
	{
		// Assert that error does not come from pipe or fcntl.
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

	void feed(const std::string &s) { feed(s.data(), s.size()); }

	void closeInput() { closeFd(_fds[1]); }

	// _active is private and _segList protected — subclass to assert on
	// retention without widening the production interface.
	class TestableRequestBuffer : public RequestBuffer
	{
	public:
		TestableRequestBuffer(int fd, BufferManager &pool) :
			RequestBuffer(Reader(FD_FILE, fd), pool)
		{
		}

		// Segments handed over to the chain by a carry relocation.
		bool retainedEmpty() const { return _segList.empty(); }

		std::string retainedStr() const
		{
			std::ostringstream os;
			os << _segList;
			return os.str();
		}
	};
};

/* readIn — plain fill, no carry */

TEST_F(RequestBufferTest, ReadInFillsActiveSegment)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	feed("hello", 5);
	EXPECT_EQ(buf.readIn(), 5);
	EXPECT_EQ(buf.unparsed(), StrView("hello"));
}

TEST_F(RequestBufferTest, SuccessiveReadsAppendContiguously)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	feed("GET /in", 7);
	ASSERT_EQ(buf.readIn(), 7);
	feed("dex HTTP/1.1\r\n", 14);
	ASSERT_EQ(buf.readIn(), 14);
	EXPECT_EQ(buf.unparsed(), StrView("GET /index HTTP/1.1\r\n"));
	EXPECT_TRUE(buf.retainedEmpty());
}

/* consumed — parser cursor */

TEST_F(RequestBufferTest, ConsumedShrinksUnparsedFromFront)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	feed("abcdef", 6);
	ASSERT_EQ(buf.readIn(), 6);
	buf.consumed(3);
	EXPECT_EQ(buf.unparsed(), StrView("def"));
}

TEST_F(RequestBufferTest, ConsumedAllLeavesEmptyView)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	feed("abc", 3);
	ASSERT_EQ(buf.readIn(), 3);
	buf.consumed(3);
	EXPECT_TRUE(buf.unparsed().empty());
}

TEST_F(RequestBufferTest, ReadAfterConsumedAppendsBehindUnparsed)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	feed("abc", 3);
	ASSERT_EQ(buf.readIn(), 3);
	buf.consumed(2); // "c" is an incomplete token
	feed("de", 2);
	ASSERT_EQ(buf.readIn(), 2);
	EXPECT_EQ(buf.unparsed(), StrView("cde"));
}

/* EOF / EAGAIN — readIn's error contract */

TEST_F(RequestBufferTest, EofSetsInClosed)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	closeInput();
	EXPECT_EQ(buf.readIn(), 0);
	EXPECT_TRUE(buf.inClosed());
}

TEST_F(RequestBufferTest, NotClosedBeforeEof)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	feed("x", 1);
	ASSERT_EQ(buf.readIn(), 1);
	EXPECT_FALSE(buf.inClosed());
}

TEST_F(RequestBufferTest, EmptyNonBlockingFdReturnsNegative)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	EXPECT_LT(buf.readIn(), 0); // EAGAIN, not EOF
	EXPECT_FALSE(buf.inClosed());
}

TEST_F(RequestBufferTest, ClosedFdReturnsNegative)
{
	closeFd(_fds[0]);
	TestableRequestBuffer buf(_fds[0], _pool);
	EXPECT_LT(buf.readIn(), 0);
	EXPECT_FALSE(buf.inClosed());
}

TEST_F(RequestBufferTest, NoRelocationWhileRoomRemains)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	std::string fill(RECV_SIZE - RECV_SIZE / 3,
					 'a'); // writable() == RECV_SIZE/3
	feed(fill);
	ASSERT_EQ(buf.readIn(), static_cast<ssize_t>(fill.size()));

	feed("z", 1);
	EXPECT_EQ(buf.readIn(), 1);
	EXPECT_TRUE(buf.retainedEmpty()); // resumed in place, no copy
	EXPECT_EQ(buf.unparsed().size(), fill.size() + 1);
}

TEST_F(RequestBufferTest, RelocatesBelowThreshold)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	std::string fill(RECV_SIZE - RECV_SIZE / 3 + 1, 'a'); // one byte under
	feed(fill);
	ASSERT_EQ(buf.readIn(), static_cast<ssize_t>(fill.size()));

	feed("z", 1);
	EXPECT_EQ(buf.readIn(), 1);
	EXPECT_FALSE(buf.retainedEmpty());
}

/* Carry — leftover moves to the front of a fresh segment */

TEST_F(RequestBufferTest, CarryMakesLeftoverContiguousWithNewBytes)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	std::string first(RECV_SIZE - 10, 'a');
	feed(first);
	ASSERT_EQ(buf.readIn(), static_cast<ssize_t>(first.size()));
	buf.consumed(first.size() - 4); // 4 bytes of an unfinished token

	feed("bbbb", 4);
	ASSERT_EQ(buf.readIn(), 4);
	EXPECT_EQ(buf.unparsed(), StrView("aaaabbbb"));
}

TEST_F(RequestBufferTest, CarryRetainsOldSegmentForParsedViews)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	std::string first(RECV_SIZE - 10, 'a');
	feed(first);
	ASSERT_EQ(buf.readIn(), static_cast<ssize_t>(first.size()));

	StrView parsedToken(buf.unparsed().data(), 5); // as a sub-parser would keep
	buf.consumed(first.size() - 4);

	feed("bbbb", 4);
	ASSERT_EQ(buf.readIn(), 4);

	EXPECT_EQ(parsedToken, StrView("aaaaa")); // segment not recycled/poisoned
	EXPECT_EQ(buf.retainedStr(), first);
}

TEST_F(RequestBufferTest, EmptyLeftoverCarriesNothing)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	std::string first(RECV_SIZE - 10, 'a');
	feed(first);
	ASSERT_EQ(buf.readIn(), static_cast<ssize_t>(first.size()));
	buf.consumed(first.size()); // parser ate everything

	feed("bbbb", 4);
	ASSERT_EQ(buf.readIn(), 4);
	EXPECT_EQ(buf.unparsed(), StrView("bbbb"));
	EXPECT_FALSE(buf.retainedEmpty());
}

TEST_F(RequestBufferTest, RepeatedRelocationsChainSegments)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	std::string chunk(RECV_SIZE - 10, 'a');

	for (int i = 0; i < 3; i++) {
		feed(chunk);
		ASSERT_GT(buf.readIn(), 0);
		buf.consumed(buf.unparsed().size() - 4); // always leave a partial token
	}

	EXPECT_EQ(buf.unparsed().size(), 4u);
	EXPECT_EQ(buf.retainedStr().size(), 2 * (RECV_SIZE - 6));
}

/* Oversize — token bigger than one segment */

TEST_F(RequestBufferTest, UnconsumedFullSegmentReportsCarryOverflow)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	std::string big(RECV_SIZE, 'x');
	feed(big);
	ASSERT_EQ(buf.readIn(), static_cast<ssize_t>(RECV_SIZE));

	feed("y", 1);
	EXPECT_EQ(buf.readIn(), RequestBuffer::CARRY_OVERFLOW);
	EXPECT_FALSE(buf.inClosed()); // overflow is not EOF
}

TEST_F(RequestBufferTest, OverflowRecoversOnceParserConsumes)
{
	TestableRequestBuffer buf(_fds[0], _pool);
	std::string big(RECV_SIZE, 'x');
	feed(big);
	ASSERT_EQ(buf.readIn(), static_cast<ssize_t>(RECV_SIZE));
	feed("y", 1);
	ASSERT_EQ(buf.readIn(), RequestBuffer::CARRY_OVERFLOW);

	buf.consumed(RECV_SIZE);
	EXPECT_EQ(buf.readIn(), 1);
	EXPECT_EQ(buf.unparsed(), StrView("y"));
}
