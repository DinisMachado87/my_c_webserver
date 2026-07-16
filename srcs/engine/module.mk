ENGINE_SRCS := Engine.cpp Signals.cpp

ENGINE_TEST_SRCS := $(notdir $(wildcard srcs/engine/test_*.cpp))

ENGINE_OBJS      := $(ENGINE_SRCS:%.cpp=$(OBJ_DIR)/engine/%.o)
ENGINE_TEST_OBJS := $(ENGINE_TEST_SRCS:%.cpp=$(OBJ_DIR)/engine/%.o)

SRCS_ALL      += $(addprefix srcs/engine/,$(ENGINE_SRCS))
TEST_SRCS_ALL += $(addprefix srcs/engine/,$(ENGINE_TEST_SRCS))

MODULES      += engine
INCLUDE_DIRS += srcs/engine
