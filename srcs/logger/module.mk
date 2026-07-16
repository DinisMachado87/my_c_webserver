LOGGER_SRCS := Logger.cpp

LOGGER_TEST_SRCS := $(notdir $(wildcard srcs/logger/test_*.cpp))

LOGGER_OBJS      := $(LOGGER_SRCS:%.cpp=$(OBJ_DIR)/logger/%.o)
LOGGER_TEST_OBJS := $(LOGGER_TEST_SRCS:%.cpp=$(OBJ_DIR)/logger/%.o)

SRCS_ALL      += $(addprefix srcs/logger/,$(LOGGER_SRCS))
TEST_SRCS_ALL += $(addprefix srcs/logger/,$(LOGGER_TEST_SRCS))

MODULES      += logger
INCLUDE_DIRS += srcs/logger
