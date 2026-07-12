#pragma once

#include <stdexcept>

class ChunkDecodeError : public std::runtime_error
{
public:
	ChunkDecodeError(const char *msg) :
		std::runtime_error(msg)
	{
	}
};
