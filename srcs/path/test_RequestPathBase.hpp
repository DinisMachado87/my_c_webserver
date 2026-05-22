#ifndef REQUESTPATHTESTBASE_HPP
#define REQUESTPATHTESTBASE_HPP

#include "test_PathBase.hpp"

class RequestPathTestBase : public PathTestBase {
protected:
	void expectIsCgi(const std::string &input, const std::string &extension) {
		RequestPath rp = makeRequestPath(input);
		EXPECT_TRUE(rp.isCgi()) << "Expected isCgi for: \"" << input << "\"";
		EXPECT_FALSE(rp.isDir()) << "Expected !isDir for: \"" << input << "\"";
		EXPECT_EQ(rp.getCgiExtension().getStr(), extension)
			<< "Expected extension \"" << extension << "\" for: \"" << input
			<< "\"";
	}

	void expectIsDir(const std::string &input) {
		RequestPath rp = makeRequestPath(input);
		EXPECT_TRUE(rp.isDir()) << "Expected isDir for: \"" << input << "\"";
		EXPECT_FALSE(rp.isCgi()) << "Expected !isCgi for: \"" << input << "\"";
	}

	void expectIsFile(const std::string &input) {
		RequestPath rp = makeRequestPath(input);
		EXPECT_FALSE(rp.isCgi()) << "Expected !isCgi for: \"" << input << "\"";
		EXPECT_FALSE(rp.isDir()) << "Expected !isDir for: \"" << input << "\"";
	}
};

#endif
