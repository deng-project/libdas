# For now build only dam, not the library around it

CC = gcc
INCL_FLAGS = -I include
CFLAGS = -Og -std=c99 -march=native -Wall -g
SRCDIR = src
DSTDIR = .
OBJ = $(DSTDIR)/ldtest.c.o \
	  $(DSTDIR)/das_loader.c.o \
	  $(DSTDIR)/das_asset_assembler.c.o \
	  $(DSTDIR)/wobj_tokens.c.o \
	  $(DSTDIR)/wobj.c.o

LD_TEST_OBJ = $(DSTDIR)/ldtest.c.o \
			  $(DSTDIR)/das_loader.c.o

ASM_TEST_OBJ = $(DSTDIR)/asmtest.c.o \
			   $(DSTDIR)/das_asset_assembler.c.o \
			   $(DSTDIR)/das_loader.c.o \
			   $(DSTDIR)/uuid.c.o \
			   $(DSTDIR)/hashmap.c.o \
			   $(DSTDIR)/wobj_tokens.c.o \
			   $(DSTDIR)/wobj.c.o

HDRS = include/** \

ALL_TARGETS = libdas.so \
			  dam


all: $(ALL_TARGETS) $(HDRS)
	$(CC) $(OBJ) -fPIC -o libdas.so
	$(CC) $(OBJ) -o dam


# Loading test section
ldtest: $(LD_TEST_OBJ) $(HDRS)
	$(CC) $(LD_TEST_OBJ) -o ldtest

ldtest.c.o: $(SRCDIR)/ldtest.c
	$(CC) -c $(SRCDIR)/ldtest.c $(CFLAGS) $(INCL_FLAGS) -o ldtest.c.o


# Assembly test section
asmtest: $(ASM_TEST_OBJ)
	$(CC) $(ASM_TEST_OBJ) -o asmtest

asmtest.c.o: $(SRCDIR)/asmtest.c
	$(CC) -c $(SRCDIR)/asmtest.c $(CFLAGS) $(INCL_FLAGS) -o asmtest.c.o



# General libarary source to object compilation targets
dam.c.o: $(SRCDIR)/dam.c
	$(CC) -c $(SRCDIR)/dam.c $(CFLAGS) $(INCL_FLAGS) -o dam.c.o

hashmap.c.o: $(SRCDIR)/hashmap.c
	$(CC) -c $(SRCDIR)/hashmap.c $(CFLAGS) $(INCL_FLAGS) -o hashmap.c.o

uuid.c.o: $(SRCDIR)/uuid.c
	$(CC) -c $(SRCDIR)/uuid.c $(CFLAGS) $(INCL_FLAGS) -o uuid.c.o

das_loader.c.o: $(SRCDIR)/das_loader.c
	$(CC) -c $(SRCDIR)/das_loader.c $(CFLAGS) $(INCL_FLAGS) -o das_loader.c.o

das_asset_assembler.c.o: $(SRCDIR)/das_asset_assembler.c
	$(CC) -c $(SRCDIR)/das_asset_assembler.c $(CFLAGS) $(INCL_FLAGS) -o das_asset_assembler.c.o

wobj_tokens.c.o: $(SRCDIR)/wobj_tokens.c
	$(CC) -c $(SRCDIR)/wobj_tokens.c $(CFLAGS) $(INCL_FLAGS) -o wobj_tokens.c.o

wobj.c.o: $(SRCDIR)/wobj.c
	$(CC) -c $(SRCDIR)/wobj.c $(CFLAGS) $(INCL_FLAGS) -o wobj.c.o


.PHONY: clean
clean:
	rm -rf *.o $(ALL_TARGETS) ldtest asmtest
