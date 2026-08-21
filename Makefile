NAME_BASE   := webserver
CXX         := c++
CXX_FLAGS   := -Wall -Werror -Wextra -std=c++98 -MMD -MP
GTEST_FLAGS := -lgtest -lgtest_main -lpthread
UTILS_DIR   := srcs/utils

# ---- Verbosity toggle: make V=1 echoes raw commands ----
Q := $(if $(V),,@)

# ---- Axes (derived from goals) ----
KEY_WORDS   := all debug test re clean fclean bear utilsTest utilsDebugTest
SELECT      := $(filter-out $(KEY_WORDS),$(MAKECMDGOALS))
IS_DEBUG    := $(filter debug,$(MAKECMDGOALS))
IS_TEST     := $(filter test,$(MAKECMDGOALS))
.DEFAULT_GOAL := all

# ---- Tree: debug forks obj; test is link-only (shares source .o) ----
ifdef IS_DEBUG
	OBJ_DIR   := obj_debug
	CXX_FLAGS += -ggdb -D_GLIBCXX_DEBUG
else
	OBJ_DIR   := obj
endif

# ---- Accumulators (filled by module.mk) ----
MODULES       :=
INCLUDE_DIRS  := srcs srcs/utils

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

# ---- Derive per-module vars from names (module name == dir) ----
# $(1) = module name
define derive_module
$(if $(TRACE),$(info [mk] derive $(1): files=$($(1)_FILES)))
$(1)_SRCS      := $$(addprefix srcs/$(1)/,$$($(1)_FILES))
$(1)_TEST_SRCS := $$(wildcard srcs/$(1)/test_*.cpp)
INCLUDE_DIRS   += srcs/$(1)
endef
$(foreach m,$(MODULES),$(eval $(call derive_module,$(m))))

$(if $(TRACE),$(info [mk] MODULES=$(MODULES)))

INCLUDE_FLAGS := $(addprefix -I,$(INCLUDE_DIRS))

# ---- Transitive dependency closure ----

# direct_deps: deps of a module set, one level
#   $(1) = modules
direct_deps = $(sort $(foreach mod,$(1),$(DEPS_$(mod))))
# walk: BFS engine — expand frontier until empty
#   $(1) = seen (expanded)   $(2) = frontier (to expand)
# frontier filtered against BOTH seen and itself — kills 2-node cycles
define find_dependencies
$(if $(2),$(call find_dependencies,$(1) $(2),$(filter-out $(1) $(2),$(sort $(call direct_deps,$(2))))),$(1))
endef
# closure: full transitive deps of selected modules
#   $(1) = starting modules
closure = $(sort $(call find_dependencies,,$(1)))

# ---- Selection (needs MODULES from includes) ----
MODULE_ARG  := $(filter $(MODULES),$(SELECT))
FILTER_ARG  := $(filter-out $(MODULES),$(SELECT))

# ---- Parse-phase trace: make ... TRACE=1 ----
ifdef TRACE
$(info [mk] SELECT=$(SELECT))
$(info [mk] MODULE_ARG=$(MODULE_ARG))
$(info [mk] FILTER_ARG=$(FILTER_ARG))
endif

