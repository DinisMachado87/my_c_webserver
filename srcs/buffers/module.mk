BUFFERS_SRCS := BufferManager.cpp SegmentList.cpp Segment.cpp \
                Reader.cpp Writer.cpp \
				IBuffer.cpp

BUFFERS_TEST_SRCS := $(notdir $(wildcard srcs/buffers/test_*.cpp))

BUFFERS_OBJS      := $(BUFFERS_SRCS:%.cpp=$(OBJ_DIR)/buffers/%.o)
BUFFERS_TEST_OBJS := $(BUFFERS_TEST_SRCS:%.cpp=$(OBJ_DIR)/buffers/%.o)

SRCS_ALL      += $(addprefix srcs/buffers/,$(BUFFERS_SRCS))
TEST_SRCS_ALL += $(addprefix srcs/buffers/,$(BUFFERS_TEST_SRCS))

MODULES      += buffers
INCLUDE_DIRS += srcs/buffers
