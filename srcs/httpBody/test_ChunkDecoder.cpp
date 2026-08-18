#include "BufferManager.hpp"
#include "ChunkDecoder.hpp"
#include "Hex.hpp"
#include "HttpStatus.hpp"
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>

using std::string;

class ChunkDecoderTest : public ::testing::Test
{
protected:
	BufferManager _pool;
	int _in[2];	 // decoder reads _in[0]
	int _out[2]; // decoder writes _out[1], test reads _out[0]

	class TestableDecoder : public ChunkDecoder
	{
	public:
		TestableDecoder(int inFd, int outFd, BufferManager &pool) :
			ChunkDecoder(Reader(FD_FILE, inFd), Writer(FD_FILE, outFd), pool)
		{
		}
		bool holdsSegment() const { return _curSegment != NULL; }
	};

	TestableDecoder *_decoder;

	void SetUp()
	{
		signal(SIGPIPE, SIG_IGN);
		ASSERT_EQ(pipe(_in), OK);
		ASSERT_EQ(pipe(_out), OK);
		ASSERT_EQ(fcntl(_in[0], F_SETFL, O_NONBLOCK), OK);
		ASSERT_EQ(fcntl(_out[0], F_SETFL, O_NONBLOCK), OK);
		ASSERT_EQ(fcntl(_out[1], F_SETFL, O_NONBLOCK), OK);
		_decoder = new TestableDecoder(_in[0], _out[1], _pool);
	}

	void TearDown()
	{
		delete _decoder;
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

	void feed(const string &s)
	{
		ASSERT_EQ(write(_in[1], s.data(), s.size()),
				  static_cast<ssize_t>(s.size()));
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

	// Drains out pipe between calls — deframed body can exceed pipe capacity.
	string decode()
	{
		string out;
		while (_decoder->readIn() > 0)
			;
		while (!_decoder->done()) {
			_decoder->writeOut();
			out += readOut();
		}
		return out + readOut();
	}

	// Wraps one payload as a single chunk: hex CRLF data CRLF.
	static string chunk(const string &data)
	{
		Hex hex;
		hex.parseFromNum(data.size());
		return string(hex.str(), hex.strLen()) + "\r\n" + data + "\r\n";
	}
	static string terminator() { return "0\r\n\r\n"; }
};

/* Deframing */

TEST_F(ChunkDecoderTest, SingleChunkYieldsPayload)
{
	feed(chunk("hello") + terminator());
	closeInput();
	EXPECT_EQ(decode(), "hello");
}

TEST_F(ChunkDecoderTest, MultipleChunksConcatenate)
{
	feed(chunk("foo") + chunk("bar") + chunk("baz") + terminator());
	closeInput();
	EXPECT_EQ(decode(), "foobarbaz");
}

TEST_F(ChunkDecoderTest, EmptyBodyJustTerminator)
{
	feed(terminator());
	closeInput();
	EXPECT_EQ(decode(), "");
}

TEST_F(ChunkDecoderTest, PayloadLargerThanOneSegment)
{
	string big(RECV_SIZE + 500, 'z');
	feed(chunk(big) + terminator());
	closeInput();
	EXPECT_EQ(decode(), big);
}

TEST_F(ChunkDecoderTest, ManySmallChunksInOneSegment)
{
	string framed;
	string expected;
	for (int i = 0; i < 20; i++) {
		framed += chunk("ab");
		expected += "ab";
	}
	feed(framed + terminator());
	closeInput();
	EXPECT_EQ(decode(), expected);
}

/* Lenient newline */

TEST_F(ChunkDecoderTest, BareLfAccepted)
{
	feed("5\nhello\n0\n\n");
	closeInput();
	EXPECT_EQ(decode(), "hello");
}

/* done() / lifecycle */

TEST_F(ChunkDecoderTest, DoneAfterTerminator)
{
	feed(chunk("x") + terminator());
	closeInput();
	while (_decoder->readIn() > 0)
		;
	while (!_decoder->done()) {
		_decoder->writeOut();
		readOut();
	}
	EXPECT_TRUE(_decoder->done());
}

TEST_F(ChunkDecoderTest, NotDoneMidBody)
{
	feed(chunk("data")); // no terminator, input left open
	while (_decoder->readIn() > 0)
		;
	_decoder->writeOut();
	readOut();
	EXPECT_FALSE(_decoder->done());
}

/* Malformed framing throws 400 */

TEST_F(ChunkDecoderTest, NonHexSizeThrows)
{
	feed("zz\r\n");
	closeInput();
	_decoder->readIn();
	EXPECT_THROW(_decoder->writeOut(), HttpStatus);
}

TEST_F(ChunkDecoderTest, OversizeLineThrows)
{
	feed(string(20, 'f') + "\r\n"); // 20 hex digits, scratch caps at 16
	closeInput();
	_decoder->readIn();
	EXPECT_THROW(_decoder->writeOut(), HttpStatus);
}

TEST_F(ChunkDecoderTest, BadTrailerByteThrows)
{
	feed("5\r\nhelloXX"); // XX where CRLF expected after payload
	closeInput();
	_decoder->readIn();
	EXPECT_THROW(_decoder->writeOut(), HttpStatus);
}
