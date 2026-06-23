#ifndef PATHTESTBASE_HPP
#define PATHTESTBASE_HPP

#include "RequestPathConsolidator.hpp"
#include "StrView.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

class PathTestBase : public ::testing::Test {
protected:
	void expectConsolidation(const std::string &input,
							 const std::string &expected) {
		StrView pathStr = input;
		Path path = PathConsolidator::consolidate(pathStr);
		EXPECT_EQ(path._path.getStr(), expected)
			<< "Input: \"" << input << "\"";
	}

	void expectConsolidationThrows(const std::string &input) {
		StrView path = input;
		EXPECT_THROW(PathConsolidator::consolidate(path), std::runtime_error)
			<< "Input: \"" << input << "\" should throw";
	}

	void expectRequestPathThrows(const std::string &input) {
		StrView path = input;
		EXPECT_THROW(RequestPathConsolidator::consolidate(path),
					 std::runtime_error)
			<< "Input: \"" << input << "\" should throw";
	}

	void expectEqualPath(const Path &a, const Path &b) {
		EXPECT_EQ(a.getPath().getStr(), b.getPath().getStr());
		EXPECT_EQ(a.getQuery().getStr(), b.getQuery().getStr());
		EXPECT_EQ(a.getFragment().getStr(), b.getFragment().getStr());
	}

	void expectEqualRequestPath(const RequestPath &a, const RequestPath &b) {
		expectEqualPath(a, b);
		EXPECT_EQ(a.getType(), b.getType());
		EXPECT_EQ(a.getCgiExtension().getStr(), b.getCgiExtension().getStr());
	}
};

#endif
