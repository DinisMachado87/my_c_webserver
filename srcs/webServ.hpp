#pragma once

#include <csignal>

/* Project-wide constants, limits, and typedefs. */
#define VERBOSE 1

/* Unix return conventions */
#define OK 0
#define ERR -1

/* Boolean semantics for readability */
#define TRACK true
#define UNTRACKED false
#define AFTER true
#define BEFORE false
#define ONGOING false
#define DONE true
#define EMPTY -1

/* HTTP */
enum methods { DEFAULT, GET, POST, DELETE };

/* Server defaults — used by Overrides when no config value is set */
#define DEFAULT_METHODS {GET, POST, DELETE}
#define DEFAULT_METHODS_LEN 3
#define DEFAULT_ROOT "/var/www/html"
#define DEFAULT_INDEX "index.html"
#define DEFAULT_AUTOINDEX false
#define MB 1048576
#define CLIENT_MAX_BODY MB
#define UPLOAD_MAX_BODY 100 * MB

/* Epoll and I/O sizing */
#define MAX_EVENTS 1024
#define RESPONSES_CUE_SIZE 10
#define TIMEOUT 1000
#define RECV_SIZE 1000
#define CHUNK_SIZE 1000

/* Logger compile-time config */
#define LOGLEVEL CONTENT
#define LOGGING true
#define LOGTOCLI true
#define LOGTOFILE true
#define NONUM -2

/* HTTP parser limits */
#define MAX_HEADER_SIZE 8192
#define MAX_CONTENT_LENGTH 1024

/* Debug toggle — wrap debug-only code in DEBUG() */
// #define DEBUG_PATH true

#ifdef DEBUG_PATH
#define DEBUG(x) x
#else
#define DEBUG(x) ((void)0)
#endif

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;

extern volatile sig_atomic_t g_shutdown;

extern const char *g_methods[];
