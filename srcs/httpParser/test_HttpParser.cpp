#include "ConfParser.hpp"
#include "HttpParser.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>

using std::endl;
using std::runtime_error;
using std::string;

// CONSTANTS

struct s_requestLine {
	const char *_requestLine;
	uint _method;
	const char *_URI;
};

static const s_requestLine GET_REQUEST_LINE
	= {"GET /folder HTTP/1.1\r\n", Location::GET, "/folder"};

static const s_requestLine POST_REQUEST_LINE
	= {"POST /folder HTTP/1.1\r\n", Location::POST, "/folder"};

static const s_requestLine DELETE_REQUEST_LINE
	= {"DELETE /folder HTTP/1.1\r\n", Location::DELETE, "/folder"};

// Header struct
struct Header {
	const char *key;
	const char *value;
};

// Base headers (without Connection or body-related headers)
static const Header baseHeaders[]
	= {{"Host", "localhost:8080"},
	   {"User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:148.0) "
					  "Gecko/20100101 Firefox/148.0"},
	   {"Accept",
		"text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"},
	   {"Accept-Language", "en-US,en;q=0.9"},
	   {"Accept-Encoding", "gzip, deflate, br, zstd"},
	   {NULL, NULL}};

// Individual header options
static const Header HOST_DUPLICATE = {"Host", "localhost:8081"};
static const Header CONTENT_TYPE_JSON = {"Content-Type", "application/json"};
static const Header CONNECTION_KEEP_ALIVE = {"Connection", "keep-alive"};
static const Header CONNECTION_CLOSE = {"Connection", "close"};
static const Header CONTENT_LENGTH_42 = {"Content-Length", "42"};
static const Header CONTENT_LENGTH_100 = {"Content-Length", "100"};
static const Header CHUNKED_ENCODING = {"Transfer-Encoding", "chunked"};

// Chunked body sample
static const char *CHUNKED_BODY_SAMPLE = "5\r\n"
										 "test_\r\n"
										 "5\r\n"
										 "test_\r\n"
										 "0\r\n"
										 "\r\n";

string miniconfig = "server {\n"
					"listen 127.0.0.1:8080;\n"
					"location / {\n"
					"allowed_methods GET POST DELETE;\n"
					"root /tmp/test;\n"
					"}\n"
					"}\n";

// TEST CLASS

class HttpParserTest : public ::testing::Test {
private:
	string _requestStr;
	string _body;

	ConfParser _confParser;
	std::vector<Server *> _servers;
	Request *_request;
	HttpParser *_parser;

protected:
	void SetUp() {
		try {
			_confParser.createServers();
			if (_servers.empty() || !_servers[0])
				throw runtime_error(TRACED("Failed to create Servers"));
			std::cout << *_servers[0] << '\n' << endl;
			_parser = new HttpParser(*_servers[0]);
		} catch (runtime_error err) {
			LOG_ERROR(err);
			throw;
		}
	}

	void TearDown() {
		if (_request)
			delete _request;
		_request = NULL;

		if (_parser)
			delete _parser;
		_parser = NULL;

		if (!_servers.empty())
			_servers.clear();
	}

public:
	HttpParserTest() :
		_confParser(miniconfig, _servers),
		_request(NULL),
		_parser(NULL) {}

	// Test helper components
	void testRequestLine(const s_requestLine &requestLine) {
		_requestStr = requestLine._requestLine;
		_requestStr.append("Host: localhost\r\n");
		_requestStr.append("\r\n");
		std::cout << "REQUEST LINE // " << _requestStr << endl;
		_request = _parser->parse(_requestStr.c_str(), _requestStr.length());
		EXPECT_TRUE(_request);
		EXPECT_EQ(_request->getMethod(), requestLine._method);
		EXPECT_EQ(_request->getPath(), requestLine._URI);
	}

	void testHeaders(const Header headers[]) {
		ASSERT_TRUE(_request);

		for (size_t i = 0; headers[i].key != NULL; ++i) {
			const char *expKey = headers[i].key;
			const char *expValue = headers[i].value;
			const StrView *value = _request->getHeaderValue(expKey);
			EXPECT_TRUE(value)
				<< "Key " << expKey << " does not exist in headers map";
			EXPECT_STREQ(value->getStr().c_str(), expValue);
			if (HasFailure())
				LOG_LABELED(Logger::LOG, expValue, value->getStr().c_str());
		}
	}

