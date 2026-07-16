RESPONSES_SRCS := Response.cpp ERRORResponse.cpp RedirectResponse.cpp \
                  GETResponse.cpp ResponseHeaders.cpp

RESPONSES_TEST_SRCS := $(notdir $(wildcard srcs/responses/test_*.cpp))

RESPONSES_OBJS      := $(RESPONSES_SRCS:%.cpp=$(OBJ_DIR)/responses/%.o)
RESPONSES_TEST_OBJS := $(RESPONSES_TEST_SRCS:%.cpp=$(OBJ_DIR)/responses/%.o)

SRCS_ALL      += $(addprefix srcs/responses/,$(RESPONSES_SRCS))
TEST_SRCS_ALL += $(addprefix srcs/responses/,$(RESPONSES_TEST_SRCS))

MODULES      += responses
INCLUDE_DIRS += srcs/responses
