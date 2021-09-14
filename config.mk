# libdas: DENG asset handling management library
# licence: Apache, see LICENCE file
# file: config.mk - makefile configuration file that contains information about
# 					object targets, final targets and compiler flags
# author: Karl-Mihkel Ott

CXX = g++
INCL_FLAGS = -I include
FLAGS = -std=c++11 -march=native -Wall -g
SRCDIR = src
DSTDIR = .

# All object files that are used to compile libdas library
LIBDAS_OBJ = $(DSTDIR)/das_asset_writer.cpp.o \
			 $(DSTDIR)/huf.cpp.o \
			 $(DSTDIR)/tex_loader.cpp.o \
	  		 $(DSTDIR)/das_loader.cpp.o \
	  		 $(DSTDIR)/uuid.cpp.o \
	  		 $(DSTDIR)/wobj.cpp.o \

# All object files that are used to compile dam executable
DAM_OBJ = $(DSTDIR)/dam.cpp.o

# All object files that are used to compile objdump binary
OBJDUMP_OBJ = $(DSTDIR)/objdump.cpp.o


###################################################
######## Test program object files targets ########
###################################################

LD_TEST_OBJ = $(DSTDIR)/das_loader.cpp.o \
			  $(DSTDIR)/ldtest.cpp.o

ASM_TEST_OBJ = $(DSTDIR)/asmtest.cpp.o \
			   $(DSTDIR)/das_asset_writer.cpp.o \
			   $(DSTDIR)/uuid.cpp.o \
			   $(DSTDIR)/wobj.cpp.o

IMG_TEST_OBJ = $(DSTDIR)/imgtest.cpp.o \
			   $(DSTDIR)/tex_loader.cpp.o \
			   $(DSTDIR)/uuid.cpp.o

MESH_QUERY_OBJ = $(DSTDIR)/das_loader.cpp.o \
				 $(DSTDIR)/mesh_query.cpp.o \
				 $(DSTDIR)/uuid.cpp.o \
				 $(DSTDIR)/hashmap.cpp.o

TRIANGULATE_OBJ = $(DSTDIR)/das_loader.cpp.o \
				  $(DSTDIR)/hashmap.cpp.o \
				  $(DSTDIR)/triangulate.cpp.o \
				  $(DSTDIR)/uuid.cpp.o \
				  $(DSTDIR)/wobj.cpp.o

HUF_TEST_OBJ = $(DSTDIR)/huf.cpp.o \
			   $(DSTDIR)/huf_test.cpp.o


CXX_HM_TEST_OBJ = $(DSTDIR)/huf_encode.cpp.o

HDRS = include/*.h \

ALL_TARGETS = libdas.a \
			  dam \
			  objdump