	void testBody(size_t expBodySize) {
		EXPECT_TRUE(_parser->_nextBodySection == expBodySize);
		const StrView &body = _request->getBody();
		EXPECT_TRUE(body.ncompare(_body.c_str(), _body.length()))
			<< body.getStr() << "\r\n"
			<< _body << endl;
	}

	// BUILD TEST STRING
	void appendBody(size_t size) {
		const string strTest = "test_";
		const size_t strTestLen = 5;
		size_t rounds = size / strTestLen;
		size_t leftover = size % strTestLen;
		while (rounds--)
			_body.append(strTest);
		_body.append(strTest, leftover);
		_requestStr.append(_body);
	}

	void addHeader(const Header &header) {
		_requestStr.append(header.key);
		_requestStr.append(": ");
		_requestStr.append(header.value);
		_requestStr.append("\r\n");
	}

	void addHeaders(const Header headers[]) {
		for (size_t i = 0; headers[i].key != NULL; ++i)
			addHeader(headers[i]);
	}

	void finalizeHeaders() { _requestStr.append("\r\n"); }

	// Partial Parse tests
	void ParseHeaders() {
		_parser->_state = HttpParser::HEADERS;
		_request = _parser->parse(_requestStr.c_str(), _requestStr.length());
	}

	void parseRequest() {
		_request = _parser->parse(_requestStr.c_str(), _requestStr.length());
	}

	void buildRequest(const s_requestLine &requestLine,
					  const Header baseHeadersArray[],
					  const Header additionalHeaders[], size_t bodySize,
					  bool isChunked = false) {
		_requestStr = requestLine._requestLine;
		addHeaders(baseHeadersArray);
		addHeaders(additionalHeaders);
		finalizeHeaders();

		if (isChunked)
			_requestStr.append(CHUNKED_BODY_SAMPLE);
		else if (bodySize > 0)
			appendBody(bodySize);
	}

	bool printDebugOnFailure() {
		if (!HasFailure())
			return false;

		LOG_LABELED(Logger::CONTENT, "Test str", _requestStr.c_str());
		LOG_OBJ("Server Parsed:", _request);
		return true;
	}

	void print() { LOG_OBJ("Server Parsed", _request); }

	void testRequestWithHeaders(const s_requestLine &requestLine,
								const Header additionalHeaders[],
								size_t bodySize, bool shouldSucceed,
								bool isChunked = false) {

		buildRequest(requestLine, baseHeaders, additionalHeaders, bodySize,
					 isChunked);
		parseRequest();
		if (shouldSucceed) {
			ASSERT_TRUE(_request);
			if (printDebugOnFailure())
				return;

			EXPECT_EQ(_request->getMethod(), requestLine._method);
			if (HasFailure())
				LOG_LABELED(Logger::LOG, _request->getMethodStr(),
							g_methods[requestLine._method]);
			EXPECT_TRUE(_request->getPath().compare(requestLine._URI));
			if (HasFailure())
				LOG_LABELED(Logger::LOG, _request->getPath().getStr().c_str(),
							requestLine._URI);
			ASSERT_FALSE(printDebugOnFailure());

			testHeaders(baseHeaders);
			ASSERT_FALSE(printDebugOnFailure());

			testHeaders(additionalHeaders);
			ASSERT_FALSE(printDebugOnFailure());

			if (bodySize > 0) {
				testBody(bodySize);
				ASSERT_FALSE(printDebugOnFailure());
			}
		} else {
			EXPECT_FALSE(_request) << "Parser should reject this combination";
			ASSERT_FALSE(printDebugOnFailure());
		}
	}
};

// TEST CASES

// Request Line
TEST_F(HttpParserTest, BasicTestGet) { testRequestLine(GET_REQUEST_LINE); }

TEST_F(HttpParserTest, BasicTestPost) { testRequestLine(POST_REQUEST_LINE); }

TEST_F(HttpParserTest, BasicTestDelete) {
	testRequestLine(DELETE_REQUEST_LINE);
}

// Simple Complete Requests
TEST_F(HttpParserTest, GetRequestWithHeaders) {
	const Header headers[] = {CONTENT_TYPE_JSON, {NULL, NULL}};
	testRequestWithHeaders(GET_REQUEST_LINE, headers, 0, true);
}

