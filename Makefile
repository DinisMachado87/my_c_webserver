NAME        := webserver
NAME_DEBUG  := $(NAME)_debug
NAME_TESTS  := $(NAME)_tests
CXX         := c++
CXX_FLAGS   := -Wall -Werror -Wextra -std=c++98 -MMD -MP
TEST_FLAGS  := -lgtest -lgtest_main -lpthread
LDFLAGS     :=
OBJ_DIR     := obj

# Filter arguments (e.g., "make test Connection SpecificTest")
CLASS_ARG   := $(word 2,$(MAKECMDGOALS))
TEST_ARG    := $(word 3,$(MAKECMDGOALS))
ifdef CLASS_ARG
	ifdef TEST_ARG
	FILTER  := *$(CLASS_ARG)*.$(TEST_ARG)
else
	FILTER  := *$(CLASS_ARG)*
endif
endif

# Detect build mode
IS_DEBUG    := $(filter debug debug_test,$(MAKECMDGOALS))
IS_TEST     := $(filter test debug_test,$(MAKECMDGOALS))

ifdef IS_DEBUG
	CXX_FLAGS += -ggdb -D_GLIBCXX_DEBUG
endif
ifdef IS_TEST
	LDFLAGS   += $(TEST_FLAGS)
endif
ifdef IS_TEST
	OBJ_DIR   := $(OBJ_DIR)_tests$(if $(IS_DEBUG),_debug)
else ifdef IS_DEBUG
	OBJ_DIR   := $(OBJ_DIR)_debug
endif

#  Utils (stays a library) 
UTILS_DIR   := srcs/utils
UTILS_LIB   := $(UTILS_DIR)/libutils.a
LIB_TARGET  := $(if $(IS_DEBUG),debug,all)

#  Accumulator variables 
SRCS_ALL      :=
TEST_SRCS_ALL := 
MODULES       := 
INCLUDE_DIRS  := srcs srcs/utils

#  Include module fragments 
include srcs/buffers/module.mk
include srcs/httpBody/module.mk
include srcs/engine/module.mk
include srcs/server/module.mk
include srcs/http/module.mk
include srcs/httpParser/module.mk
include srcs/responses/module.mk
include srcs/path/module.mk
include srcs/sockets/module.mk
include srcs/parser/module.mk
include srcs/logger/module.mk

#  Include flags 
INCLUDE_FLAGS := $(addprefix -I,$(INCLUDE_DIRS))

#  Object lists 
ALL_OBJS      := $(SRCS_ALL:srcs/%.cpp=$(OBJ_DIR)/%.o)
ALL_TEST_OBJS := $(TEST_SRCS_ALL:srcs/%.cpp=$(OBJ_DIR)/%.o)
MAIN_OBJ      := $(OBJ_DIR)/main.o

#  Test filtering 
ifdef CLASS_ARG
	ACTIVE_TEST_OBJS := $(filter %/test_$(CLASS_ARG).o,$(ALL_TEST_OBJS))
else
	ACTIVE_TEST_OBJS := $(ALL_TEST_OBJS)
endif

#  Select binary name 
ifdef IS_TEST
	NAME    := $(NAME_TESTS)
else ifdef IS_DEBUG
	NAME    := $(NAME_DEBUG)
endif

#  Compile rules 
define module_rule
$(OBJ_DIR)/$(1)/%.o: srcs/$(1)/%.cpp
	@mkdir -p $$(dir $$@)
	$$(CXX) $$(CXX_FLAGS) $$(INCLUDE_FLAGS) -c $$< -o $$@
endef

$(foreach mod,$(MODULES),$(eval $(call module_rule,$(mod))))

$(MAIN_OBJ): srcs/main.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) $(INCLUDE_FLAGS) -c $< -o $@

#  Dependencies 
DEPS := $(ALL_OBJS:.o=.d) $(ALL_TEST_OBJS:.o=.d) $(MAIN_OBJ:.o=.d)
-include $(DEPS)

#  Test runner 
define run_tests
@if [ -z "$(FILTER)" ]; then \
	$(1) ./$(NAME) gtest_break_on_failure; \
	else \
	echo "Running tests matching: $(FILTER)"; \
	$(1) ./$(NAME) gtest_filter=$(FILTER) gtest_break_on_failure; \
	fi
endef

#  Utils library 
$(UTILS_LIB):
	$(MAKE) -C $(UTILS_DIR) $(LIB_TARGET)

#  Main targets 
all: $(NAME)
debug: $(NAME)

$(NAME): $(if $(IS_TEST),$(ALL_OBJS) $(ACTIVE_TEST_OBJS),$(MAIN_OBJ) $(ALL_OBJS)) $(UTILS_LIB)
	$(CXX) $(CXX_FLAGS) $^ $(LDFLAGS) -o $@

#  Full test targets 
test: $(NAME)
	$(call run_tests,)

debug_test: $(NAME)
	$(call run_tests,gdb args)

#  Per-module test targets 
buffers_tests: $(BUFFERS_OBJS) $(BUFFERS_TEST_OBJS) $(UTILS_LIB)
	$(CXX) $(CXX_FLAGS) $^ $(TEST_FLAGS) -o $@

body_tests: $(BODY_OBJS) $(BODY_TEST_OBJS) $(BUFFERS_OBJS) $(UTILS_LIB)
	$(CXX) $(CXX_FLAGS) $^ $(TEST_FLAGS) -o $@

#  Per-module test runners 
buffersTest: buffers_tests
	@if [ -z "$(FILTER)" ]; then \
		./buffers_tests --gtest_break_on_failure; \
	else \
		echo "Running tests matching: $(FILTER)"; \
		./buffers_tests --gtest_filter=$(FILTER) --gtest_break_on_failure; \
	fi

bodyTest: body_tests
	@if [ -z "$(FILTER)" ]; then \
		./body_tests --gtest_break_on_failure; \
	else \
		echo "Running tests matching: $(FILTER)"; \
		./body_tests --gtest_filter=$(FILTER) --gtest_break_on_failure; \
	fi

buffersDebugTest: CXX_FLAGS += -ggdb -D_GLIBCXX_DEBUG
buffersDebugTest: buffers_tests
	$(call run_tests,gdb args)

bodyDebugTest: CXX_FLAGS += -ggdb -D_GLIBCXX_DEBUG
bodyDebugTest: body_tests
	$(call run_tests,gdb args)

#  Libraries 
utilsTest:
	$(MAKE) -C $(UTILS_DIR) test $(CLASS_ARG) $(TEST_ARG)

utilsDebugTest:
	$(MAKE) -C $(UTILS_DIR) debug_test $(CLASS_ARG) $(TEST_ARG)

#  Bear 
bear:
	bear  $(MAKE) re
	bear append  $(MAKE) -C $(UTILS_DIR) re

#  Clean 
clean:
	rm -rf obj obj_debug obj_tests obj_tests_debug
	$(MAKE) -C $(UTILS_DIR) clean

fclean: clean
	rm -f webserver webserver_debug webserver_tests \
		buffers_tests body_tests
	$(MAKE) -C $(UTILS_DIR) fclean

re: fclean all

#  Dummy targets for filter arguments 
ifdef CLASS_ARG
	$(CLASS_ARG):
@:
	endif
	ifdef TEST_ARG
$(TEST_ARG):
	@:
endif

.PHONY: all debug clean fclean re \
	test debug_test \
	buffers_tests body_tests \
	buffersTest bodyTest \
	buffersDebugTest bodyDebugTest \
	utilsTest utilsDebugTest \
	bear
