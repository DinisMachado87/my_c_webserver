#ifndef SIGNALS_HPP
#define SIGNALS_HPP

/* Installs SIGINT/SIGTERM handlers that set g_shutdown,
 * and ignores SIGPIPE. Call once before the event loop. */
void setup_signals();

#endif
