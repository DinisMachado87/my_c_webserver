#pragma once

#include "BufferManager.hpp"
#include "IOBuffer.hpp"
#include "Response.hpp"
#include "ResponseHeaders.hpp"

/* Serves a static file. execute() serializes status line + headers into the
 * head of _body's chain. send() then streams the file through _body — file fd
 * in, socket fd out, one segment per call to bound memory. Because readIn()
 * refills the tail segment, headers and the first file bytes leave together.
 * Owns the file fd handed over by the Router. */
class GETResponse : public Response
{
public:
	GETResponse(const Request *req, size_t fileSize, const int fileFd,
				const int sockFd, BufferManager &bm);
	~GETResponse();

	ASocket *execute();
	int send();

private:
	int _fileFd;
	size_t _fileSize;
	IOBuffer _body;
	ResponseHeaders _headers;

	/* Explicit disables */
	GETResponse();
	GETResponse(const GETResponse &other);
	GETResponse &operator=(const GETResponse &other);

	void buildHeaders();

	static const char *getMimeType(const StrView &path);
};
