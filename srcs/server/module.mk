SERVER_SRCS := Server.cpp Location.cpp Overrides.cpp

SERVER_TEST_SRCS := $(notdir $(wildcard srcs/server/test_*.cpp))

SERVER_OBJS      := $(SERVER_SRCS:%.cpp=$(OBJ_DIR)/server/%.o)
SERVER_TEST_OBJS := $(SERVER_TEST_SRCS:%.cpp=$(OBJ_DIR)/server/%.o)

SRCS_ALL      += $(addprefix srcs/server/,$(SERVER_SRCS))
TEST_SRCS_ALL += $(addprefix srcs/server/,$(SERVER_TEST_SRCS))

MODULES      += server
INCLUDE_DIRS += srcs/server
