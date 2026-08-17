httpParser_FILES := HttpError.cpp HttpHeadersParser.cpp HttpParser.cpp \
                    HttpToken.cpp RequestLineParser.cpp Router.cpp
DEPS_httpParser  := buffers http logger parser path responses server
MODULES          += httpParser
