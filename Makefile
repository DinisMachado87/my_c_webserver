NAME			:= webserver
NAME_DEBUG		:= $(NAME)_debug
NAME_TESTS		:= $(NAME)_tests
CXX				:= c++
CXX_FLAGS		:= -Wall -Werror -Wextra -std=c++98 -MMD -MP
TEST_FLAGS		:= -lgtest -lgtest_main -lpthread
LDFLAGS			:=
SRC_DIR			:= srcs
OBJ_DIR			:= obj

# Capture filter arguments (e.g., "make debug_test Token SpecificTest")
CLASS_ARG		:= $(word 2,$(MAKECMDGOALS))
TEST_ARG		:= $(word 3,$(MAKECMDGOALS))
# Create gtest filter pattern based on arguments
ifdef CLASS_ARG
  ifdef TEST_ARG
    FILTER		:= *$(CLASS_ARG)*.$(TEST_ARG)
  else
    FILTER		:= *$(CLASS_ARG)*
  endif
else
  FILTER		:=
endif

# Source Main
SRCS_MAIN		:= main.cpp
SRCS_ENGINE		:= engine Engine.cpp Signals.cpp
SRCS_SERVER		:= server Server.cpp Location.cpp Overrides.cpp
SRCS_HTTP		:= http Request.cpp RequestLine.cpp HttpStatus.cpp
SRCS_HTTP_PARSER:= httpParser RequestLineParser.cpp HttpToken.cpp HttpError.cpp \
				   HttpParser.cpp HttpHeadersParser.cpp 
SRCS_RESPONSES	:= responses Response.cpp ERRORResponse.cpp RedirectResponse.cpp \
				   ResponseHeaders.cpp
SRCS_PATH		:= path PathConsolidator.cpp RequestPathConsolidator.cpp \
				   Path.cpp RequestPath.cpp
SRCS_SOCKET		:= sockets ASocket.cpp Listening.cpp Connection.cpp CGISocketPair.cpp 
SRCS_PARSER		:= parser Token.cpp Expect.cpp ConfParser.cpp
SRCS_BUFFERS	:= buffers BufferManager.cpp Segment.cpp SegmentList.cpp \
				   StackBuffer.cpp StreamBuffer.cpp IOBuffer.cpp 
SRCS_UTILS		:= utils StrView.cpp Clock.cpp StrViewMap.cpp 
SRCS_LOGGER		:= logger Logger.cpp 

SRC_GROUPS		:= SRCS_ENGINE SRCS_SERVER SRCS_SOCKET SRCS_PARSER SRCS_UTILS \
				   SRCS_HTTP SRCS_LOGGER SRCS_PATH SRCS_HTTP_PARSER SRCS_BUFFERS \
				   SRCS_RESPONSES 

define make_paths
$(addprefix $(word 1,$(1))/,$(wordlist 2,$(words $(1)),$(1)))
endef

# Create test file paths from a source group
define make_test_paths
$(addprefix $(word 1,$(1))/test_,$(wordlist 2,$(words $(1)),$(1)))
endef

# Build SRCS_CORE and SRCS_TEST_CORE dynamically from groups
SRCS_CORE		:= $(foreach group,$(SRC_GROUPS),$(call make_paths,$($(group))))

# If CLASS_ARG is provided, only include matching test files
ifdef CLASS_ARG
	SRCS_TEST_CORE	:= $(filter %/test_$(CLASS_ARG).cpp,$(foreach group,$(SRC_GROUPS),$(call make_test_paths,$($(group)))))
else
	SRCS_TEST_CORE	:= $(foreach group,$(SRC_GROUPS),$(call make_test_paths,$($(group))))
endif

# Extract module directories from source groups
MODULES			:= $(foreach group,$(SRC_GROUPS),$(word 1,$($(group)))) $(ENGINE_DIR) $(UTILS_DIR) $(SOCKETS_DIR) $(LOGGER_DIR) $(PATH_DIR) $(HTTPPARSER_DIR) $(RESPONSES_DIR) $(RESPONSE_DIR) $(BODY_DIR)

# Includes
INCLUDE_DIRS	:= $(SRC_DIR) $(addprefix $(SRC_DIR)/,$(MODULES))
INCLUDE_FLAGS	:= $(addprefix -I,$(INCLUDE_DIRS))

# Detect build mode from MAKECMDGOALS
IS_DEBUG		:= $(filter debug debug_test,$(MAKECMDGOALS))
IS_TEST			:= $(filter test debug_test,$(MAKECMDGOALS))

# Source configuration
ifdef IS_TEST
	SRCS		:= $(SRCS_CORE) $(SRCS_TEST_CORE)
else
	SRCS		:= $(SRCS_MAIN) $(SRCS_CORE)
endif

# Compiler flags
ifdef IS_DEBUG
	CXX_FLAGS	+= -ggdb -D_GLIBCXX_DEBUG
endif

# Linker flags
ifdef IS_TEST
	LDFLAGS		+= $(TEST_FLAGS)
endif

# Binary name and object directory
ifdef IS_TEST
	NAME		:= $(NAME_TESTS)
	OBJ_DIR		:= $(OBJ_DIR)_tests$(if $(IS_DEBUG),_debug)
else ifdef IS_DEBUG
	NAME		:= $(NAME_DEBUG)
	OBJ_DIR		:= $(OBJ_DIR)_debug
endif

OBJS			:= $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
DEPS			:= $(OBJS:.o=.d)

# Define test run command
define run_tests
	@if [ -z "$(FILTER)" ]; then \
		$(1) ./$(NAME) --gtest_break_on_failure; \
	else \
		echo "Running tests matching: $(FILTER)"; \
		$(1) ./$(NAME) --gtest_filter=$(FILTER) --gtest_break_on_failure; \
	fi
endef

# Rules
all: $(NAME)

debug: $(NAME)

test: $(NAME)
	$(call run_tests,)

debug_test: $(NAME)
	$(call run_tests,gdb --args)

$(NAME): $(OBJS)
	$(CXX) $(CXX_FLAGS) $(OBJS) $(LDFLAGS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) $(INCLUDE_FLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf obj obj_debug obj_tests obj_tests_debug

fclean: clean
	rm -f $(NAME) $(NAME_DEBUG) $(NAME_TESTS)

re: fclean all

# Dummy targets for filter arguments
ifdef CLASS_ARG
$(CLASS_ARG):
	@:
endif
ifdef TEST_ARG
$(TEST_ARG):
	@:
endif

.PHONY: all debug test debug_test clean fclean re
