#include "BufferManager.hpp"
#include "Reader.hpp"
#include "Segment.hpp"
#include "Writer.hpp"
#include "webServ.hpp"
#include <cstring>
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>

/* SegmentTest */
// Segment constructor is private (pool-owned). Build through BufferManager.
class SegmentTest : public ::testing::Test
{
protected:
	BufferManager pool;
	Segment *seg;
	// SetUp is gtest's hook — exact name matters, no override in C++98 to catch
	// a typo.
	void SetUp() { seg = pool.getSegment(); }

	// Assert view holds exactly n bytes matching src.
	void expectBytes(StrView v, const char *src, size_t n)
	{
		ASSERT_EQ(v.size(), n);
		EXPECT_EQ(std::memcmp(v.data(), src, n), 0);
	}
};

TEST_F(SegmentTest, CopyInWritesActualBytes)
{
	const char *src = "hello world";
	size_t n = seg->copyIn(src, 11);
	ASSERT_EQ(n, 11u);
	expectBytes(seg->writtenView(), src, 11);
}

TEST_F(SegmentTest, CopyInTruncatesToWritable)
{
	std::string big(RECV_SIZE + 100, 'x');
	size_t n = seg->copyIn(big.data(), big.size());
	ASSERT_EQ(n, static_cast<size_t>(RECV_SIZE));
	ASSERT_EQ(seg->writable(), 0u);
	expectBytes(seg->writtenView(), big.data(), RECV_SIZE);
}

TEST_F(SegmentTest, CopyInResumeAccumulatesOrdered)
{
	seg->copyIn("abc", 3);
	seg->copyIn("def", 3);
	expectBytes(seg->writtenView(), "abcdef", 6);
}

TEST_F(SegmentTest, LastWrittenContent)
{
	seg->copyIn("abcdef", 6);
	expectBytes(seg->lastWritten(3), "def", 3);
}

TEST_F(SegmentTest, AllSentExactBoundary)
{
	seg->copyIn("ab", 2);
	EXPECT_FALSE(seg->allSent());
}

/* Reader path — real pipe */
class SegmentPipeTest : public ::testing::Test
{
protected:
	BufferManager pool;
	Segment *seg;
	int fds[2];
	// SetUp is gtest's hook — exact name matters, no override in C++98 to catch
	// a typo.
	void SetUp()
	{
		ASSERT_EQ(pipe(fds), 0);
		seg = pool.getSegment();
	}
	void TearDown()
	{
		if (fds[0] != -1)
			close(fds[0]);
		if (fds[1] != -1)
			close(fds[1]);
	}

	// Write n bytes into the pipe's write end, assert full write.
	void writePipe(const char *src, size_t n)
	{
		ASSERT_EQ(write(fds[1], src, n), static_cast<ssize_t>(n));
	}

	// Read n bytes from pipe's read end into buf, assert full read.
	// Blocks if fewer than n buffered — only safe when writer put >= n first.
	void readPipe(char *buf, size_t n)
	{
		ASSERT_EQ(read(fds[0], buf, n), static_cast<ssize_t>(n));
	}

	// Assert view holds exactly n bytes matching src.
	void expectBytes(StrView v, const char *src, size_t n)
	{
		ASSERT_EQ(v.size(), n);
		EXPECT_EQ(std::memcmp(v.data(), src, n), 0);
	}
};

TEST_F(SegmentPipeTest, ReadFromReads)
{
	const char *src = "piped";
	writePipe(src, 5);
	Reader reader(FD_FILE, fds[0]);
	ssize_t bytesRead = seg->readFrom(reader);
	ASSERT_EQ(bytesRead, 5);
	expectBytes(seg->writtenView(), src, 5);
}

TEST_F(SegmentPipeTest, SecondReadFromAppends)
{
	Reader reader(FD_FILE, fds[0]);
	writePipe("aaa", 3);
	seg->readFrom(reader);
	writePipe("bbb", 3);
	seg->readFrom(reader);
	expectBytes(seg->writtenView(), "aaabbb", 6);
}

TEST_F(SegmentPipeTest, ReadFromFullSegNoSyscall)
{
	std::string big(RECV_SIZE, 'x');
	seg->copyIn(big.data(), RECV_SIZE);
	ASSERT_EQ(seg->writable(), 0u);
	Reader reader(FD_FILE, fds[0]);
	writePipe("yyy", 3);
	ssize_t bytesRead = seg->readFrom(reader);
	EXPECT_EQ(bytesRead, 0);
	expectBytes(seg->writtenView(), big.data(), RECV_SIZE);
}

TEST_F(SegmentPipeTest, ReadFromClosedFd)
{
	close(fds[0]);
	fds[0] = -1;
	Reader reader(FD_FILE, fds[0]);
	ssize_t bytesRead = seg->readFrom(reader);
	EXPECT_LT(bytesRead, 0);
}

TEST_F(SegmentPipeTest, SendToDrainsOrdered)
{
	seg->copyIn("sendme", 6);
	Writer writer(FD_FILE, fds[1]);
	ASSERT_EQ(seg->sendTo(writer), 6);
	char buf[16];
	readPipe(buf, 6);
	expectBytes(StrView(buf, 6), "sendme", 6);
}

// Full drain only — pipe accepts all 6 at once, can't force short write here.
TEST_F(SegmentPipeTest, SendToEmptiesBufferWhenSpace)
{
	seg->copyIn("abcdef", 6);
	Writer writer(FD_FILE, fds[1]);
	seg->sendTo(writer);
	char buf[16];
	readPipe(buf, 6);
	EXPECT_EQ(seg->unsentView().size(), 0u);
}

TEST_F(SegmentPipeTest, SendToEmptyNoSyscall)
{
	Writer writer(FD_FILE, fds[1]);
	ssize_t bytesSent = seg->sendTo(writer);
	EXPECT_EQ(bytesSent, 0);
}
