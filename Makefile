# libdas: DENG asset handling management library
# licence: Apache, see LICENCE file
# file: Makefile - makefile written for GNU make to specify all targets and their rules
# author: Karl-Mihkel Ott


include config.mk

# All main targets to make
all: $(ALL_TARGETS) $(HDRS)
	ar rcs libdas.a $(LIBDAS_OBJ) 
	$(CC) $(DAM_OBJ) -o dam -L . -ldas

libdas.a: $(LIBDAS_OBJ) $(HDRS)
	ar rcs libdas.a $(LIBDAS_OBJ)

dam: $(DAM_OBJ) $(HDRS)
	$(CC) $(DAM_OBJ) -o dam -L . -ldas

objdump: $(OBJDUMP_OBJ) $(HDRS)
	$(CC) $(OBJDUMP_OBJ) -o objdump -L . -ldas


#############################################
###### Test targets (built explicitly) ######
#############################################


# das loader test 
ldtest: $(LD_TEST_OBJ) $(HDRS)
	$(CC) $(LD_TEST_OBJ) -o ldtest

$(DSTDIR)/ldtest.c.o: $(SRCDIR)/ldtest.c $(HDRS)
	$(CC) -c $(SRCDIR)/ldtest.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/ldtest.c.o


# das assembly test 
asmtest: $(ASM_TEST_OBJ) $(HDRS)
	$(CC) $(ASM_TEST_OBJ) -o asmtest

$(DSTDIR)/asmtest.c.o: $(SRCDIR)/asmtest.c $(HDRS)
	$(CC) -c $(SRCDIR)/asmtest.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/asmtest.c.o


# Image loader test
imgtest: $(IMG_TEST_OBJ) $(HDRS)
	$(CC) $(IMG_TEST_OBJ) -o imgtest -lm

$(DSTDIR)/imgtest.c.o: $(SRCDIR)/imgtest.c $(HDRS)
	$(CC) -c $(SRCDIR)/imgtest.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/imgtest.c.o


# Manifold mesh triangle neighbour finder algorithm implementation / test
tri_nbral: $(MESH_QUERY_OBJ) $(HDRS)
	$(CC) $(MESH_QUERY_OBJ) -o mesh_query

$(DSTDIR)/tri_nbral.c.o: $(SRCDIR)/tests/mesh_query.c
	$(CC) -c $(SRCDIR)/tests/tri_nbral.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/tri_nbral.c.o

# Huffman encoding test
huf_encode: $(CXX_HM_TEST_OBJ)
	$(CXX) $(CXX_HM_TEST_OBJ) -o huf_encode

huf_encode.cpp.o: $(SRCDIR)/tests/huf_encode.cpp 
	$(CXX) -c $(SRCDIR)/tests/huf_encode.cpp $(CXXFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/huf_encode.cpp.o


# Non-triangle faced mesh triangulation implementation
triangulate: $(TRIANGULATE_OBJ) $(HDRS)
	$(CC) $(TRIANGULATE_OBJ) -o triangulate

$(DSTDIR)/triangulate.c.o: $(SRCDIR)/tests/triangulate.c
	$(CC) -c $(SRCDIR)/tests/triangulate.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/triangulate.c.o



#########################################
###### General object file targets ######
#########################################

$(DSTDIR)/dam.c.o: $(SRCDIR)/dam.c $(HDRS)
	$(CC) -c $(SRCDIR)/dam.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/dam.c.o

$(DSTDIR)/das_asset_assembler.c.o: $(SRCDIR)/das_asset_assembler.c $(HDRS)
	$(CC) -c $(SRCDIR)/das_asset_assembler.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/das_asset_assembler.c.o

$(DSTDIR)/das_loader.c.o: $(SRCDIR)/das_loader.c $(HDRS)
	$(CC) -c $(SRCDIR)/das_loader.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/das_loader.c.o

$(DSTDIR)/hashmap.c.o: $(SRCDIR)/hashmap.c $(HDRS)
	$(CC) -c $(SRCDIR)/hashmap.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/hashmap.c.o

$(DSTDIR)/objdump.c.o: $(SRCDIR)/objdump.c $(HDRS)
	$(CC) -c $(SRCDIR)/objdump.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/objdump.c.o

$(DSTDIR)/tex_loader.c.o: $(SRCDIR)/tex_loader.c $(HDRS)
	$(CC) -c $(SRCDIR)/tex_loader.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/tex_loader.c.o

$(DSTDIR)/uuid.c.o: $(SRCDIR)/uuid.c $(HDRS)
	$(CC) -c $(SRCDIR)/uuid.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/uuid.c.o

$(DSTDIR)/wobj.c.o: $(SRCDIR)/wobj.c $(HDRS)
	$(CC) -c $(SRCDIR)/wobj.c $(CFLAGS) $(INCL_FLAGS) -o $(DSTDIR)/wobj.c.o




.PHONY: clean $(HDRS)
clean:
	rm -rf *.o $(CLEAN_TARGETS)
