// test_Writer.cpp
#include "Writer.hpp"
#include <cstring>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

/* Writer dispatch — FILE via pipe, SOCKET via socketpair.
 * writeOne and writeGather each branch on fdType. */
class WriterTest : public ::testing::Test
{
protected:
	int pipeFds[2];
	int sockFds[2];

	void SetUp()
	{
		ASSERT_EQ(pipe(pipeFds), 0);
		ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockFds), 0);
	}
	void TearDown()
	{
		closeFd(pipeFds[0]);
		closeFd(pipeFds[1]);
		closeFd(sockFds[0]);
		closeFd(sockFds[1]);
	}

	static void closeFd(int &fd)
	{
		if (fd != -1)
			close(fd);
		fd = -1;
	}

	void expectRead(int fd, const char *want, size_t n)
	{
		char buf[64];
		ASSERT_EQ(read(fd, buf, n), static_cast<ssize_t>(n));
		EXPECT_EQ(std::memcmp(buf, want, n), 0);
	}

	// const_cast: iov_base is void*, literals are const.
	static void makeIov(struct iovec iov[2], const char *a, const char *b)
	{
		iov[0].iov_base = const_cast<char *>(a);
		iov[0].iov_len = std::strlen(a);
		iov[1].iov_base = const_cast<char *>(b);
		iov[1].iov_len = std::strlen(b);
	}
};

/* writeOne */

TEST_F(WriterTest, WriteOneFileToPipe)
{
	Writer writer(Writer::FILE, pipeFds[1]);
	ASSERT_EQ(writer.writeOne("file", 4), 4);
	expectRead(pipeFds[0], "file", 4);
}

TEST_F(WriterTest, WriteOneSocketToPeer)
{
	Writer writer(Writer::SOCKET, sockFds[0]);
	ASSERT_EQ(writer.writeOne("sock", 4), 4);
	expectRead(sockFds[1], "sock", 4);
}

TEST_F(WriterTest, WriteOneClosedFdReturnsNegative)
{
	closeFd(pipeFds[1]);

	Writer writer(Writer::FILE, pipeFds[1]);
	EXPECT_LT(writer.writeOne("x", 1), 0);
}

/* writeGather */

TEST_F(WriterTest, WriteGatherFileConcatenates)
{
	struct iovec iov[2];
	makeIov(iov, "ab", "cd");

	Writer writer(Writer::FILE, pipeFds[1]);
	ASSERT_EQ(writer.writeGather(iov, 2), 4);
	expectRead(pipeFds[0], "abcd", 4);
}

TEST_F(WriterTest, WriteGatherSocketConcatenates)
{
	struct iovec iov[2];
	makeIov(iov, "ab", "cd");

	Writer writer(Writer::SOCKET, sockFds[0]);
	ASSERT_EQ(writer.writeGather(iov, 2), 4);
	expectRead(sockFds[1], "abcd", 4);
}
