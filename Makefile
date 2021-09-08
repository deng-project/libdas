# libdas: DENG asset handling management library
# licence: Apache, see LICENCE file
# file: Makefile - makefile written for GNU make to specify all targets and their rules
# author: Karl-Mihkel Ott


include config.mk

# All main targets to make
all: $(ALL_TARGETS) $(HDRS)

dam: $(DAM_OBJ) $(HDRS)
	$(CXX) $(DAM_OBJ) -o dam -L . -ldas

hf_comp: $(HF_COMP_OBJ) $(HDRS)
	$(CXX) $(HF_COMP_OBJ) -o hf_comp

libdas.a: $(LIBDAS_OBJ) $(HDRS)
	ar rcs libdas.a $(LIBDAS_OBJ)

objdump: $(OBJDUMP_OBJ) $(HDRS)
	$(CXX) $(OBJDUMP_OBJ) -o objdump -L . -ldas


#############################################
###### Test targets (built explicitly) ######
#############################################


# das loader test 
ldtest: $(LD_TEST_OBJ) $(HDRS)
	$(CXX) $(LD_TEST_OBJ) -o ldtest

$(DSTDIR)/ldtest.cpp.o: $(SRCDIR)/tests/ldtest.cpp $(HDRS)
	$(CXX) -c $(SRCDIR)/tests/ldtest.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/ldtest.cpp.o


# das assembly test 
asmtest: $(ASM_TEST_OBJ) $(HDRS)
	$(CXX) $(ASM_TEST_OBJ) -o asmtest

$(DSTDIR)/asmtest.cpp.o: $(SRCDIR)/tests/asmtest.cpp $(HDRS)
	$(CXX) -c $(SRCDIR)/tests/asmtest.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/asmtest.cpp.o


# Image loader test
imgtest: $(IMG_TEST_OBJ) $(HDRS)
	$(CXX) $(IMG_TEST_OBJ) -o imgtest -lm

$(DSTDIR)/imgtest.cpp.o: $(SRCDIR)/imgtest.cpp $(HDRS)
	$(CXX) -c $(SRCDIR)/imgtest.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/imgtest.cpp.o


# Manifold mesh triangle neighbour finder algorithm implementation / test
tri_nbral: $(MESH_QUERY_OBJ) $(HDRS)
	$(CXX) $(MESH_QUERY_OBJ) -o mesh_query

$(DSTDIR)/tri_nbral.cpp.o: $(SRCDIR)/tests/mesh_query.cpp
	$(CXX) -c $(SRCDIR)/tests/tri_nbral.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/tri_nbral.cpp.o


# Non-triangle faced mesh triangulation implementation
triangulate: $(TRIANGULATE_OBJ) $(HDRS)
	$(CXX) $(TRIANGULATE_OBJ) -o triangulate

$(DSTDIR)/triangulate.cpp.o: $(SRCDIR)/tests/triangulate.cpp
	$(CXX) -c $(SRCDIR)/tests/triangulate.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/triangulate.cpp.o



#########################################
###### General object file targets ######
#########################################

$(DSTDIR)/dam.cpp.o: $(SRCDIR)/dam.cpp $(HDRS)
	$(CXX) -c $(SRCDIR)/dam.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/dam.cpp.o

$(DSTDIR)/das_asset_writer.cpp.o: $(SRCDIR)/das_asset_writer.cpp $(HDRS)
	$(CXX) -c $(SRCDIR)/das_asset_writer.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/das_asset_writer.cpp.o

$(DSTDIR)/das_loader.cpp.o: $(SRCDIR)/das_loader.cpp $(HDRS)
	$(CXX) -c $(SRCDIR)/das_loader.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/das_loader.cpp.o

$(DSTDIR)/hf_comp.cpp.o: $(SRCDIR)/hf_comp.cpp $(HDRS)
	$(CXX) -c $(SRCDIR)/hf_comp.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/hf_comp.cpp.o

$(DSTDIR)/huf.cpp.o: $(SRCDIR)/huf.cpp $(HDRS)
	$(CXX) -c $(SRCDIR)/huf.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/huf.cpp.o

$(DSTDIR)/objdump.cpp.o: $(SRCDIR)/objdump.cpp $(HDRS)
	$(CXX) -c $(SRCDIR)/objdump.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/objdump.cpp.o

$(DSTDIR)/tex_loader.cpp.o: $(SRCDIR)/tex_loader.cpp $(HDRS)
	$(CXX) -c $(SRCDIR)/tex_loader.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/tex_loader.cpp.o

$(DSTDIR)/uuid.cpp.o: $(SRCDIR)/uuid.cpp $(HDRS)
	$(CXX) -c $(SRCDIR)/uuid.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/uuid.cpp.o

$(DSTDIR)/wobj.cpp.o: $(SRCDIR)/wobj.cpp $(HDRS)
	$(CXX) -c $(SRCDIR)/wobj.cpp $(FLAGS) $(INCL_FLAGS) -o $(DSTDIR)/wobj.cpp.o


# Clean up task
.PHONY: clean $(HDRS)
clean:
	rm -rf $(DSTDIR)/*.o $(ALL_TARGETS)
