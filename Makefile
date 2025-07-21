CXX              := g++
CC               := gcc
#OPTIM_OR_DEBUG   := -O3 -g0
OPTIM_OR_DEBUG   := -O0 -g3 -ggdb -DDEBUG
ifneq (,$(findstring -ggdb,$(OPTIM_OR_DEBUG)))
STRIP            := @echo "Debug mode, don't strip"
else
STRIP            := strip
endif
COMMON_FLAGS     := -I inc -I src/famo -MMD -MP
COMMON_FLAGS     += -D_POSIX_C_SOURCE -D_GNU_SOURCE -D_XOPEN_SOURCE
CXXFLAGS         := $(COMMON_FLAGS) -std=c++20
CXXFLAGS         += -Wall -Wextra -Wformat=2 -Wformat-security -Wstrict-overflow -Wno-unused-result -fstack-protector-all -Werror
CXXFLAGS         += $(OPTIM_OR_DEBUG) -fvisibility=hidden
CFLAGS           := $(COMMON_FLAGS) -std=c2x
CFLAGS           += -Wall -Wextra
LDFLAGS          := -L/usr/lib/x86_64-linux-gnu -lwebsockets
CPP_SRCS         :=\
 src/famo/entrées_sorties_via_fichier_texte.cpp\
 src/famo/entrées_sorties_via_web_socket.cpp\
 src/famo/four_à_micro_ondes.cpp\
 src/famo/websocket_and_http_servers.cpp\
 src/main.cpp
OBJS             := $(C_SRCS:%.c=BUILD/%.o) $(CPP_SRCS:%.cpp=BUILD/%.o)
DEPS             := $(C_SRCS:%=BUILD/%.d)   $(CPP_SRCS:%=BUILD/%.d)
FSM              := exe/fsm
VALGRIND_OPTIONS :=\
 valgrind\
 --leak-check=full\
 --show-leak-kinds=all\
 --track-origins=yes\
 --track-fds=yes\
 --error-exitcode=2\
 --expensive-definedness-checks=yes\
 --max-stackframe=5242880
EFENCE_OPTIONS :=\
 EF_ALLOW_MALLOC_0=1\
 EF_DISABLE_BANNER=1\
 LD_PRELOAD=/usr/lib/libefence.so.0.0
INTERFACE_IN_PATH := "four à micro-ondes-in.txt"
INTERFACE_OUT_PATH := "four à micro-ondes-out.txt"

.PHONY: clean show-deps func-tests func-tests-memcheck func-tests-efence doc

all: $(FSM)

help:
	@printf "\033[1;96m all          \033[0m Build GNU/Linux and Microsoft Windows targets\n"
	@printf "\033[1;96m clean        \033[0m Remove all generated sources, objects, executables and log files\n"
	@printf "\033[1;96m show-deps    \033[0m Show dependencies for debugging Makefile\n"
	@printf "\033[1;96m run          \033[0m Execute $(FUNC_TESTS)\n"
	@printf "\033[1;96m run-memcheck \033[0m Use Valgring tools suite on $(FUNC_TESTS)\n"
	@printf "\033[1;96m run-efence   \033[0m Use Electric Fence tools suite on $(FUNC_TESTS)\n"
	@printf "\033[1;96m doc          \033[0m Generate HTML documentation from source with 'doxygen' tool\n"

clean:
	rm -fR BUILD exe bin $(INTERFACE_IN_PATH) $(INTERFACE_OUT_PATH)

show-deps:
	@echo $(DEPS)

run: $(FSM)
	@xed $(INTERFACE_IN_PATH) &
	$(FSM)

run-memcheck: $(FSM)
	@xed $(INTERFACE_IN_PATH) &
	$(VALGRIND_OPTIONS) $(FSM)

run-efence: $(FSM)
	@xed $(INTERFACE_IN_PATH) &
	$(EFENCE_OPTIONS) $(FSM)

run-ws: $(FSM)
	@(sleep 2 && brave-browser-stable http://localhost:2416)&
	$(FSM) -v -w 2416

run-ws-memcheck: $(FSM)
	@(sleep 2 && brave-browser-stable http://localhost:2416)&
	$(VALGRIND_OPTIONS) $(FSM) -w 2416

doc:
	@rm -fr doc doxygen-*.txt
	@mkdir doc
	doxygen > doxygen-out.txt 2>doxygen-err.txt

$(FSM): $(OBJS)
	@mkdir -p $$(dirname $@)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@
	$(STRIP) $@

BUILD/%.o: %.cpp
	@mkdir -p $$(dirname $@)
	$(CXX) -c $(CXXFLAGS) -MF BUILD/$<.d $< -o $@

BUILD/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC)  -c $(CFLAGS)   -MF BUILD/$<.d $< -o $@

-include $(DEPS)
