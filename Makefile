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

OBJECT_DIR = build_$(detected_OS)

run: $(target)
	$(target)

all: $(target)	
	@echo $(target)
	@echo $(detected_OS)

$(OBJECT_DIR)/%.o: %.c 
	mkdir -p $(OBJECT_DIR)
	gcc -c $< -I/usr/include -Wall -Wpedantic $(OPTIMIZE_FLAGS) $(DEBUG_FLAGS) -o $@

SRC := libuv_test.c wmq_error.c debug.c

TARGET_OBJS     = $(addsuffix .o,$(addprefix $(OBJECT_DIR)/,$(basename $(SRC))))

DLL_DEPS :=
ifeq ($(detected_OS), Windows)
	DLL_DEPS += $(OBJECT_DIR)/libuv-1.dll
endif

$(OBJECT_DIR)/libuv-1.dll:
	cp /mingw64/bin/libuv-1.dll $(OBJECT_DIR)/

$(target): $(TARGET_OBJS) $(DLL_DEPS)
	gcc $^ -L/usr/lib $(OPTIMIZE_FLAGS) $(DEBUG_FLAGS) -luv $(LINKER_DEBUG) -o $@ 

clean:
	rm -f $(APP) *.elf *.exe *.o 
	rm -rf build_*
