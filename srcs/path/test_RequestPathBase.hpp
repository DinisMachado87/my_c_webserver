#ifndef REQUESTPATHTESTBASE_HPP
#define REQUESTPATHTESTBASE_HPP

#include "RequestPath.hpp"
#include "RequestPathConsolidator.hpp"
#include "test_PathBase.hpp"
#include <ostream>

class RequestPathTestBase : public PathTestBase {
protected:
	void expectIsCgi(const std::string &input, const std::string &extension) {
		RequestPath rp = RequestPathConsolidator::consolidate(input);
		uchar type = rp.getType();
		EXPECT_FALSE(type == RequestPath::DIR)
			<< "Expected !isDir for: \"" << input << "\""
			<< "Got: " << RequestPath::typeLabels[type];
		EXPECT_EQ(rp.getCgiExtension().getStr(), extension)
			<< "Expected extension \"" << extension << "\" for: \"" << input
			<< "\"";
	}

	void expectIsDir(const std::string &input) {
		RequestPath rp = RequestPathConsolidator::consolidate(input);
		EXPECT_TRUE(rp.getType() == RequestPath::DIR)
			<< "Expected isDir for: \"" << input << "\"";
		EXPECT_FALSE(rp.getType() == RequestPath::CGI)
			<< "Expected !isCgi for: \"" << input << "\"";
	}

	void expectIsFile(const std::string &input) {
		RequestPath rp = RequestPathConsolidator::consolidate(input);
		EXPECT_FALSE(rp.getType() == RequestPath::CGI)
			<< "Expected !isCgi for: \"" << input << "\"";
		EXPECT_FALSE(rp.getType() == RequestPath::DIR)
			<< "Expected !isDir for: \"" << input << "\"";
	}
};

#endif
