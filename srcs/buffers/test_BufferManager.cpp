#include "BufferManager.hpp"
#include "Segment.hpp"
#include "webServ.hpp"
#include <cstring>
#include <gtest/gtest.h>
#include <vector>

class BufferManagerTest : public ::testing::Test
{
protected:
	BufferManager pool;
};

TEST_F(BufferManagerTest, GetSegmentUsable)
{
	Segment *seg = pool.getSegment();
	ASSERT_TRUE(seg != NULL);
	seg->copyIn("ok", 2);
	StrView v = seg->writtenView();
	EXPECT_EQ(std::memcmp(v.data(), "ok", 2), 0);
}

TEST_F(BufferManagerTest, PoolReuseSamePointer)
{
	Segment *first = pool.getSegment();
	pool.returnSegment(first);
	Segment *second = pool.getSegment();
	EXPECT_EQ(first, second); // exact seg recycled
}

TEST_F(BufferManagerTest, ReturnedSegCursorRewound)
{
	Segment *seg = pool.getSegment();
	seg->copyIn("data", 4);
	pool.returnSegment(seg);
	Segment *back = pool.getSegment();
	EXPECT_EQ(back->used(), 0u); // reset ran on return
}

TEST_F(BufferManagerTest, ReturnSegmentReturnsNull)
{
	Segment *seg = pool.getSegment();
	EXPECT_TRUE(pool.returnSegment(seg) == NULL);
}

TEST_F(BufferManagerTest, AllocateOnEmptyPool)
{
	std::vector<Segment *> held;
	for (size_t i = 0; i < SLAB_START_SIZE; ++i)
		held.push_back(pool.getSegment());
	Segment *extra = pool.getSegment(); // pool empty -> allocate
	ASSERT_TRUE(extra != NULL);
	for (size_t i = 0; i < held.size(); ++i)
		ASSERT_NE(extra, held[i]); // distinct: fresh slab
	extra->copyIn("z", 1);
	EXPECT_EQ(extra->writtenView().data()[0], 'z');
}

TEST_F(BufferManagerTest, SlabDoublingAllDistinctUsable)
{
	std::vector<Segment *> held;
	for (size_t i = 0; i < SLAB_START_SIZE * 3; ++i) {
		Segment *s = pool.getSegment();
		ASSERT_TRUE(s != NULL);
		for (size_t j = 0; j < held.size(); ++j)
			ASSERT_NE(s, held[j]); // no aliasing across slabs
		s->copyIn("v", 1);
		held.push_back(s);
	}
	for (size_t i = 0; i < held.size(); ++i)
		EXPECT_EQ(held[i]->writtenView().data()[0], 'v');
}
