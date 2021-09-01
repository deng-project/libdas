# libdas: DENG asset handling management library
# licence: Apache, see LICENCE file
# file: config.mk - makefile configuration file that contains information about
# 					object targets, final targets and compiler flags
# author: Karl-Mihkel Ott

CC = gcc
INCL_FLAGS = -I include
CFLAGS = -std=c99 -march=native -Wall -g
SRCDIR = src
DSTDIR = .

# All object files that are used to compile libdas library
LIBDAS_OBJ = $(DSTDIR)/das_asset_assembler.c.o \
	  		 $(DSTDIR)/das_loader.c.o \
			 $(DSTDIR)/tex_loader.c.o \
	  		 $(DSTDIR)/hashmap.c.o \
	  		 $(DSTDIR)/uuid.c.o \
	  		 $(DSTDIR)/wobj.c.o

# All object files that are used to compile dam executable
DAM_OBJ = $(DSTDIR)/dam.c.o

LD_TEST_OBJ = $(DSTDIR)/das_loader.c.o \
			  $(DSTDIR)/ldtest.c.o

ASM_TEST_OBJ = $(DSTDIR)/asmtest.c.o \
			   $(DSTDIR)/das_asset_assembler.c.o \
			   $(DSTDIR)/das_loader.c.o \
			   $(DSTDIR)/hashmap.c.o \
			   $(DSTDIR)/uuid.c.o \
			   $(DSTDIR)/wobj.c.o

IMG_TEST_OBJ = $(DSTDIR)/imgtest.c.o \
			   $(DSTDIR)/tex_loader.c.o \
			   $(DSTDIR)/uuid.c.o

MESH_QUERY_OBJ = $(DSTDIR)/das_loader.c.o \
				 $(DSTDIR)/mesh_query.c.o \
				 $(DSTDIR)/uuid.c.o \
				 $(DSTDIR)/hashmap.c.o

TRIANGULATE_OBJ = $(DSTDIR)/das_loader.c.o \
				  $(DSTDIR)/hashmap.c.o \
				  $(DSTDIR)/triangulate.c.o \
				  $(DSTDIR)/uuid.c.o \
				  $(DSTDIR)/wobj.c.o

HDRS = include/** \

ALL_TARGETS = $(LIBDAS_OBJ) \
			  $(DAM_OBJ)

CLEAR_TARGETS = libdas.so \
				libdas.a \
				dam \
				$(LIBDAS_OBJ) \
				$(DAM_OBJ) \
				ldtest \
				asmtest \
