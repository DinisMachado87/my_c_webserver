SOCKETS_SRCS := ASocket.cpp Listening.cpp Connection.cpp CGISocketPair.cpp

SOCKETS_TEST_SRCS := $(notdir $(wildcard srcs/sockets/test_*.cpp))

SOCKETS_OBJS      := $(SOCKETS_SRCS:%.cpp=$(OBJ_DIR)/sockets/%.o)
SOCKETS_TEST_OBJS := $(SOCKETS_TEST_SRCS:%.cpp=$(OBJ_DIR)/sockets/%.o)

SRCS_ALL      += $(addprefix srcs/sockets/,$(SOCKETS_SRCS))
TEST_SRCS_ALL += $(addprefix srcs/sockets/,$(SOCKETS_TEST_SRCS))

MODULES      += sockets
INCLUDE_DIRS += srcs/sockets
