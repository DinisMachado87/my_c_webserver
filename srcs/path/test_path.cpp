#include "PathConsolidator.hpp"
#include "StrView.hpp"
#include "test_PathBase.hpp"
#include <sstream>
#include <string>

class PathTest : public PathTestBase {
protected:
	Path _path;
	std::string _input;

	void setPath(const char *input) {
		_input = input;
		_path._path = _input;
	}
};

TEST_F(PathTest, DefaultConstruction) {
	EXPECT_EQ(_path.getPath().size(), 0u);
	EXPECT_EQ(_path.getQuery().size(), 0u);
	EXPECT_EQ(_path.getFragment().size(), 0u);
}
TEST_F(PathTest, GetPath) {
	setPath("/var/www/html");
	EXPECT_EQ(_path.getPath().getStr(), "/var/www/html");
}
TEST_F(PathTest, GetQuery) {
	std::string str = "/search?q=hello";
	_path = PathConsolidator::consolidate(str);
	EXPECT_EQ(_path.getQuery().getStr(), "q=hello");
}
TEST_F(PathTest, GetFragment) {
	std::string str = "/page#section";
	_path = PathConsolidator::consolidate(str);
	EXPECT_EQ(_path.getFragment().getStr(), "section");
}
TEST_F(PathTest, CopyConstruction) {
	setPath("/var/www/html");
	Path copy(_path);
	expectEqualPath(copy, _path);
}
TEST_F(PathTest, AssignmentOperator) {
	setPath("/var/www/html");
	Path other = _path;
	expectEqualPath(other, _path);
}
TEST_F(PathTest, SelfAssignment) {
	setPath("/var/www/html");
	_path = _path;
	EXPECT_EQ(_path.getPath().getStr(), "/var/www/html");
}
