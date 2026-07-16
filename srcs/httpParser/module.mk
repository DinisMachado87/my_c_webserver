PARSER_HTTP_SRCS := RequestLineParser.cpp HttpToken.cpp HttpError.cpp \
                    HttpParser.cpp HttpHeadersParser.cpp Router.cpp

PARSER_HTTP_TEST_SRCS := $(notdir $(wildcard srcs/httpParser/test_*.cpp))

PARSER_HTTP_OBJS      := $(PARSER_HTTP_SRCS:%.cpp=$(OBJ_DIR)/httpParser/%.o)
PARSER_HTTP_TEST_OBJS := $(PARSER_HTTP_TEST_SRCS:%.cpp=$(OBJ_DIR)/httpParser/%.o)

SRCS_ALL      += $(addprefix srcs/httpParser/,$(PARSER_HTTP_SRCS))
TEST_SRCS_ALL += $(addprefix srcs/httpParser/,$(PARSER_HTTP_TEST_SRCS))

MODULES      += httpParser
INCLUDE_DIRS += srcs/httpParser
