#include "BufferManager.hpp"
#include "StackBuffer.hpp"
#include <cstring>
#include <gtest/gtest.h>

class StackBufferTest : public ::testing::Test
{
protected:
	BufferManager bm;
	StackBuffer stack;

	StackBufferTest() :
		stack(bm)
	{
	}
};

TEST_F(StackBufferTest, FreshStackIsEmpty)
{
	EXPECT_TRUE(stack.empty());
	EXPECT_EQ(stack.totalLen(), 0u);
}

TEST_F(StackBufferTest, PopBackFromEmptyReturnsNull)
{
	EXPECT_TRUE(stack.popBack() == NULL);
}

TEST_F(StackBufferTest, PushAndTopReturnsLastPushed)
{
	Segment *a = bm.getBuffer();
	Segment *b = bm.getBuffer();
	a->copyIn("first", 5);
	b->copyIn("second", 6);

	stack.pushBack(a);
	stack.pushBack(b);

	Segment *popped = stack.popBack();
	EXPECT_EQ(popped, b);
	bm.returnBuffers(popped);
}

TEST_F(StackBufferTest, PopBackReturnsLIFOOrder)
{
	Segment *a = bm.getBuffer();
	Segment *b = bm.getBuffer();
	Segment *c = bm.getBuffer();
	a->copyIn("a", 1);
	b->copyIn("b", 1);
	c->copyIn("c", 1);

	stack.pushBack(a);
	stack.pushBack(b);
	stack.pushBack(c);

	EXPECT_EQ(stack.popBack(), c);
	EXPECT_EQ(stack.popBack(), b);
	EXPECT_EQ(stack.popBack(), a);
	EXPECT_TRUE(stack.empty());
}

TEST_F(StackBufferTest, TotalLenTracksAcrossSegments)
{
	Segment *a = bm.getBuffer();
	Segment *b = bm.getBuffer();
	a->copyIn("hello", 5);
	b->copyIn("world!!", 7);

	stack.pushBack(a);
	EXPECT_EQ(stack.totalLen(), 5u);

	stack.pushBack(b);
	EXPECT_EQ(stack.totalLen(), 12u);

	bm.returnBuffers(stack.popBack());
	EXPECT_EQ(stack.totalLen(), 5u);
}
