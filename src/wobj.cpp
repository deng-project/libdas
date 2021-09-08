/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: wobj.cpp - wavefront obj parser source file
/// author: Karl-Mihkel Ott


#define DAS_FILE
#define __WOBJ_CPP
#include <wobj.h>

namespace libdas {

    WavefrontObjParser::WavefrontObjParser() {
        tokenise();
    }


    WavefrontObjParser::WavefrontObjParser(bool no_cleanup) : m_no_cleanup(no_cleanup) {
        tokenise();
    }


    WavefrontObjParser::WavefrontObjParser(const std::string &file_name) {
        tokenise();
        parseWavefrontObj(file_name);
    }


    WavefrontObjParser::WavefrontObjParser(const std::string &file_name, bool no_cleanup) : m_no_cleanup(no_cleanup) {
        tokenise();
        parseWavefrontObj(file_name);
    }


    WavefrontObjParser::~WavefrontObjParser() {
        if(!m_no_cleanup)
            cleanEntities();
    }


    /// Clean all loaded wavefront obj entities
    void WavefrontObjParser::cleanEntities() {
        for(size_t i = 0; i < m_ents.size(); i++) {
            // Check the dimention count of the file and free data accordingly
            switch(m_ents[i].data.dc) {
                case 2:
                    free(m_ents[i].data.vert_data.v2d.mul.pos);
                    free(m_ents[i].data.vert_data.v2d.mul.tex);
                    break;

                case 3:
                    free(m_ents[i].data.vert_data.v3d.mul.pos);
                    free(m_ents[i].data.vert_data.v3d.mul.tex);
                    free(m_ents[i].data.vert_data.v3d.mul.norm);
                    break;
                default:
                    break;
            }

            free(m_ents[i].data.ind_data.pos);
            free(m_ents[i].data.ind_data.tex);
            free(m_ents[i].data.ind_data.norm);
        }

        m_ents.clear();
    }


    /// Tokenise all statements
    void WavefrontObjParser::tokenise() {
        m_statement_map.reserve(STATEMENT_C * 2);
        m_statement_map["o"] = { "o", 0, 1, DAS_WAVEFRONT_OBJ_SPEC_TYPE_OBJ_DECL };
        m_statement_map["g"] = { "g", 0, 1, DAS_WAVEFRONT_OBJ_SPEC_TYPE_GROUP_DECL };
        m_statement_map["v"] = { "v", 3, 4, DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_DECL };
        m_statement_map["vt"] = { "vt", 1, 3, DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_TEX_DECL };
        m_statement_map["vn"] = { "vn", 3, 3, DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_NORM_DECL };
        m_statement_map["vp"] = { "vp", 1, 3, DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_PARAM_DECL };
        m_statement_map["f"] = { "f", 3, 4, DAS_WAVEFRONT_OBJ_SPEC_TYPE_FACE_DECL };
        m_statement_map["l"] = { "l", 1, UINT32_MAX, DAS_WAVEFRONT_OBJ_SPEC_TYPE_POL_LINE_DECL };
        m_statement_map["mtllib"] = { "mtllib", 1, 1, DAS_WAVEFRONT_OBJ_SPEC_TYPE_MTL_INCL };
        m_statement_map["usemtl"] = { "usemtl", 1, 1, DAS_WAVEFRONT_OBJ_SPEC_TYPE_MTL_USE };
        m_statement_map["s"] = { "s", 1, 1, DAS_WAVEFRONT_OBJ_SPEC_TYPE_SHADING_SPEC };
    }


    /// Check if memory reallocations need to be done
    void WavefrontObjParser::reallocCheck(void **pData, uint32_t *p_cap, uint32_t n, uint32_t size, const char *err_msg) {
        // Reallocate if the capacity is smaller than the required amount of elements
        if(n * size >= (*p_cap)) {
            uint32_t old_cap = (*p_cap);
            (*p_cap) = n * 2;
            (*p_cap) = (*p_cap) < (old_cap << 1) ? old_cap << 1 : (*p_cap);
            void *tmp = realloc((*pData), (*p_cap) * size);
            if(!tmp) MEM_ERR(err_msg);
            (*pData) = tmp;
        }
    }


    /// Read file to buffer and remove all comments
    void WavefrontObjParser::preProcessFileData(const std::string &file_name) {
        // Open specified file for reading
        std::ifstream fs = std::ifstream(file_name, std::ios::binary);
        if(!fs.is_open())
            std::runtime_error("Could not open file " + file_name + " for reading");

        fs.seekg(0, std::ios_base::end);
        m_buf_len = fs.tellg();
        fs.seekg(0, std::ios_base::beg);
        m_pBuffer = (char*) malloc(m_buf_len + 1);
        fs.read(m_pBuffer, m_buf_len);

        // Preprocessor stage 1: elliminate all comments
        char *beg, *lend = 0;
        while((beg = strchr(m_pBuffer, (int) '#'))) {
            lend = strchr(beg, (int) 0x0a);
            lend = !lend ? m_pBuffer + m_buf_len : lend;
            memset(beg, (int) 0x20, lend - beg);
        }

        fs.close();
    }


