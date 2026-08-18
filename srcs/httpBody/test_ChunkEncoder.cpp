#include "BufferManager.hpp"
#include "ChunkEncoder.hpp"
#include "Hex.hpp"
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>

using std::string;

// hex CRLF data CRLF for the first `size` bytes of `data`.
static string expectedChunk(size_t size, const string &data)
{
	Hex hex;
	hex.parseFromNum(size);
	return string(hex.str(), hex.strLen()) + "\r\n" + data.substr(0, size)
		   + "\r\n";
}

/* Encoder writes a FILE fd (writev) over a pipe. Covers framing and
 * lifecycle; whole writes are fine at this size. */
class ChunkEncoderTest : public ::testing::Test
{
protected:
	BufferManager _pool;
	int _in[2];	 // encoder reads _in[0]
	int _out[2]; // encoder writes _out[1], test reads _out[0]

	// Exposes _curSegment for the pooling assertion — no public getter.
	class TestableEncoder : public ChunkEncoder
	{
	public:
		TestableEncoder(int inFd, int outFd, BufferManager &pool) :
			ChunkEncoder(Reader(FD_FILE, inFd), Writer(FD_FILE, outFd), pool)
		{
		}
		bool holdsSegment() const { return _curSegment != NULL; }
	};

	TestableEncoder *_encoder;

	void SetUp()
	{
		signal(SIGPIPE, SIG_IGN);
		ASSERT_EQ(pipe(_in), OK);
		ASSERT_EQ(pipe(_out), OK);
		ASSERT_EQ(fcntl(_in[0], F_SETFL, O_NONBLOCK), OK);
		ASSERT_EQ(fcntl(_out[0], F_SETFL, O_NONBLOCK), OK);
		ASSERT_EQ(fcntl(_out[1], F_SETFL, O_NONBLOCK), OK);
		_encoder = new TestableEncoder(_in[0], _out[1], _pool);
	}

	void TearDown()
	{
		delete _encoder; // before closeFd — destructor returns segment to pool
		closeFd(_in[0]);
		closeFd(_in[1]);
		closeFd(_out[0]);
		closeFd(_out[1]);
	}

	static void closeFd(int &fd)
	{
		if (fd != -1)
			close(fd);
		fd = -1;
	}

	void feed(const char *src, size_t n)
	{
		ASSERT_EQ(write(_in[1], src, n), static_cast<ssize_t>(n));
	}
	void closeInput() { closeFd(_in[1]); }

	string readOut()
	{
		string out;
		char buf[RECV_SIZE];
		ssize_t n;
		while ((n = read(_out[0], buf, sizeof(buf))) > 0)
			out.append(buf, static_cast<size_t>(n));
		return out;
	}

	// Drains out pipe between calls — pipe capacity is smaller than a
	// multi-segment body, so a plain loop would wedge on EAGAIN.
	string encode()
	{
		string out;
		while (_encoder->readIn() > 0)
			;
		while (!_encoder->done()) {
			_encoder->writeOut();
			out += readOut();
		}
		return out + readOut();
	}
};

/* Framing */

TEST_F(ChunkEncoderTest, FramesSingleSegmentBody)
{
	feed("hello", 5);
	closeInput();
	EXPECT_EQ(encode(), "5\r\nhello\r\n0\r\n\r\n");
}

TEST_F(ChunkEncoderTest, EmptyBodyEofSendsOnlyTerminator)
{
	closeInput();
	EXPECT_EQ(encode(), "0\r\n\r\n");
}

TEST_F(ChunkEncoderTest, MultiSegmentBodyFramesPerSegment)
{
	string big(RECV_SIZE + 10, 'z');
	feed(big.data(), big.size());
	closeInput();
	string out = encode();

	string firstChunk = expectedChunk(RECV_SIZE, big);
	string secondChunk = expectedChunk(10, big);
	EXPECT_EQ(out, firstChunk + secondChunk + "0\r\n\r\n");
}

/* Lifecycle */

TEST_F(ChunkEncoderTest, NotDoneBeforeTerminatorSent)
{
	feed("data", 4);
	_encoder->readIn(); // input still open, terminator not armed
	_encoder->writeOut();
	EXPECT_FALSE(_encoder->done());
}

TEST_F(ChunkEncoderTest, DoneAfterTerminator)
{
	closeInput();
	_encoder->readIn();
	while (!_encoder->done())
		_encoder->writeOut();
	EXPECT_TRUE(_encoder->done());
}

TEST_F(ChunkEncoderTest, WriteOutEmptyOpenInputReturnsZero)
{
	EXPECT_EQ(_encoder->writeOut(), 0);
	EXPECT_FALSE(_encoder->done());
}

TEST_F(ChunkEncoderTest, WriteOutAfterDoneReturnsZero)
{
	closeInput();
	_encoder->readIn();
	while (!_encoder->done())
		_encoder->writeOut();
	EXPECT_EQ(_encoder->writeOut(), 0);
}

TEST_F(ChunkEncoderTest, SingleShotFramesOneSegmentPerCall)
{
	feed("aa", 2);
	feed("bb", 2);
	closeInput();
	_encoder->readIn();
	_encoder->writeOut();
	EXPECT_EQ(readOut(), "4\r\naabb\r\n");
}

TEST_F(ChunkEncoderTest, NoSegmentAfterDone)
{
	feed("x", 1);
	closeInput();
	_encoder->readIn();
	_encoder->writeOut();
	EXPECT_FALSE(_encoder->holdsSegment());
}
