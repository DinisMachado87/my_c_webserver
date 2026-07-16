CONF_PARSER_SRCS := Token.cpp Expect.cpp ConfParser.cpp

CONF_PARSER_TEST_SRCS := $(notdir $(wildcard srcs/parser/test_*.cpp))

CONF_PARSER_OBJS      := $(CONF_PARSER_SRCS:%.cpp=$(OBJ_DIR)/parser/%.o)
CONF_PARSER_TEST_OBJS := $(CONF_PARSER_TEST_SRCS:%.cpp=$(OBJ_DIR)/parser/%.o)

SRCS_ALL      += $(addprefix srcs/parser/,$(CONF_PARSER_SRCS))
TEST_SRCS_ALL += $(addprefix srcs/parser/,$(CONF_PARSER_TEST_SRCS))

MODULES      += parser
INCLUDE_DIRS += srcs/parser
