responses_FILES := Response.cpp ERRORResponse.cpp ResponseHeaders.cpp \
				   GETResponse.cpp RedirectResponse.cpp
DEPS_responses  := buffers http httpParser logger sockets
MODULES         += responses
