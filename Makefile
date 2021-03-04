ifeq ($(OS),Windows_NT)
    # is Windows_NT on XP, 2000, 7, Vista, 10...
    detected_OS := Windows
else
    detected_OS := $(shell uname -s)
    # same as "uname -s"
endif

APP=libuv_test

ifeq ($(detected_OS), Windows)
    target = $(APP).exe
endif

ifeq ($(detected_OS), Linux)
    target = $(APP)
endif

all: $(target)	
	@echo $(target)
	@echo $(detected_OS)

LINKER_DEBUG:=

ifneq ($(V),)
LINKER_DEBUG = -Wl,--verbose
endif

%.o: %.c 
	gcc -c $< -I/usr/include -Wall -Wpedantic -o $@


$(target): libuv_test.o
	gcc $^ -L/usr/lib -luv $(LINKER_DEBUG) -o $@ 


clean:
	rm -f $(APP) *.elf *.exe *.o