    /// Parse a single face block
    IndexBlock WavefrontObjParser::parseFace(const char *face) {
        // UINT32_MAX basically means that this index is going to be ignored
        IndexBlock ind_block = { UINT32_MAX };
        uint32_t face_len = strlen(face);
        
        // Separator and number buffer declaration
        char buf[32] = { 0 };
        const char *sep[2] = { 0 };
        
        // Find all separators
        for(uint32_t i = 1, j = 0; i < face_len && j < 3; i++) {
            if(face[i] == '/') {
                sep[j] = face + i;
                j++;
            }
        }

        // Check if the first separator exists and if it does then copy value till separator
        if(sep[0]) {
            strncpy(buf, face, sep[0] - face);
            ind_block.pos = (uint32_t) atoi(buf);
            memset(buf, 0, 32);
        }
        else {
            ind_block.pos = (uint32_t) atoi(face);
            return ind_block;
        }
        
        // Check if the texture index exists
        const char *end = !sep[1] ? face + face_len : sep[1];
        if(end - sep[0] > 1) {
            strncpy(buf, sep[0] + 1, end - sep[0]);
            ind_block.tex = (uint32_t) atoi(buf);
            memset(buf, 0, 32);
        }

        // Check if second separator exists and has space between the third one
        end = face + face_len;
        if(sep[1] && end - sep[1] > 1) {
            strncpy(buf, sep[1] + 1, end - sep[1]);
            ind_block.norm = (uint32_t) atoi(buf);
            memset(buf, 0, 32);
        }

        return ind_block;
    } 


    /// Copy all face indices to entity structure
    void WavefrontObjParser::copyFaceIndices(WavefrontObjEntity &ent, const char **words, uint32_t word_c) {
        ent.data.fv = word_c - 1;
        if(ent.data.fv == 0) {
            printf("zero\n");
        }

        // Check if indices need reallocation
        uint32_t cap = ent.data.ind_cap;
        reallocCheck((void**) &ent.data.ind_data.pos, &cap, ent.data.ind_data.n + 1, sizeof(uint32_t), 
                     "Failed to allocate memory for position indices");

        cap = ent.data.ind_cap;
        reallocCheck((void**) &ent.data.ind_data.tex, &cap, ent.data.ind_data.n + 1, sizeof(uint32_t), 
                     "Failed to allocate memory for texture indices");

        cap = ent.data.ind_cap;
        reallocCheck((void**) &ent.data.ind_data.norm, &cap, ent.data.ind_data.n + 1, sizeof(uint32_t), 
                     "Failed to allocate memory vertex normal indices");

        ent.data.ind_cap = cap;

        // For each face block parse it and save its data
        for(uint32_t i = 1; i < word_c; i++) {
            IndexBlock block = parseFace(words[i]);
            
            // Check if parsed vertices index is UINT32_MAX
            ent.data.ind_data.n++;
            ent.data.ind_data.pos[ent.data.ind_data.n - 1] = block.pos - 1;
            ent.data.ind_data.tex[ent.data.ind_data.n - 1] = block.tex - 1;
            ent.data.ind_data.norm[ent.data.ind_data.n - 1] = block.norm - 1;
        }
    }


    /// Parse one line statement
    WavefrontObjSpecType WavefrontObjParser::parseStatement(const char **words, uint32_t word_c, uint64_t line) {
        // If no words are present then return
        if(!word_c) return DAS_WAVEFRONT_OBJ_SPEC_TYPE_NONE;

        // Check if statement is valid and not NULL otherwise throw syntax error
        if(m_statement_map.find(words[0]) == m_statement_map.end()) {
            char buf[MAX_WORD_SIZE] = { 0 }; 
            sprintf(buf, "invalid keyword \"%s\"\n", words[0]);
            WAVEFRONT_SYNTAX_ERROR(line, buf);
        }

        WavefrontObjStatement &statement =  m_statement_map.at(words[0]);

        // Check if statement has enough arguments
        if((uint32_t) (word_c - 1) < statement.min_obj_c)
            NOT_ENOUGH_ARGS(line);

        // Check if statement has too many arguments
        else if((uint32_t) (word_c - 1) > statement.max_obj_c)
            TOO_MANY_ARGS(line);

        return statement.spec_type;
    }

