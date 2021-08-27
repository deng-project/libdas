/// libdas: DENG asset loader and assembler library
/// licence: Apache, see LICENCE file
/// file: dam.h - deng asset manager header file
/// author: Karl-Mihkel Ott


#ifndef __DAM_H
#define __DAM_H


/// DAM has different versioning than DENG
#define __DAM_VERSION "0.1.1"

#ifdef __DAM_C
    #define __DAM_PREVIEW_TITLE     "DAM"
    #include <stdlib.h>
    #include <stdio.h>
    #include <time.h>
    #include <string.h>

    #include <data/assets.h>
    #include <data/das_file.h>
    #include <data/das_loader.h>
    #include <data/das_static_assembler.h>
    #include <data/wavefront_obj.h>
#endif


#ifdef __DAM_C
    #define MAX_STR    4096
    /// Structure for containing all info about assembling a new asset
    #define VUU     0x76757500
    #define VUN     0x76756e00
    #define VMU     0x766d7500
    #define VMN     0x766d6e00

    typedef struct dam_Options {
        char *input;
        char *das_file;
        char *meta;
        uint32_t vmode;
        bool show_finfo;
    } dam_AssemblyInfo;


    const char *__help_text = 
        "Basic usage of DAM: dam [OPTIONS] <DAS_FILE>\n" \
        "-h / --help -- view help text\n" \
        "-i / --input <INPUT> -- input file name\n" \
        "-v / --vert <vuu, vun, vmu, vmn> -- specify the new asset vertices mode\n" \
        "-nm / --no-meta -- display information about asset without any metadata (DEFAULT)\n" \
        "-m / --meta -- read metadata from stdin\n";

    /*
     * Read information about the asset
     * Information is following:
     *  - Name of the asset
     *  - Date and time of the creation
     *  - Type of vertices
     *  - Total count of unique vertices
     *  - Total count of drawn vertices
     */
    static void __dam_ListAsset(const char *file_name);


    /// Assemble deng asset from specified asset assembly flags
    static void __dam_AssembleAsset(das_Options *opt);

    
    /// Parse all user input data
    static das_Options __dam_ParseInput(int argc, char *argv[]);
#endif

#endif

