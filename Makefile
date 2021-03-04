ifeq ($(OS),Windows_NT)
    # is Windows_NT on XP, 2000, 7, Vista, 10...
    detected_OS := Windows
else
    detected_OS := $(shell uname -s)
    # same as "uname -s"
endif

APP=libuv_test

ifeq ($(detected_OS), Windows)
    target = build_$(detected_OS)/$(APP).exe
endif

ifeq ($(detected_OS), Linux)
    target = build_$(detected_OS)/$(APP)
endif

LINKER_DEBUG:=

ifneq ($(V),)
LINKER_DEBUG = -Wl,--verbose
endif

ifneq ($(DEBUG),)
OPTIMIZE_FLAGS      := -Og 
DEBUG_FLAGS            = -ggdb3 -DDEBUG 
else
OPTIMIZE_FLAGS      := -O2
ifeq ($(DEBUG),INFO)
DEBUG_FLAGS            = -ggdb3
endif
endif

all: $(target)	
	@echo $(target)
	@echo $(detected_OS)

build_$(detected_OS)/%.o: %.c 
	mkdir -p build_$(detected_OS)/
	gcc -c $< -I/usr/include -Wall -Wpedantic $(OPTIMIZE_FLAGS) $(DEBUG_FLAGS) -o $@

$(target): build_$(detected_OS)/libuv_test.o 
	gcc $^ -L/usr/lib -luv $(LINKER_DEBUG) -o $@ 


clean:
	rm -f $(APP) *.elf *.exe *.o 
	rm -rf build_*
