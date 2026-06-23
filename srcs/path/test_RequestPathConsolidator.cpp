#include "RequestPathConsolidator.hpp"
#include "test_RequestPathBase.hpp"

class RequestPathConsolidatorTest : public RequestPathTestBase {};

// CGI detection
TEST_F(RequestPathConsolidatorTest, DetectsCgiPhp) {
	expectIsCgi("/scripts/upload.php", ".php");
}
TEST_F(RequestPathConsolidatorTest, DetectsCgiPy) {
	expectIsCgi("/scripts/run.py", ".py");
}
TEST_F(RequestPathConsolidatorTest, DetectsCgiAfterConsolidation) {
	expectIsCgi("/scripts/../cgi/run.php", ".php");
}
TEST_F(RequestPathConsolidatorTest, NoCgiForPlainFile) {
	expectIsFile("/var/www/html/index.html");
}
TEST_F(RequestPathConsolidatorTest, NoCgiForNoExtension) {
	expectIsFile("/var/www/html/index");
}

// Directory detection
TEST_F(RequestPathConsolidatorTest, DetectsDirectory) {
	expectIsDir("/var/www/html/");
}
TEST_F(RequestPathConsolidatorTest, DetectsDirectoryAfterConsolidation) {
	expectIsDir("/var/www/../html/");
}
TEST_F(RequestPathConsolidatorTest, RootIsDirectory) { expectIsDir("/"); }
TEST_F(RequestPathConsolidatorTest, NoTrailingSlashNotDir) {
	expectIsFile("/var/www/html");
}

// Path consolidation correct
TEST_F(RequestPathConsolidatorTest, PathConsolidatedCorrectly) {
	RequestPath rp = RequestPathConsolidator::consolidate("/a/b/../c");
	EXPECT_EQ(rp.getPath().getStr(), "/a/c");
}
TEST_F(RequestPathConsolidatorTest, CgiPathConsolidatedCorrectly) {
	RequestPath rp
		= RequestPathConsolidator::consolidate("/a/b/../scripts/run.php");
	EXPECT_EQ(rp.getPath().getStr(), "/a/scripts/run.php");
	EXPECT_TRUE(rp.getType() == RequestPath::CGI);
}

// Errors
TEST_F(RequestPathConsolidatorTest, NegativeLevelThrows) {
	expectRequestPathThrows("/..");
}
TEST_F(RequestPathConsolidatorTest, PathTraversalThrows) {
	expectRequestPathThrows("/public/../../etc/passwd");
}
