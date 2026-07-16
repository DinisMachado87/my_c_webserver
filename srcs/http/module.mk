HTTP_SRCS := Request.cpp RequestLine.cpp HttpStatus.cpp

HTTP_TEST_SRCS := $(notdir $(wildcard srcs/http/test_*.cpp))

HTTP_OBJS      := $(HTTP_SRCS:%.cpp=$(OBJ_DIR)/http/%.o)
HTTP_TEST_OBJS := $(HTTP_TEST_SRCS:%.cpp=$(OBJ_DIR)/http/%.o)

SRCS_ALL      += $(addprefix srcs/http/,$(HTTP_SRCS))
TEST_SRCS_ALL += $(addprefix srcs/http/,$(HTTP_TEST_SRCS))

MODULES      += http
INCLUDE_DIRS += srcs/http
