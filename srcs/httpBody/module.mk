BODY_SRCS := ChunkEncoder.cpp

BODY_TEST_SRCS := $(notdir $(wildcard srcs/httpBody/test_*.cpp))

BODY_OBJS      := $(BODY_SRCS:%.cpp=$(OBJ_DIR)/httpBody/%.o)
BODY_TEST_OBJS := $(BODY_TEST_SRCS:%.cpp=$(OBJ_DIR)/httpBody/%.o)

SRCS_ALL      += $(addprefix srcs/httpBody/,$(BODY_SRCS))
TEST_SRCS_ALL += $(addprefix srcs/httpBody/,$(BODY_TEST_SRCS))

MODULES      += httpBody
INCLUDE_DIRS += srcs/httpBody

