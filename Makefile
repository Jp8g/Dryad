CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O3 -fPIC
LIB_NAME = libdryad.a

INC_DIRS = include
INC_FLAGS = $(addprefix -I,$(INC_DIRS))

PKGS =

BACKEND ?= ALSA

ifeq ($(BACKEND),ALSA)
    CFLAGS += -DDRYAD_ALSA
    PKGS += alsa
endif

PKG_CFLAGS = $(shell pkg-config --cflags $(PKGS))
PKG_LIBS   = $(shell pkg-config --libs $(PKGS))

SRCS = $(shell find src -name "*.c")
OBJS = $(SRCS:%.c=build/%.o)
DEPS = $(OBJS:.o=.d)

all: $(LIB_NAME)

-include $(DEPS)

$(LIB_NAME): $(OBJS)
	@echo "Archiving $@..."
	@ar rcs $@ $(OBJS)

build/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(PKG_CFLAGS) $(INC_FLAGS) -MMD -MP -c $< -o $@

clean:
	@echo "Cleaning..."
	rm -rf build $(LIB_NAME)

print-pkgs:
	@echo $(PKGS)

print-incs:
	@echo $(INC_DIRS)

.PHONY: all clean print-pkgs print-incs