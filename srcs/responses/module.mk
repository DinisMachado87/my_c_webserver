responses_FILES := Response.cpp ResponseHeaders.cpp GETResponse.cpp \
                   ERRORResponse.cpp RedirectResponse.cpp
DEPS_responses  := buffers http httpParser logger sockets
MODULES         += responses
