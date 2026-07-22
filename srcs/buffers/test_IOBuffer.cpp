#include "BufferManager.hpp"
#include "IOBuffer.hpp"
#include "Segment.hpp"
#include "webServ.hpp"
#include <cstring>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class IOBufferTest : public ::testing::Test
{
protected:
	BufferManager _pool;
	int _inFds[2];
	int _outFds[2];

	void SetUp()
	{
		signal(SIGPIPE, SIG_IGN);
		// Assert that error does not come from pipe or fcntl.
		ASSERT_EQ(pipe(_inFds), OK);
		ASSERT_EQ(pipe(_outFds), OK);
		ASSERT_EQ(fcntl(_inFds[0], F_SETFL, O_NONBLOCK), OK);
		ASSERT_EQ(fcntl(_outFds[0], F_SETFL, O_NONBLOCK), OK);
	}

	void TearDown()
	{
		closeFd(_inFds[0]);
		closeFd(_inFds[1]);
		closeFd(_outFds[0]);
		closeFd(_outFds[1]);
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
		ASSERT_EQ(write(_inFds[1], src, n), static_cast<ssize_t>(n));
	}

	void closeInput() { closeFd(_inFds[1]); }

	// Drain everything the buffer wrote out. Read end nonblocking, so
	// returns what is there without waiting.
	std::string dataSent()
	{
		std::string out;
		char scratch[RECV_SIZE];
		ssize_t bytesRead;
		while ((bytesRead = read(_outFds[0], scratch, sizeof(scratch))) > 0)
			out.append(scratch, static_cast<size_t>(bytesRead));
		return out;
	}

	// IOBuffer exposes no chain getter — subclass for assertions, same
	// pattern as the IBuffer tests.
	class TestableIObuffer : public IOBuffer
	{
	public:
		TestableIObuffer(int inFd, int outFd, BufferManager &pool) :
			IOBuffer(inFd, Reader::FILE, outFd, IOBuffer::FILE, pool)
		{
		}

		Segment::e_comparison compareChain(const StrView &expected) const
		{
			return _segList.compare(expected);
		}

		std::string dataRead()
		{
			std::ostringstream os;
			os << _segList;
			return os.str();
		}
	};
};

/* writeOut */

TEST_F(IOBufferTest, WriteOutSendsBufferedBytes)
{
	TestableIObuffer buf(_inFds[0], _outFds[1], _pool);
	feed("hello", 5);
	buf.readIn();
	EXPECT_EQ(buf.writeOut(), 5);
	EXPECT_EQ(dataSent(), "hello");
}

TEST_F(IOBufferTest, WriteOutEmptyChainReturnsZero)
{
	TestableIObuffer buf(_inFds[0], _outFds[1], _pool);
	EXPECT_EQ(buf.writeOut(), 0);
}

TEST_F(IOBufferTest, WriteOutOneSegmentPerCall)
{
	TestableIObuffer buf(_inFds[0], _outFds[1], _pool);
	std::string big(RECV_SIZE + 100, 'z');
	feed(big.data(), big.size());
	buf.readIn();
	buf.readIn(); // second segment
	EXPECT_EQ(buf.writeOut(), static_cast<ssize_t>(RECV_SIZE));
	EXPECT_EQ(dataSent().size(), static_cast<size_t>(RECV_SIZE));
}

TEST_F(IOBufferTest, SuccessiveWriteOutsDrainChain)
{
	TestableIObuffer buf(_inFds[0], _outFds[1], _pool);
	std::string big(RECV_SIZE + 100, 'z');
	feed(big.data(), big.size());
	closeInput();

	while (buf.readIn() > 0)
		;
	while (buf.writeOut() > 0)
		;
	EXPECT_EQ(dataSent(), big);
}

/* Segment recycling */

TEST_F(IOBufferTest, SentSegmentReturnsToPool)
{
	TestableIObuffer buf(_inFds[0], _outFds[1], _pool);
	feed("abc", 3);
	buf.readIn();
	buf.writeOut();
	EXPECT_EQ(buf.dataRead(), ""); // chain empty, segment pooled
}

TEST_F(IOBufferTest, PartialSendKeepsSegmentAtHead)
{
	TestableIObuffer buf(_inFds[0], _outFds[1], _pool);
	std::string big(RECV_SIZE, 'y');
	feed(big.data(), big.size());
	buf.readIn();

	closeFd(_outFds[0]); // reader gone, pipe write fails
	EXPECT_LT(buf.writeOut(), 0);
	EXPECT_EQ(buf.compareChain(big), Segment::MATCH); // still buffered
}

/* done() */

TEST_F(IOBufferTest, NotDoneUntilDrained)
{
	TestableIObuffer buf(_inFds[0], _outFds[1], _pool);
	feed("data", 4);
	buf.readIn();
	closeInput();
	buf.readIn(); // EOF
	EXPECT_FALSE(buf.done());
	buf.writeOut();
	EXPECT_TRUE(buf.done());
}

/* Relay */

TEST_F(IOBufferTest, RelayPreservesOrder)
{
	TestableIObuffer buf(_inFds[0], _outFds[1], _pool);
	feed("aaa", 3);
	buf.readIn();
	buf.writeOut();
	feed("bbb", 3);
	buf.readIn();
	buf.writeOut();
	EXPECT_EQ(dataSent(), "aaabbb");
}

/* SOCKET kind — send/sendmsg path instead of write/writev */

TEST_F(IOBufferTest, SocketKindSendsToPeer)
{
	int sockFds[2];
	ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockFds), OK);

	IOBuffer buf(_inFds[0], Reader::FILE, sockFds[0], IOBuffer::SOCKET, _pool);
	feed("sock", 4);
	buf.readIn();
	EXPECT_EQ(buf.writeOut(), 4);

	char scratch[16];
	ASSERT_EQ(read(sockFds[1], scratch, 4), 4);
	EXPECT_EQ(std::memcmp(scratch, "sock", 4), 0);

	closeFd(sockFds[0]);
	closeFd(sockFds[1]);
}
