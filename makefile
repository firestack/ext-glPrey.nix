
CC ?= gcc
RM ?= rm -f
PKGCONFIG ?= pkg-config
SDL2CONFIG ?= sdl2-config

CFLAGS += -std=c99 -pedantic -Wall -Wextra
LDFLAGS += -lm

CFLAGS_GLPREY = $(shell $(PKGCONFIG) sdl2 gl glu --cflags)
LDFLAGS_GLPREY = $(shell $(PKGCONFIG) sdl2 gl glu --libs)

ifdef DEBUG
CFLAGS += -DDEBUG=1 -g3 -fsanitize=address,undefined
endif

SOURCES = glprey.c wad.c bsp.c mip.c
SOURCES_BSP2PLY = bsp2ply.c bsp.c
SOURCES_WAD2PNG = wad2png.c wad.c mip.c

all: clean glprey bsp2ply wad2png

glprey: $(SOURCES)
	$(CC) -o glprey $(SOURCES) $(LDFLAGS) $(CFLAGS) $(CFLAGS_GLPREY) $(LDFLAGS_GLPREY)

bsp2ply: $(SOURCES_BSP2PLY)
	$(CC) -o bsp2ply $(SOURCES_BSP2PLY) $(LDFLAGS) $(CFLAGS)

wad2png: $(SOURCES_WAD2PNG)
	$(CC) -o wad2png $(SOURCES_WAD2PNG) $(LDFLAGS) $(CFLAGS)

clean:
	$(RM) glprey bsp2ply wad2png *.o *.exe
