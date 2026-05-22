#ifndef PATHTESTBASE_HPP
#define PATHTESTBASE_HPP

#include "RequestPathConsolidator.hpp"
#include "StrView.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

class PathTestBase : public ::testing::Test {
protected:
	std::string _buffer;

	StrView makeStrView(const std::string &input) {
		_buffer = input;
		return StrView(_buffer, 0, static_cast<uint>(_buffer.size()));
	}

	RequestPath makeRequestPath(const std::string &input) {
		RequestPath rp;
		rp._path = makeStrView(input);
		RequestPathConsolidator::consolidate(rp._path, rp);
		return rp;
	}

	void expectConsolidation(const std::string &input,
							 const std::string &expected) {
		StrView path = makeStrView(input);
		PathConsolidator::consolidate(path);
		EXPECT_EQ(path.getStr(), expected) << "Input: \"" << input << "\"";
	}

	void expectConsolidationThrows(const std::string &input) {
		StrView path = makeStrView(input);
		EXPECT_THROW(PathConsolidator::consolidate(path), std::runtime_error)
			<< "Input: \"" << input << "\" should throw";
	}

	void expectRequestPathThrows(const std::string &input) {
		RequestPath rp;
		rp._path = makeStrView(input);
		EXPECT_THROW(RequestPathConsolidator::consolidate(rp._path, rp),
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
		EXPECT_EQ(a.isCgi(), b.isCgi());
		EXPECT_EQ(a.isDir(), b.isDir());
		EXPECT_EQ(a.getCgiExtension().getStr(), b.getCgiExtension().getStr());
	}
};

#endif