TEST_F(HttpParserTest, PostRequestWithHeaders) {
	const Header headers[] = {CONTENT_TYPE_JSON, {NULL, NULL}};
	testRequestWithHeaders(POST_REQUEST_LINE, headers, 0, true);
}

TEST_F(HttpParserTest, DeleteRequestWithHeaders) {
	const Header headers[] = {CONTENT_TYPE_JSON, {NULL, NULL}};
	testRequestWithHeaders(DELETE_REQUEST_LINE, headers, 0, true);
}

// VALID Combinations - GET without body
TEST_F(HttpParserTest, GetNoBodyKeepAlive) {
	const Header headers[] = {CONNECTION_KEEP_ALIVE, {NULL, NULL}};
	testRequestWithHeaders(GET_REQUEST_LINE, headers, 0, true);
}

TEST_F(HttpParserTest, GetNoBodyConnectionClose) {
	const Header headers[] = {CONNECTION_CLOSE, {NULL, NULL}};
	testRequestWithHeaders(GET_REQUEST_LINE, headers, 0, true);
}

// VALID Combinations - POST with Content-Length
TEST_F(HttpParserTest, PostContentLengthKeepAlive) {
	const Header headers[]
		= {CONTENT_LENGTH_42, CONNECTION_KEEP_ALIVE, {NULL, NULL}};
	testRequestWithHeaders(POST_REQUEST_LINE, headers, 42, true);
}

TEST_F(HttpParserTest, PostContentLengthConnectionClose) {
	const Header headers[]
		= {CONTENT_LENGTH_100, CONNECTION_CLOSE, {NULL, NULL}};
	testRequestWithHeaders(POST_REQUEST_LINE, headers, 100, true);
}

// VALID Combinations - POST with Chunked
TEST_F(HttpParserTest, PostChunkedKeepAlive) {
	const Header headers[]
		= {CHUNKED_ENCODING, CONNECTION_KEEP_ALIVE, {NULL, NULL}};
	testRequestWithHeaders(POST_REQUEST_LINE, headers, 0, true, true);
}

TEST_F(HttpParserTest, PostChunkedConnectionClose) {
	const Header headers[] = {CHUNKED_ENCODING, CONNECTION_CLOSE, {NULL, NULL}};
	testRequestWithHeaders(POST_REQUEST_LINE, headers, 0, true, true);
}

// VALID Combinations - DELETE with Content-Length
TEST_F(HttpParserTest, DeleteContentLengthKeepAlive) {
	const Header headers[]
		= {CONTENT_LENGTH_42, CONNECTION_KEEP_ALIVE, {NULL, NULL}};
	testRequestWithHeaders(DELETE_REQUEST_LINE, headers, 42, true);
}

TEST_F(HttpParserTest, DeleteContentLengthConnectionClose) {
	const Header headers[]
		= {CONTENT_LENGTH_42, CONNECTION_CLOSE, {NULL, NULL}};
	testRequestWithHeaders(DELETE_REQUEST_LINE, headers, 42, true);
}

// INVALID Combinations
TEST_F(HttpParserTest, InvalidBothContentLengthAndChunked) {
	const Header headers[] = {CONTENT_LENGTH_42,
							  CHUNKED_ENCODING,
							  CONNECTION_KEEP_ALIVE,
							  {NULL, NULL}};
	testRequestWithHeaders(POST_REQUEST_LINE, headers, 0, false);
}

TEST_F(HttpParserTest, DuplicateHostHeadersShouldFail) {
	const Header headers[] = {HOST_DUPLICATE, CONTENT_TYPE_JSON, {NULL, NULL}};
	testRequestWithHeaders(GET_REQUEST_LINE, headers, 0, false);
}

// Headers
TEST_F(HttpParserTest, HeadersTest) {
	addHeaders(baseHeaders);
	finalizeHeaders();
	ParseHeaders();
	testHeaders(baseHeaders);
	print();
}

TEST_F(HttpParserTest, HeadersTestSubset) {
	const Header headers[]
		= {CONTENT_TYPE_JSON, CONTENT_LENGTH_42, {NULL, NULL}};

	addHeaders(headers);
	finalizeHeaders();
	appendBody(42);
	ParseHeaders();

	testHeaders(headers);
	testBody(42);
}
