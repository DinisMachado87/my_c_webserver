NAME			:= libutils.a
NAME_TESTS		:= utils_tests
CXX				:= c++
CXX_FLAGS		:= -Wall -Werror -Wextra -std=c++98 -MMD -MP
TEST_FLAGS		:= -lgtest -lgtest_main -lpthread
AR				:= ar rcs
OBJ_DIR			:= obj

FILTER_ARG		:= $(word 2,$(MAKECMDGOALS))
ifdef FILTER_ARG
  FILTER		:= *$(FILTER_ARG)*
endif

SRCS			:= StrView.cpp Clock.cpp StrViewMap.cpp Hex.cpp
TEST_SRCS		:= test_StrView.cpp test_StrViewMap.cpp test_Hex.cpp

IS_DEBUG		:= $(filter debug debug_test,$(MAKECMDGOALS))
IS_TEST			:= $(filter test debug_test,$(MAKECMDGOALS))

ifdef IS_DEBUG
	CXX_FLAGS	+= -ggdb -D_GLIBCXX_DEBUG
endif

ifdef IS_TEST
	OBJ_DIR		:= $(OBJ_DIR)_tests$(if $(IS_DEBUG),_debug)
else ifdef IS_DEBUG
	OBJ_DIR		:= $(OBJ_DIR)_debug
endif

LIB_OBJS		:= $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJS		:= $(TEST_SRCS:%.cpp=$(OBJ_DIR)/%.o)
DEPS			:= $(LIB_OBJS:.o=.d) $(if $(IS_TEST),$(TEST_OBJS:.o=.d))

define run_tests
	@if [ -z "$(FILTER)" ]; then \
		$(1) ./$(NAME_TESTS) --gtest_break_on_failure; \
	else \
		echo "Running tests matching: $(FILTER)"; \
		$(1) ./$(NAME_TESTS) --gtest_filter=$(FILTER) --gtest_break_on_failure; \
	fi
endef

all: $(NAME)

debug: $(NAME)

test: $(NAME_TESTS)
	$(call run_tests,)

debug_test: $(NAME_TESTS)
	$(call run_tests,gdb --args)

$(NAME): $(LIB_OBJS)
	$(AR) $(NAME) $(LIB_OBJS)

$(NAME_TESTS): $(LIB_OBJS) $(TEST_OBJS)
	$(CXX) $(CXX_FLAGS) $(LIB_OBJS) $(TEST_OBJS) $(TEST_FLAGS) -o $(NAME_TESTS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf obj obj_debug obj_tests obj_tests_debug

fclean: clean
	rm -f libutils.a libutils_debug.a $(NAME_TESTS)

re: fclean all

ifdef FILTER_ARG
$(FILTER_ARG):
	@:
endif

.PHONY: all debug test debug_test clean fclean re
