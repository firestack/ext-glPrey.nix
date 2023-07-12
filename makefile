
CC ?= gcc
RM ?= rm -f
PKGCONFIG ?= pkg-config
SDL2CONFIG ?= sdl2-config

CFLAGS += -std=c99 -pedantic -Wall -Wextra $(shell $(PKGCONFIG) sdl2 gl glu --cflags)
LDFLAGS += $(shell $(PKGCONFIG) sdl2 gl glu --libs) -lm

ifdef DEBUG
CFLAGS += -g3 -fsanitize=address,undefined
endif

SOURCES = glprey.c wad.c bsp.c mip.c
SOURCES_BSP2PLY = bsp2ply.c bsp.c

all: clean glprey bsp2ply

glprey: $(SOURCES)
	$(CC) -o glprey $(SOURCES) $(LDFLAGS) $(CFLAGS)

bsp2ply: $(SOURCES_BSP2PLY)
	$(CC) -o bsp2ply $(SOURCES_BSP2PLY) $(CFLAGS)

clean:
	$(RM) glprey bsp2ply *.o *.exe
