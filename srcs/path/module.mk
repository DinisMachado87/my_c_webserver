PATH_SRCS := PathConsolidator.cpp RequestPathConsolidator.cpp \
             Path.cpp RequestPath.cpp

PATH_TEST_SRCS := $(notdir $(wildcard srcs/path/test_*.cpp))

PATH_OBJS      := $(PATH_SRCS:%.cpp=$(OBJ_DIR)/path/%.o)
PATH_TEST_OBJS := $(PATH_TEST_SRCS:%.cpp=$(OBJ_DIR)/path/%.o)

SRCS_ALL      += $(addprefix srcs/path/,$(PATH_SRCS))
TEST_SRCS_ALL += $(addprefix srcs/path/,$(PATH_TEST_SRCS))

MODULES      += path
INCLUDE_DIRS += srcs/path
