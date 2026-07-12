#include "ChunkEncoder.hpp"
#include "Hex.hpp"
#include "StrView.hpp"
#include <cstddef>
#include <cstring>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

ChunkEncoder::ChunkEncoder(int inFd, int outFd, BufferManager &bm) :
	InOutBuffer(inFd, ::read, outFd, bm),
	_state(PREPARE),
	_finishing(false),
	_headerLen(0),
	_payloadLen(0),
	_sent(0)
{
}

/* main flow */
ssize_t ChunkEncoder::writeOut()
{
	ssize_t total = 0;

	while (_state != FINISH) {
		switch (_state) {
		case PREPARE:
			if (!prepareNextChunk())
				return total;
			// fall through
		case SEND:
		case SEND_TERMINATOR: {
			struct iovec segmentsVec[3];
			int nSections = buildChunk(segmentsVec);
			ssize_t bytesSent = sendChunk(segmentsVec, nSections);
			if (bytesSent <= 0)
				return (total > 0) ? total : bytesSent;
			total += bytesSent;
			if (_sent < _headerLen + _payloadLen + 2)
				return total; // partial send, wait for EPOLLOUT
			finalizeChunk();
		} // fallthrough
		default:
			break;
		}
	}
	return total;
}

/* Private Helpers */
void ChunkEncoder::makeSizeHeader()
{
	Hex hex;
	hex.parseFromNum(_readEnd->readable());

	std::memcpy(_sizeHeader, hex.str(), hex.strLen());
	char *crlfPos = _sizeHeader + hex.strLen();
	std::memcpy(crlfPos, "\r\n", 2);
	_headerLen = hex.strLen() + 2;
	_payloadLen = _readEnd->readable();
	_sent = 0;
}

/* "0\r\n" header, no payload, "\r\n" trailer */
void ChunkEncoder::makeTerminator()
{
	std::memcpy(_sizeHeader, "0\r\n", 3);
	_headerLen = 3;
	_payloadLen = 0;
	_sent = 0;
}

bool ChunkEncoder::prepareNextChunk()
{
	if (_readEnd && _readEnd->readable() > 0) {
		makeSizeHeader();
		_state = SEND;
		return true;
	}
	if (_finishing) {
		makeTerminator();
		_state = SEND_TERMINATOR;
		return true;
	}
	return false;
}

void ChunkEncoder::finalizeChunk()
{
	if (_state == SEND_TERMINATOR) {
		_state = FINISH;
		return;
	}
	_buffManager.returnBuffers(popFront());
	_state = PREPARE;
}

static void addSection(const StrView &section, struct iovec *segmentsVec,
					   int &nSections, size_t &offset)
{
	if (offset >= section.size()) {
		offset -= section.size();
		return;
	}
	const char *cursor = section.data() + offset;
	segmentsVec[nSections].iov_base = const_cast<char *>(cursor);
	segmentsVec[nSections].iov_len = section.size() - offset;
	nSections++;
	offset = 0;
}

int ChunkEncoder::buildChunk(struct iovec *segmentsVec)
{
	int nSections = 0;
	size_t curOffset = _sent;
	const StrView header(_sizeHeader, _headerLen);
	const StrView payload(_payloadLen ? _readEnd->data() : "", _payloadLen);
	const StrView newLine("\r\n", 2);
	addSection(header, segmentsVec, nSections, curOffset);
	addSection(payload, segmentsVec, nSections, curOffset);
	addSection(newLine, segmentsVec, nSections, curOffset);
	return nSections;
}

/* sendmsg() sends all sections in a single syscall — no copy/concatenation,
 * one user-kernel context switch. MSG_NOSIGNAL avoids SIGPIPE. */
ssize_t ChunkEncoder::sendChunk(struct iovec *segmentsVec, int nSections)
{
	struct msghdr chunk = {};
	chunk.msg_iov = segmentsVec;
	chunk.msg_iovlen = nSections;
	ssize_t n = sendmsg(_outFd, &chunk, MSG_NOSIGNAL);
	if (n > 0)
		_sent += static_cast<size_t>(n);
	return n;
}

void ChunkEncoder::finalize() { _finishing = true; }
bool ChunkEncoder::done() const { return _state == FINISH; }

// Pipe/file EOF is normal — CGI finished, not a disconnect.
StrView ChunkEncoder::onEof() { return StrView(); }
