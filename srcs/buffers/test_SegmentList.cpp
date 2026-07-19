#include "BufferManager.hpp"
#include "Segment.hpp"
#include "SegmentList.hpp"
#include <cstddef>
#include <gtest/gtest.h>

class SegmentListTest : public ::testing::Test
{
public:
	typedef enum { HEAD, TAIL } t_listEnd;

protected:
	BufferManager _SegmentPool;
	SegmentList _segList;
	SegmentListTest() :
		_SegmentPool(),
		_segList(_SegmentPool)
	{
	}

	/* Helpers */
	static void tag(Segment *s, char c) { s->copyIn(&c, 1); }

	// controlSegs optional — captures push order for pointer-identity checks.
	void pushTaggedSegments(SegmentListTest::t_listEnd end,
							const char *tagChars, Segment **controlSegs = NULL)
	{
		for (char tagChar = *tagChars; tagChar; tagChar = *(++tagChars)) {
			Segment *seg = _SegmentPool.getSegment();
			tag(seg, tagChar);

			if (end == SegmentListTest::TAIL)
				_segList.pushTail(seg);
			else
				_segList.pushHead(seg);

			if (controlSegs)
				*controlSegs++ = seg;
		}
	}

	void assertOrder(SegmentListTest::t_listEnd end, const char *tags,
					 size_t size)
	{
		for (size_t i = 0; i < size; ++i) {
			Segment *seg = NULL;

			if (end == SegmentListTest::HEAD)
				seg = _segList.popHead();
			else
				seg = _segList.popTail();

			ASSERT_TRUE(seg) << "underflow at " << i;
			StrView view = seg->writtenView();
			ASSERT_GE(view.size(), 1u);
			EXPECT_EQ(view.data()[0], tags[i]) << "pos " << i;
		}
		EXPECT_TRUE(_segList.popHead() == NULL);
	}
};

/* Order/content tests */

TEST_F(SegmentListTest, PushTailForwardOrder)
{
	pushTaggedSegments(TAIL, "abc");
	assertOrder(HEAD, "abc", 3);
}

TEST_F(SegmentListTest, PushTailEmptySingleNode)
{
	pushTaggedSegments(TAIL, "a");
	EXPECT_TRUE(_segList.popHead());
	EXPECT_FALSE(_segList.popHead());
}

TEST_F(SegmentListTest, PushHeadReversedOrder)
{
	pushTaggedSegments(HEAD, "abc");
	assertOrder(HEAD, "cba", 3);
}

TEST_F(SegmentListTest, PushTailChangesTailNotInterior)
{
	pushTaggedSegments(TAIL, "ab");
	assertOrder(HEAD, "ab", 2);
}

/* Identity / link-primitive tests */

TEST_F(SegmentListTest, PopTailReverseOrder)
{
	Segment *controlSegs[3];
	pushTaggedSegments(TAIL, "abc", controlSegs);
	EXPECT_EQ(_segList.popTail(), controlSegs[2]);
	EXPECT_EQ(_segList.popTail(), controlSegs[1]);
	EXPECT_EQ(_segList.popTail(), controlSegs[0]);
	EXPECT_FALSE(_segList.popHead());
}

TEST_F(SegmentListTest, PopHeadLeavesChainValid)
{
	Segment *controlSegs[3];
	pushTaggedSegments(TAIL, "abc", controlSegs);
	EXPECT_EQ(_segList.popHead(), controlSegs[0]);
	assertOrder(HEAD, "bc", 2);
}

TEST_F(SegmentListTest, PoppedSegRelinksClean)
{
	Segment *controlSegs[2];
	pushTaggedSegments(TAIL, "ab", controlSegs);
	Segment *popped = _segList.popHead();

	SegmentList destList(_SegmentPool);
	destList.pushTail(popped); // dirty _next would trail
	EXPECT_EQ(destList.popHead(), popped);
	EXPECT_FALSE(destList.popHead());
}

TEST_F(SegmentListTest, PopToEmptyResetsEndpoints)
{
	pushTaggedSegments(TAIL, "a");
	_segList.popHead();

	Segment *controlSegs[1];
	pushTaggedSegments(TAIL, "b", controlSegs); // empty-push path
	EXPECT_EQ(_segList.popHead(), *controlSegs);
	EXPECT_FALSE(_segList.popHead());
}

TEST_F(SegmentListTest, PopHeadPatchesPrevPointer)
{
	Segment *controlSegs[2];
	pushTaggedSegments(TAIL, "ab", controlSegs);
	_segList.popHead();
	EXPECT_EQ(_segList.popTail(), controlSegs[1]); // survivor b, _prev nulled
	EXPECT_FALSE(_segList.popTail());
}

/* reset tests */

TEST_F(SegmentListTest, ResetZerosCursors)
{
	Segment *a = _SegmentPool.getSegment();
	a->copyIn("Hello", 5);
	_segList.pushTail(a);
	_segList.reset();
	EXPECT_EQ(a->used(), 0u);
}

TEST_F(SegmentListTest, ResetNullsEndpoints)
{
	pushTaggedSegments(TAIL, "ab");
	_segList.reset(); // segs to recycle stack
	Segment *b = _SegmentPool.getSegment();
	_segList.pushTail(b);
	EXPECT_EQ(_segList.popHead(), b);
	EXPECT_FALSE(_segList.popHead());
}
