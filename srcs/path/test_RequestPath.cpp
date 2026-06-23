#include "RequestPath.hpp"
#include "RequestPathConsolidator.hpp"
#include "test_RequestPathBase.hpp"
#include <string>

class RequestPathTest : public RequestPathTestBase {
protected:
	RequestPath _path;
	std::string _buff;

	void consolidate(const char *input) {
		_buff = input;
		_path = RequestPathConsolidator::consolidate(_buff);
	}
};

TEST_F(RequestPathTest, DefaultConstruction) {
	EXPECT_FALSE(_path.getType() == RequestPath::DIR);
	EXPECT_FALSE(_path.getType() == RequestPath::CGI);
	EXPECT_EQ(_path.getCgiExtension().size(), 0u);
}
TEST_F(RequestPathTest, InheritsPathGetters) {
	consolidate("/var/www/html/index.html");
	EXPECT_EQ(_path.getPath().getStr(), "/var/www/html/index.html");
	EXPECT_EQ(_path.getQuery().size(), 0u);
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
