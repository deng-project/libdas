/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: wobj.h - wavefront obj parser header
/// author: Karl-Mihkel Ott


#ifndef __WOBJ_H
#define __WOBJ_H

#ifdef __WOBJ_CPP
    #include <iostream>
    #include <fstream>
    #include <string>
    #include <cstring>
    #include <vector>
    #include <unordered_map>

    #define BUFFER_CAP(N, T) N * sizeof(T) 
    #include <uuid.h>
    #include <assets.h>

    /// Syntax error handling macros for Wavefront OBJ files
    #define WAVEFRONT_SYNTAX_ERROR(line, desc)          fprintf(stderr, "Wavefront OBJ syntax error on line %ld: %s\n", line, desc), \
                                                        exit(EXIT_FAILURE)

    #define WAVEFRONT_EMPTY_OBJECT(line)                fprintf(stderr, "Wavefront OBJ error, empty object on line %ld\n", line), \
                                                        exit(EXIT_FAILURE)

    #define WAVEFRONT_EMPTY_GROUP(line)                 fprintf(stderr, "Wavefront OBJ error, empty group specified on line %ld\n", line), \
                                                        exit(EXIT_FAILURE)

    #define TOO_MANY_ARGS(line)                         fprintf(stderr, "Wavefront OBJ error, too many arguments line %ld\n", line), \
                                                        exit(EXIT_FAILURE)
    
    #define NOT_ENOUGH_ARGS(line)                       fprintf(stderr, "Wavefront OBJ error, not enough arguments on line %ld\n", line), \
                                                        exit(EXIT_FAILURE)

    #define TOO_LONG_WORD(line)                         fprintf(stderr, "Wavefront OBJ error, too long word %d\n", line), \
                                                        exit(EXIT_FAILURE)

    #define MEM_ERR(err_msg)                            fprintf(stderr, "Memory allocation error: %s\n", err_msg), \
                                                        exit(EXIT_FAILURE)
#endif

#define STATEMENT_C                                 11
#define DEFAULT_ENTITY_CAP                          8
#define DEFAULT_MEM_CAP                             128
#define MAX_WORD_SIZE                               512


namespace libdas {

    struct IndexBlock {
        uint32_t pos;
        uint32_t tex;
        uint32_t norm;
    };


    enum WavefrontObjSpecType {
        DAS_WAVEFRONT_OBJ_SPEC_TYPE_NONE            = -1,
        DAS_WAVEFRONT_OBJ_SPEC_TYPE_OBJ_DECL        = 0,
        DAS_WAVEFRONT_OBJ_SPEC_TYPE_GROUP_DECL      = 1,
        DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_DECL       = 2,
        DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_TEX_DECL   = 3,
        DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_NORM_DECL  = 4,
        DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_PARAM_DECL = 5,
        DAS_WAVEFRONT_OBJ_SPEC_TYPE_FACE_DECL       = 6,
        DAS_WAVEFRONT_OBJ_SPEC_TYPE_POL_LINE_DECL   = 7,
        DAS_WAVEFRONT_OBJ_SPEC_TYPE_MTL_INCL        = 8,
        DAS_WAVEFRONT_OBJ_SPEC_TYPE_MTL_USE         = 9,
        DAS_WAVEFRONT_OBJ_SPEC_TYPE_SHADING_SPEC    = 10
    };


    /// Structure for defining all statements with their keywords and valid object types that could be used
    struct WavefrontObjStatement {
        const char *keyword;
        uint32_t min_obj_c;
        uint32_t max_obj_c;
        WavefrontObjSpecType spec_type;
    };

    /// Type specifier for entities used in OBJ files
    enum WavefrontObjEntityType {
        DAS_WAVEFRONT_ENTITY_TYPE_UNKNOWN     = -1,
        DAS_WAVEFRONT_ENTITY_TYPE_GROUP       = 0,
        DAS_WAVEFRONT_ENTITY_TYPE_OBJECT      = 1,
        DAS_WAVEFRONT_ENTITY_TYPE_FIRST       = DAS_WAVEFRONT_ENTITY_TYPE_UNKNOWN,
        DAS_WAVEFRONT_ENTITY_TYPE_LAST        = DAS_WAVEFRONT_ENTITY_TYPE_OBJECT
    };


    /// Struct for entity(groups and objects) positions and names that are used in Wavefront OBJ files
    struct WavefrontObjEntityData {
        char name[MAX_WORD_SIZE];
        char *start_ptr;
        uint8_t dc; 
        IndicesDynamic ind_data;
        uint32_t ind_cap;

        VertDynamic vert_data;
        uint16_t fv;                // The number of vertices contained in one face
        uint32_t v_cap;
        uint32_t vt_cap;
        uint32_t vn_cap;
    };


    /// Main entity structure for Wavefront obj entities
    struct WavefrontObjEntity {
        WavefrontObjEntityData data;
        WavefrontObjEntityType type;
    };


    class WavefrontObjParser {
        private:
            char *m_pBuffer;
            size_t m_buf_len;
            bool m_no_cleanup = false;
            std::unordered_map<std::string, WavefrontObjStatement> m_statement_map;
            std::vector<WavefrontObjEntity> m_ents = std::vector<WavefrontObjEntity>();

        private:
            void tokenise();

            // file handling methods
            void reallocCheck(void **pData, uint32_t *p_cap, uint32_t n, uint32_t size, const char *err_msg);
            void preProcessFileData(const std::string &file_name);
            IndexBlock parseFace(const char *face);
            void copyFaceIndices(WavefrontObjEntity &ent, const char **words, uint32_t word_c);
            WavefrontObjSpecType parseStatement(const char **words, uint32_t word_c, uint64_t line);
            void analyseStatement(const char **line_words, uint32_t word_c,  uint64_t lc);
            void extractBlocks(char *beg, uint32_t len, char ***p_words, uint32_t *p_word_c, 
                               uint32_t *p_word_cap, uint64_t max_word_len);
            void newEntity(WavefrontObjEntityType ent_type, const char *name);
            uint32_t findLineCount(char *str, uint32_t len);

        public:
            WavefrontObjParser();
            WavefrontObjParser(bool no_cleanup);
            WavefrontObjParser(const std::string &file_name);
            WavefrontObjParser(const std::string &file_name, bool no_cleanup);
            ~WavefrontObjParser();

            /// Parse the given Wavefront obj file into its entities
            void parseWavefrontObj(const std::string &file_name);

            /// Explicitly clean up all created wavefront obj entities
            void cleanEntities();
            std::vector<WavefrontObjEntity> &getEntities();
    };
}

#endif