    /// Analyse the given line statement and perform actions accordingly
    void WavefrontObjParser::analyseStatement(const char **swords, uint32_t word_c,  uint64_t lc) { 
        // Parse the line statement
        WavefrontObjSpecType obj_spec_type = parseStatement(swords, word_c, lc);

        // Check for new action 
        switch(obj_spec_type) {
        case DAS_WAVEFRONT_OBJ_SPEC_TYPE_OBJ_DECL: {
            // Set a name variable if it exists
            const char *name = NULL;
            if(word_c >= 2) 
                name = swords[1];
            
            // Create a new object entity
            newEntity(DAS_WAVEFRONT_ENTITY_TYPE_OBJECT, name);
            break;
        }

        case DAS_WAVEFRONT_OBJ_SPEC_TYPE_GROUP_DECL: {
            // Set a name variable if it exists
            const char *name = NULL;
            if(word_c >= 2) 
                name = swords[1];
            
            // Create a new group entity
            newEntity(DAS_WAVEFRONT_ENTITY_TYPE_GROUP, name);
            break;
        }

        case DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_DECL:
            // Check if vertex reallocation is needed
            reallocCheck((void**) & m_ents.back().data.vert_data.v3d.mul.pos,
                         &m_ents.back().data.v_cap, m_ents.back().data.vert_data.v3d.mul.pn + 1, 
                         sizeof(PosData), "Failed to reallocate memory for position vertices");
                
            // Set the entity vertices
            m_ents.back().data.vert_data.v3d.mul.pos[m_ents.back().data.vert_data.v3d.mul.pn].vert_x = static_cast<float>(std::atof(swords[1]));
            m_ents.back().data.vert_data.v3d.mul.pos[m_ents.back().data.vert_data.v3d.mul.pn].vert_y = static_cast<float>(std::atof(swords[2]));
            m_ents.back().data.vert_data.v3d.mul.pos[m_ents.back().data.vert_data.v3d.mul.pn].vert_z = static_cast<float>(std::atof(swords[3]));
            m_ents.back().data.vert_data.v3d.mul.pn++;
            break;

        case DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_TEX_DECL:
            // Check if vertex reallocation is needed
            reallocCheck((void**) &m_ents.back().data.vert_data.v3d.mul.tex,
                         &m_ents.back().data.vt_cap, m_ents.back().data.vert_data.v3d.mul.tn + 1, 
                         sizeof(TextureData), "Failed to reallocate memory for position vertices");

            // Set the entity texture vertices
            m_ents.back().data.vert_data.v3d.mul.tex[m_ents.back().data.vert_data.v3d.mul.tn].tex_x = static_cast<float>(std::atof(swords[1]));
            m_ents.back().data.vert_data.v3d.mul.tex[m_ents.back().data.vert_data.v3d.mul.tn].tex_y = static_cast<float>(std::atof(swords[2]));
            m_ents.back().data.vert_data.v3d.mul.tn++;
            break;

        case DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_NORM_DECL:
            // Check if vertex normal reallocation is needed
            reallocCheck((void**) &m_ents.back().data.vert_data.v3d.mul.norm,
                         &m_ents.back().data.vn_cap, m_ents.back().data.vert_data.v3d.mul.nn + 1, 
                         sizeof(NormalData), "Failed to reallocate memory for position vertices");
                
            // Set the entity vertex normals
            m_ents.back().data.vert_data.v3d.mul.norm[m_ents.back().data.vert_data.v3d.mul.nn].nor_x = static_cast<float>(std::atof(swords[1]));
            m_ents.back().data.vert_data.v3d.mul.norm[m_ents.back().data.vert_data.v3d.mul.nn].nor_y = static_cast<float>(std::atof(swords[2]));
            m_ents.back().data.vert_data.v3d.mul.norm[m_ents.back().data.vert_data.v3d.mul.nn].nor_z = static_cast<float>(std::atof(swords[3]));
            m_ents.back().data.vert_data.v3d.mul.nn++;
            break;

        case DAS_WAVEFRONT_OBJ_SPEC_TYPE_FACE_DECL:
            copyFaceIndices(m_ents.back(), swords, word_c);
            break;

        default:
            break;
        }
    }


    /// Find all text blocks used between beg and beg + len
    void WavefrontObjParser::extractBlocks(char *beg, uint32_t len, char ***p_words, uint32_t *p_word_c, 
                              uint32_t *p_word_cap, uint64_t max_word_len) {
        char *cur = beg;
        while(cur < beg + len) {
            // Skip all whitespaces, tabs and newlines
            while(cur < beg + len && (*cur == 0x20 || *cur == 0x09 || *cur == 0x0A || *cur == 0x0D))
                cur++;

            // Quit the loop if the maximum reading length is reached
            if(cur >= beg + len) break;
            
            // Check if block needs to be reallocated
            uint32_t old_cap = *p_word_cap;
            reallocCheck((void**) p_words, p_word_cap, (*p_word_c) + 1, sizeof(char*), 
                         "Could not reallocate memory for wavefront obj statements");

            // For each allocated pointer location allocate memory for each character
            for(uint32_t i = old_cap; i < (*p_word_cap); i++)
                (*p_words)[i] = (char*) calloc(max_word_len, sizeof(char));

            // For each character until whitespace or newline set the block's value
            for(uint32_t i = 0; cur < beg + len && *cur != 0x20 && *cur != 0x09 && *cur != 0x0A && *cur != 0x0D; i++, cur++) {
                // Check if the block size is larger than allowed
                if(i >= max_word_len) {
                    uint32_t lc = findLineCount(m_pBuffer, cur - m_pBuffer);
                    TOO_LONG_WORD(lc);
                }
                (*p_words)[(*p_word_c)][i] = *cur;
            }
            
            (*p_word_c)++;
            cur++;
        }
    }


