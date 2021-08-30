# libdas: DENG asset handling management library $(HDRS)
# licence: Apache, see LICENCE file $(HDRS)
# file: Makefile - makefile written for GNU make to specify all targets and their rules $(HDRS)
# author: Karl-Mihkel Ott $(HDRS)


include config.mk

all: $(ALL_TARGETS) $(HDRS)
	ar rcs libdas.a $(LIBDAS_OBJ) 
	$(CC) $(DAM_OBJ) -o dam -L . -ldas


#############################################
###### Test targets (built explicitly) ######
#############################################


# Loading test section
ldtest: $(LD_TEST_OBJ) $(HDRS)
	$(CC) $(LD_TEST_OBJ) -o ldtest

ldtest.c.o: $(SRCDIR)/ldtest.c $(HDRS)
	$(CC) -c $(SRCDIR)/ldtest.c $(CFLAGS) $(INCL_FLAGS) -o ldtest.c.o


# Assembly test section
asmtest: $(ASM_TEST_OBJ) $(HDRS)
	$(CC) $(ASM_TEST_OBJ) -o asmtest

asmtest.c.o: $(SRCDIR)/asmtest.c $(HDRS)
	$(CC) -c $(SRCDIR)/asmtest.c $(CFLAGS) $(INCL_FLAGS) -o asmtest.c.o


#########################################
###### General object file targets ######
#########################################

dam.c.o: $(SRCDIR)/dam.c $(HDRS)
	$(CC) -c $(SRCDIR)/dam.c $(CFLAGS) $(INCL_FLAGS) -o dam.c.o

das_asset_assembler.c.o: $(SRCDIR)/das_asset_assembler.c $(HDRS)
	$(CC) -c $(SRCDIR)/das_asset_assembler.c $(CFLAGS) $(INCL_FLAGS) -o das_asset_assembler.c.o

das_loader.c.o: $(SRCDIR)/das_loader.c $(HDRS)
	$(CC) -c $(SRCDIR)/das_loader.c $(CFLAGS) $(INCL_FLAGS) -o das_loader.c.o

hashmap.c.o: $(SRCDIR)/hashmap.c $(HDRS)
	$(CC) -c $(SRCDIR)/hashmap.c $(CFLAGS) $(INCL_FLAGS) -o hashmap.c.o

tex_loader.c.o: $(SRCDIR)/tex_loader.c $(HDRS)
	$(CC) -c $(SRCDIR)/tex_loader.c $(CFLAGS) $(INCL_FLAGS) -o tex_loader.c.o

uuid.c.o: $(SRCDIR)/uuid.c $(HDRS)
	$(CC) -c $(SRCDIR)/uuid.c $(CFLAGS) $(INCL_FLAGS) -o uuid.c.o

wobj.c.o: $(SRCDIR)/wobj.c $(HDRS)
	$(CC) -c $(SRCDIR)/wobj.c $(CFLAGS) $(INCL_FLAGS) -o wobj.c.o



.PHONY: clean $(HDRS)
clean:
	rm -rf *.o $(CLEAN_TARGETS)
