CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O3 -fPIC
LIB_NAME = libdryad.a

INC = -Iinclude

PKGS = alsa
PKG_CFLAGS = $(shell pkg-config --cflags $(PKGS))
PKG_LIBS   = $(shell pkg-config --libs $(PKGS))

ALL_CFLAGS = $(CFLAGS) $(INC) $(PKG_CFLAGS)

SRCS = src/dryad.c

OBJS = $(SRCS:%.c=build/%.o)

all: $(LIB_NAME)

$(LIB_NAME): $(OBJS)
	@echo "Archiving $@..."
	@ar rcs $@ $(OBJS)

build/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "CC $<"
	@$(CC) $(ALL_CFLAGS) -c $< -o $@

clean:
	rm -rf build $(LIB_NAME)

.PHONY: all clean