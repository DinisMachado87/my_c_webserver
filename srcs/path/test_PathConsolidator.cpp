#include "test_PathBase.hpp"

class PathConsolidatorTest : public PathTestBase {};

// Simple paths
TEST_F(PathConsolidatorTest, SimplePath) {
	expectConsolidation("/a/b/c", "/a/b/c");
}
TEST_F(PathConsolidatorTest, RootPath) { expectConsolidation("/", "/"); }

// Current directory
TEST_F(PathConsolidatorTest, CurrentDirectory) {
	expectConsolidation("/a/./b", "/a/b");
}
TEST_F(PathConsolidatorTest, MultipleDots) {
	expectConsolidation("/a/././b/./c", "/a/b/c");
}
TEST_F(PathConsolidatorTest, OnlyDot) { expectConsolidation("/.", "/"); }

// Parent directory
TEST_F(PathConsolidatorTest, ParentDirectory) {
	expectConsolidation("/a/b/../c", "/a/c");
}
TEST_F(PathConsolidatorTest, MultipleParents) {
	expectConsolidation("/a/b/c/../../d", "/a/d");
}
TEST_F(PathConsolidatorTest, ParentAtEnd) {
	expectConsolidation("/a/b/..", "/a");
}
TEST_F(PathConsolidatorTest, AllTheWayBack) {
	expectConsolidation("/a/b/../../", "/");
}
TEST_F(PathConsolidatorTest, ComplexPath) {
	expectConsolidation("/a/b/../c/./d/../e", "/a/c/e");
}

// Consecutive slashes
TEST_F(PathConsolidatorTest, DoubleSlash) {
	expectConsolidation("/a//b", "/a/b");
}
TEST_F(PathConsolidatorTest, TripleSlash) {
	expectConsolidation("/a///b///c", "/a/b/c");
}
TEST_F(PathConsolidatorTest, LeadingDoubleSlash) {
	expectConsolidation("//a/b", "/a/b");
}

// Mixed
TEST_F(PathConsolidatorTest, MixedDotsDotDots) {
	expectConsolidation("/a/./b/../c/./d", "/a/c/d");
}
TEST_F(PathConsolidatorTest, DotBeforeDotDot) {
	expectConsolidation("/a/./b/./../c", "/a/c");
}

// Trailing slashes
TEST_F(PathConsolidatorTest, TrailingSlash) {
	expectConsolidation("/a/b/c/", "/a/b/c/");
}
TEST_F(PathConsolidatorTest, TrailingSlashWithDotDot) {
	expectConsolidation("/a/b/../", "/a/");
}

// Lookalike segments
TEST_F(PathConsolidatorTest, DotDotInFilename) {
	expectConsolidation("/a/..b/c", "/a/..b/c");
}
TEST_F(PathConsolidatorTest, DotDotDot) {
	expectConsolidation("/a/.../b", "/a/.../b");
}

// Long paths
TEST_F(PathConsolidatorTest, DeepPath) {
	expectConsolidation("/a/b/c/d/e/f/g/h/i/j", "/a/b/c/d/e/f/g/h/i/j");
}
TEST_F(PathConsolidatorTest, DeepWithDotDots) {
	expectConsolidation("/a/b/c/d/../e/../f/g", "/a/b/c/f/g");
}

// Error cases
TEST_F(PathConsolidatorTest, NegativeLevel_Single) {
	expectConsolidationThrows("/..");
}
TEST_F(PathConsolidatorTest, NegativeLevel_Multiple) {
	expectConsolidationThrows("/../..");
}
TEST_F(PathConsolidatorTest, NegativeLevel_AfterPath) {
	expectConsolidationThrows("/a/../..");
}
TEST_F(PathConsolidatorTest, NegativeLevel_Complex) {
	expectConsolidationThrows("/a/b/../../..");
}
TEST_F(PathConsolidatorTest, PathTraversalAttempt) {
	expectConsolidationThrows("/public/../../etc/passwd");
}
TEST_F(PathConsolidatorTest, MultipleTraversalAttempt) {
	expectConsolidationThrows("/public/../../../root");
}

// Real world
TEST_F(PathConsolidatorTest, TypicalWebPath) {
	expectConsolidation("/var/www/html/index.html", "/var/www/html/index.html");
}
TEST_F(PathConsolidatorTest, RelativeWebPath) {
	expectConsolidation("/images/../css/style.css", "/css/style.css");
}
TEST_F(PathConsolidatorTest, NestedResources) {
	expectConsolidation("/api/v1/../../public/docs", "/public/docs");
}