    /// Create a new object or group instance for entity type
    void WavefrontObjParser::newEntity(WavefrontObjEntityType ent_type ,const char *name) {
        m_ents.push_back(WavefrontObjEntity());

        // Copy the name if available
        if(name) strcpy(m_ents.back().data.name, name);
        m_ents.back().type = ent_type;

        // Allocate initial amount of memory for indices 
        m_ents.back().data.ind_cap = DEFAULT_MEM_CAP;
        m_ents.back().data.ind_data.pos = (uint32_t*) calloc(DEFAULT_MEM_CAP, sizeof(uint32_t));
        m_ents.back().data.ind_data.tex = (uint32_t*) calloc(DEFAULT_MEM_CAP, sizeof(uint32_t));
        m_ents.back().data.ind_data.norm = (uint32_t*) calloc(DEFAULT_MEM_CAP, sizeof(uint32_t));

        // Allocate initial amount of memory for vertices 
        m_ents.back().data.v_cap = DEFAULT_MEM_CAP;
        m_ents.back().data.vert_data.v3d.mul.pos = (PosData*) calloc (
            DEFAULT_MEM_CAP, sizeof(PosData));

        m_ents.back().data.vt_cap = DEFAULT_MEM_CAP;
        m_ents.back().data.vert_data.v3d.mul.tex = (TextureData*) calloc (
            DEFAULT_MEM_CAP, sizeof(TextureData));

        m_ents.back().data.vn_cap = DEFAULT_MEM_CAP;
        m_ents.back().data.vert_data.v3d.mul.norm = (NormalData*) calloc (
            DEFAULT_MEM_CAP, sizeof(NormalData));
    }


    /// Find the current char line count from the beginning of the string
    uint32_t WavefrontObjParser::findLineCount(char *str, uint32_t cbyte) {
        uint32_t lc = 1;

        // For each character check if it is newline
        for(size_t i = 0; i < cbyte; i++)
            if(str[i] == 0x0A) lc++;

        return lc;
    }


    /// Parse the given Wavefront obj file into its entities
    void WavefrontObjParser::parseWavefrontObj(const std::string &file_name) {
        // Load object into memory
        m_ents.reserve(DEFAULT_ENTITY_CAP);

        // Add some extra complexity by removing all comments
        preProcessFileData(file_name);

        // Set up the initial global entity
        newEntity(DAS_WAVEFRONT_ENTITY_TYPE_OBJECT, "Default");

        // Allocate memory for line words
        uint32_t word_cap = 8;
        uint32_t word_c = 0;

        // Allocate memory for line words
        char **swords = (char**) calloc(word_cap, sizeof(char*));
        for(uint32_t i = 0; i < word_cap; i++) 
            swords[i] = (char*) calloc(MAX_WORD_SIZE, sizeof(char));

        // Read the buffer line by line
        char *cur = m_pBuffer;
        char *end = NULL;
        uint64_t lc = 1;
        while(cur < m_pBuffer + m_buf_len) {
            // Find the newline and if it does not exist set the end as eof
            end = strchr(cur, 0x0a);
            end = !end ? m_pBuffer + m_buf_len : end;

            // Find all the statements in the line
            extractBlocks(cur, end - cur, &swords, &word_c, &word_cap, MAX_WORD_SIZE);
            
            // If no blocks were found skip the iteration
            if(!word_c) {
                lc++;
                cur = end + 1;
                continue;
            }

            // Analyse the retrieved line
            analyseStatement(const_cast<const char**>(swords), word_c, lc);
            cur = end + 1;

            // Reset all the words that were read
            for(uint32_t i = 0; i < word_c; i++)
                memset(swords[i], 0, MAX_WORD_SIZE * sizeof(char));
            
            lc++;
            word_c = 0;
        }

        // Clean all the memory that was for line words
        for(uint32_t i = 0; i < word_cap; i++)
            free(swords[i]);
        free(swords);
        free(m_pBuffer);
    }


    std::vector<WavefrontObjEntity> &WavefrontObjParser::getEntities() {
        return m_ents;
    }
}