ifdef MODULE_ARG
	ifndef IS_TEST
		$(error Can only select module for test compilation. Can't link without main — try: make test $(MODULE_ARG))
	endif
	NEED_DIRS     := $(call closure,$(MODULE_ARG))
	SELECTED_TEST := $(foreach m,$(MODULE_ARG),$($(m)_TEST_SRCS))
else
	NEED_DIRS     := $(MODULES)
	SELECTED_TEST := $(foreach m,$(NEED_DIRS),$($(m)_TEST_SRCS))
endif

SELECTED_SRC := $(foreach m,$(NEED_DIRS),$($(m)_SRCS))

# ---- Guard: empty closure means a cycle or a missing DEPS_ ----
ifeq ($(strip $(NEED_DIRS)),)
	$(error [mk] closure produced empty NEED_DIRS for '$(MODULE_ARG)' — dependency cycle or missing DEPS_ entry)
endif

$(if $(TRACE),$(info [mk] NEED_DIRS=$(NEED_DIRS)))

# ---- Object lists ----
ifeq ($(strip $(SELECTED_SRC)),)
	$(error [mk] no sources for NEED_DIRS='$(NEED_DIRS)' — check <dir>_SRCS names match module dirs)
endif

OBJS      := $(SELECTED_SRC:srcs/%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJS := $(SELECTED_TEST:srcs/%.cpp=$(OBJ_DIR)/%.o)
MAIN_OBJ  := $(OBJ_DIR)/main.o

ifdef IS_TEST
	LINK_OBJS := $(OBJS) $(TEST_OBJS)
	LDFLAGS   += $(GTEST_FLAGS)
else
	LINK_OBJS := $(MAIN_OBJ) $(OBJS)
endif

# ---- Binary name per mode ----
NAME		:= $(NAME_BASE)$(if $(IS_DEBUG),_debug)$(if $(IS_TEST),_tests)
UTILS_LIB   := $(UTILS_DIR)/libutils$(if $(IS_DEBUG),_debug).a

# ---- Header dependency tracking (scoped to selection) ----
DEPS := $(OBJS:.o=.d) $(TEST_OBJS:.o=.d) $(MAIN_OBJ:.o=.d)
-include $(DEPS)

# ---- Per-module compile rules ----
define module_rule
$(OBJ_DIR)/$(1)/%.o: srcs/$(1)/%.cpp
	@mkdir -p $$(dir $$@)
	@echo "  CXX     $$<"
	$(Q)$$(CXX) $$(CXX_FLAGS) $$(INCLUDE_FLAGS) -c $$< -o $$@
endef

$(foreach m,$(MODULES),$(eval $(call module_rule,$(m))))

$(MAIN_OBJ): srcs/main.cpp
	@mkdir -p $(dir $@)
	@echo "  CXX     $<"
	$(Q)$(CXX) $(CXX_FLAGS) $(INCLUDE_FLAGS) -c $< -o $@

# ---- Build entry ----
build: $(NAME)

$(NAME): $(LINK_OBJS) $(UTILS_LIB)
	@echo "  LINK    $@"
	$(Q)$(CXX) $(CXX_FLAGS) $(LINK_OBJS) $(UTILS_LIB) $(LDFLAGS) -o $@
	@echo "flags:   $(CXX_FLAGS)"
	@echo "modules: $(NEED_DIRS)"
	@echo "objects: $(words $(LINK_OBJS))"

.PHONY: $(UTILS_LIB)
$(UTILS_LIB):
	@$(MAKE) -C $(UTILS_DIR) $(if $(IS_DEBUG),debug) NAME=$(notdir $@)

# ---- Run helper ----
RUN := $(if $(IS_DEBUG),gdb -ex 'handle SIGPIPE nostop noprint pass' --args,)
GTEST_FILTER := $(if $(FILTER_ARG),--gtest_filter=*$(FILTER_ARG)*)

define run_tests
	@echo
	$(RUN) ./$(NAME) $(GTEST_FILTER) --gtest_break_on_failure
endef

# ---- Goals ----
all:   build
debug: build

test: build
	$(call run_tests)

# module words select + trigger build; filter words absorbed
$(foreach m,$(MODULES),$(eval $(m): build ;))
$(foreach f,$(filter-out $(MODULES),$(FILTER_ARG)),$(eval $(f): ;))

utilsTest:
	$(MAKE) -C $(UTILS_DIR) test $(FILTER_ARG)
utilsDebugTest:
	$(MAKE) -C $(UTILS_DIR) debug_test $(FILTER_ARG)

bear:
	bear -- $(MAKE) re

clean:
	rm -rf obj obj_debug
	$(MAKE) -C $(UTILS_DIR) clean

fclean: clean
	rm -f $(NAME_BASE) $(NAME_BASE)_debug $(NAME_BASE)_tests $(NAME_BASE)_debug_tests build.log
	$(MAKE) -C $(UTILS_DIR) fclean

re: fclean all

.PHONY: all debug test build re clean fclean bear utilsTest utilsDebugTest \
	$(MODULES) $(FILTER_ARG)
