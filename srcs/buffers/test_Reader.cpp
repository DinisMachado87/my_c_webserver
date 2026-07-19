// test_Reader.cpp
#include "Reader.hpp"
#include <cstring>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

/* Reader — FILE via pipe, SOCKET via socketpair, NONE no syscall. */
class ReaderTest : public ::testing::Test
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

	// Fill through reader, assert byte count and content match.
	void expectFill(Reader &reader, const char *want, size_t n)
	{
		char buf[64];
		ssize_t got = reader.fill(buf, sizeof(buf));
		ASSERT_EQ(got, static_cast<ssize_t>(n));
		EXPECT_EQ(std::memcmp(buf, want, n), 0);
	}
};

TEST_F(ReaderTest, FileReadsFromPipe)
{
	ASSERT_EQ(write(pipeFds[1], "file", 4), 4);
	Reader reader(Reader::FILE, pipeFds[0]);
	expectFill(reader, "file", 4);
}

TEST_F(ReaderTest, SocketReceivesFromPeer)
{
	ASSERT_EQ(send(sockFds[1], "sock", 4, 0), 4);
	Reader reader(Reader::SOCKET, sockFds[0]);
	expectFill(reader, "sock", 4);
}

TEST_F(ReaderTest, NoneReturnsZero)
{
	Reader reader(Reader::NONE, pipeFds[0]);
	char buf[8];
	EXPECT_EQ(reader.fill(buf, sizeof(buf)), 0);
}

TEST_F(ReaderTest, ClosedFdReturnsNegative)
{
	closeFd(pipeFds[0]);
	Reader reader(Reader::FILE, pipeFds[0]);
	char buf[8];
	EXPECT_LT(reader.fill(buf, sizeof(buf)), 0);
}
