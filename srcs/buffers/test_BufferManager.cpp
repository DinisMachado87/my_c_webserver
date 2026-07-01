#include "BufferManager.hpp"
#include "Segment.hpp"
#include <gtest/gtest.h>
#include <set>

class BufferManagerTest : public ::testing::Test
{
protected:
	BufferManager bm;
};

TEST_F(BufferManagerTest, GetBufferReturnsNonNull)
{
	Segment *seg = bm.getBuffer();
	ASSERT_TRUE(seg != NULL);
}

TEST_F(BufferManagerTest, GetBufferReturnsResetSegment)
{
	Segment *seg = bm.getBuffer();
	EXPECT_EQ(seg->readable(), 0u);
	EXPECT_TRUE(seg->_prev == NULL);
	EXPECT_TRUE(seg->_next == NULL);
	EXPECT_TRUE(seg->allSent());
}

TEST_F(BufferManagerTest, GetBufferTwiceReturnsDifferentPointers)
{
	Segment *a = bm.getBuffer();
	Segment *b = bm.getBuffer();
	EXPECT_NE(a, b);
}

TEST_F(BufferManagerTest, ReturnedBufferIsReused)
{
	Segment *first = bm.getBuffer();
	bm.returnBuffers(first);
	Segment *second = bm.getBuffer();
	EXPECT_EQ(first, second);
}

TEST_F(BufferManagerTest, ReturnedBufferIsClean)
{
	Segment *seg = bm.getBuffer();
	seg->copyIn("dirty", 5);
	bm.returnBuffers(seg);

	Segment *reused = bm.getBuffer();
	EXPECT_EQ(reused->readable(), 0u);
	EXPECT_TRUE(reused->_prev == NULL);
	EXPECT_TRUE(reused->_next == NULL);
}

TEST_F(BufferManagerTest, ReturnNullDoesNotCrash)
{
	EXPECT_TRUE(bm.returnBuffers(NULL) == NULL);
}

TEST_F(BufferManagerTest, SlabGrowthBeyondInitialBatch)
{
	std::set<Segment *> seen;
	for (int i = 0; i < 25; i++) { // 25 > (first alocation = 8) + (second = 16)
		Segment *seg = bm.getBuffer();
		ASSERT_TRUE(seg != NULL) << "Failed on allocation " << i;
		EXPECT_TRUE(seen.find(seg) == seen.end())
			<< "Duplicate on allocation " << i;
		seen.insert(seg);
	}
}

TEST_F(BufferManagerTest, ReturnBuffersReturnsChain)
{
	Segment *a = bm.getBuffer();
	Segment *b = bm.getBuffer();
	Segment *c = bm.getBuffer();

	a->_next = b;
	b->_next = c;
	c->_next = NULL;

	bm.returnBuffers(a);

	std::set<Segment *> original;
	original.insert(a);
	original.insert(b);
	original.insert(c);

	for (int i = 0; i < 3; i++) {
		Segment *seg = bm.getBuffer();
		EXPECT_TRUE(original.find(seg) != original.end())
			<< "Unexpected pointer on retrieval " << i;
		original.erase(seg);
	}
	EXPECT_TRUE(original.empty());
}
