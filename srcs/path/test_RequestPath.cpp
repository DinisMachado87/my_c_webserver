#include "test_RequestPathBase.hpp"
#include <sstream>

class RequestPathTest : public RequestPathTestBase {
protected:
	RequestPath _path;

	void consolidate(const std::string &input) {
		_path = makeRequestPath(input);
	}
};

TEST_F(RequestPathTest, DefaultConstruction) {
	EXPECT_FALSE(_path.isDir());
	EXPECT_FALSE(_path.isCgi());
	EXPECT_EQ(_path.getCgiExtension().getLen(), 0u);
}
TEST_F(RequestPathTest, InheritsPathGetters) {
	consolidate("/var/www/html/index.html");
	EXPECT_EQ(_path.getPath().getStr(), "/var/www/html/index.html");
	EXPECT_EQ(_path.getQuery().getLen(), 0u);
}

// CGI via fixture member
TEST_F(RequestPathTest, IsCgi) {
	consolidate("/scripts/run.php");
	expectIsCgi("/scripts/run.php", ".php");
}
TEST_F(RequestPathTest, IsDir) { expectIsDir("/var/www/html/"); }
TEST_F(RequestPathTest, IsFile) { expectIsFile("/var/www/html/index.html"); }

// Copy and assignment
TEST_F(RequestPathTest, CopyConstruction) {
	consolidate("/scripts/run.php");
	RequestPath copy(_path);
	expectEqualRequestPath(copy, _path);
}
TEST_F(RequestPathTest, AssignmentOperator) {
	consolidate("/scripts/run.php");
	RequestPath other;
	other = _path;
	expectEqualRequestPath(other, _path);
}
TEST_F(RequestPathTest, SelfAssignment) {
	consolidate("/scripts/run.php");
	_path = _path;
	expectEqualRequestPath(_path, _path);
}

// Print
TEST_F(RequestPathTest, PrintIncludesHttpInfo) {
	consolidate("/scripts/run.php");
	std::ostringstream oss;
	_path.print(oss);
	std::string out = oss.str();
	EXPECT_NE(out.find("isCgi"), std::string::npos);
	EXPECT_NE(out.find("isDir"), std::string::npos);
}
TEST_F(RequestPathTest, StreamOperator) {
	consolidate("/var/www/");
	std::ostringstream oss;
	oss << _path;
	EXPECT_FALSE(oss.str().empty());
}
