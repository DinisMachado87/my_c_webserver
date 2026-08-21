#pragma once

/* HttpParser state machine. REQUEST_LINE → HEADERS → ROUTE.
 * Each sub-parser advances _mainState when its stage completes. */
enum state { REQUEST_LINE, HEADERS, ROUTE, STATE_SIZE };
