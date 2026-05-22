#include "test_PathBase.hpp"
#include <sstream>

class PathTest : public PathTestBase {
protected:
	Path _path;

	void setPath(const std::string &input) { _path._path = makeStrView(input); }
};

TEST_F(PathTest, DefaultConstruction) {
	EXPECT_EQ(_path.getPath().getLen(), 0u);
	EXPECT_EQ(_path.getQuery().getLen(), 0u);
	EXPECT_EQ(_path.getFragment().getLen(), 0u);
}
TEST_F(PathTest, GetPath) {
	setPath("/var/www/html");
	EXPECT_EQ(_path.getPath().getStr(), "/var/www/html");
}
TEST_F(PathTest, GetQuery) {
	_buffer = "/search?q=hello";
	_path._query = StrView(_buffer, 8, 7);
	EXPECT_EQ(_path.getQuery().getStr(), "q=hello");
}
TEST_F(PathTest, GetFragment) {
	_buffer = "/page#section";
	_path._fragment = StrView(_buffer, 6, 7);
	EXPECT_EQ(_path.getFragment().getStr(), "section");
}
TEST_F(PathTest, CopyConstruction) {
	setPath("/var/www/html");
	Path copy(_path);
	expectEqualPath(copy, _path);
}
TEST_F(PathTest, AssignmentOperator) {
	setPath("/var/www/html");
	Path other;
	other = _path;
	expectEqualPath(other, _path);
}
TEST_F(PathTest, SelfAssignment) {
	setPath("/var/www/html");
	_path = _path;
	EXPECT_EQ(_path.getPath().getStr(), "/var/www/html");
}
TEST_F(PathTest, PrintOutput) {
	setPath("/var/www");
	std::ostringstream oss;
	_path.print(oss);
	EXPECT_NE(oss.str().find("/var/www"), std::string::npos);
}
TEST_F(PathTest, StreamOperator) {
	setPath("/var/www");
	std::ostringstream oss;
	oss << _path;
	EXPECT_NE(oss.str().find("/var/www"), std::string::npos);
}
