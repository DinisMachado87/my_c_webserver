#include "GETResponse.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "webServ.hpp"
#include <unistd.h>

//  MIME lookup

struct MimeEntry {
	const char *ext;
	const char *type;
};

static const MimeEntry g_mimeTable[]
	= {{".html", "text/html"},		  {".htm", "text/html"},
	   {".css", "text/css"},		  {".js", "application/javascript"},
	   {".json", "application/json"}, {".xml", "application/xml"},
	   {".txt", "text/plain"},		  {".png", "image/png"},
	   {".jpg", "image/jpeg"},		  {".jpeg", "image/jpeg"},
	   {".gif", "image/gif"},		  {".svg", "image/svg+xml"},
	   {".ico", "image/x-icon"},	  {".pdf", "application/pdf"},
	   {".zip", "application/zip"},	  {NULL, NULL}};

// Compares in place — no substr, no allocation. lastSplit(BEFORE) keeps the
// dot; a '/' inside the tail means the dot was in a directory name, not an ext.
const char *GETResponse::getMimeType(const StrView &path)
{
	StrView ext = path.lastSplit('.', StrView::BEFORE);
	if (ext.empty() || *ext.data() != '.' || ext.find('/') != std::string::npos)
		return "application/octet-stream";

	for (const MimeEntry *mime = g_mimeTable; mime->ext; mime++)
		if (ext == mime->ext)
			return mime->type;

	return "application/octet-stream";
}
//  Constructor / Destructor

GETResponse::GETResponse(const Request *req, size_t fileSize, const int fileFd,
						 const int sockFd, BufferManager &bm) :
	Response(HttpStatus::_OK, req),
	_fileFd(fileFd),
	_fileSize(fileSize),
	_body(Reader(FD_FILE, fileFd), Writer(FD_SOCKET, sockFd), bm)
{
}

// IOBuffer holds the fd but does not own it — this class closes it.
GETResponse::~GETResponse()
{
	if (_fileFd >= 0)
		close(_fileFd);
}

//  execute
ASocket *GETResponse::execute()
{
	buildHeaders();

	const StrView &p = _request->getPath().path();
	LOG_INTERNAL(Logger::LOG, "GETResponse execute: ", p.data(), p.size(),
				 NONUM, 0, INT_MAX);
	return NULL;
}

// Status line + headers land at the head of the same chain the file streams
// through, so the first writeOut() carries both.
void GETResponse::buildHeaders()
{
	_body << "HTTP/1.1 " << _status.codeAndMsg() << "\r\n";

	_headers.set("Content-Type", getMimeType(_request->getPath().path()));
	_headers.set("Content-Length", _fileSize);
	_headers.setIfMissing("Connection", "keep-alive");

	_headers.serialize(_body);
}

//  send

int GETResponse::send()
{
	if (!_body.inClosed())
		_body.readIn();

	if (_body.writeOut() < 0)
		return ONGOING; // EAGAIN — retry on the next EPOLLOUT

	return _body.done() ? DONE : ONGOING;
}
