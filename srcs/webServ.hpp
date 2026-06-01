#pragma once

#include <csignal>
#define VERBOSE 1
// Unix return code
#define OK 0
#define ERR -1
// Multical returns
#define ONGOING false
#define DONE true
// FD
#define EMPTY -1
// http
enum methods { DEFAULT, GET, POST, DELETE };
// default server settings
#define DEFAULT_METHODS {GET, POST, DELETE}
#define DEFAULT_METHODS_LEN 3

#define DEFAULT_ROOT "./defaultLocation/"
#define DEFAULT_INDEX "index.html"
#define MB 1048576 // megabyte
#define CLIENT_MAX_BODY MB
#define UPLOAD_MAX_BODY 100 * MB
// EPOLL Macros
#define MAX_EVENTS 1024
#define RESPONSES_CUE_SIZE 10
#define TIMEOUT 1000
#define RECV_SIZE 1000
#define CHUNK_SIZE 1000
// logger
#define LOGLEVEL CONTENT
#define LOGGING true
#define LOGTOCLI true
#define LOGTOFILE true
#define NONUM -2

// HTTP Parser Limits
#define MAX_HEADER_SIZE 8192
#define MAX_CONTENT_LENGTH 1024

// #define DEBUG_PATH true

#ifdef DEBUG_PATH
#define DEBUG(x) x
#else
#define DEBUG(x) ((void)0)
#endif

// Type Macros
typedef unsigned int uint;
typedef unsigned char uchar;

extern volatile sig_atomic_t g_shutdown;

extern const char *g_methods[];
