/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: dam.c - deng asset manager header file
/// author: Karl-Mihkel Ott

#define DAS_FILE
#define __DAM_C
#include <dam.h>


namespace libdas {

    /// Find the source file format
    AssetSrcFileFormat dam::findSrcFormat(const std::string &file_name) {
        AssetSrcFileFormat srcf = FORMAT_UNKNOWN;
        size_t flen = file_name.size();

        // Find the last period from the file name
        const char *per = NULL;
        for(int32_t i = (int32_t) flen - 1; i >= 0; i--) {
            if(file_name[i] == '.') {
                per = file_name.c_str() + i;
                break;
            }
        }

        if(!per || per - file_name.c_str() + 1 > flen - 1) {
            fprintf(stderr, "Unknown file format for file %s\n", file_name.c_str());
            exit(1);
        }
        per++;

        // Check the file format for the given file
        if(!strcmp(per, "stl"))
            srcf = STANDARD_TRIANGLE_LANG;
        else if(!strcmp(per, "obj"))
            srcf = WAVEFRONT_OBJ;
        else if(!strcmp(per, "gltf"))
            srcf = GLTF;
        else if(!strcmp(per, "flx"))
            srcf = FLUXBOX;
        else {
            fprintf(stderr, "Unknown file format %s for file %s\n", per, file_name.c_str());
            exit(1);
        }

        return srcf;
    }


    /// Parse user required actions
    dam_Options dam::parseInput(int argc, char *argv[]) {
        // Check if any arguments are even given
        if(argc == 1) {
            std::cout << "DENG asset manager v " << MAJOR << "." << MINOR << "." << REVISION << std::endl;
            std::cout << m_to_help;
            exit(0);
        }
        dam_Options opt = { 0 };
        opt.out_file = argv[argc - 1];
        opt.meta = NULL;

        // DAM has two cases where options are not regarded, when showing helptext and when showing asset information
        if((argc == 2 && !strcmp(argv[1], "-h")) || !strcmp(argv[1], "--help") || argc < 2) {
            std::cout << m_help_text;
            exit(0);
        }
        else if(argc == 2) {
            opt.show_finfo = true;
            return opt;
        }

        // When those two cases are avoided, dam assumes that a new asset needs to be assembled
        // Start iterating through options
        bool wait_input = false;
        bool wait_mode = false;
        uint32_t mflag_c = 0;
        for(int i = 1; i < argc - 1; i++) {
            if(wait_input) {
                opt.input = argv[i];
                wait_input = false;
            }

            else if(wait_mode) {
                // Check if vertices mode is invalid
                if(strcmp(argv[i], "vuu") && strcmp(argv[i], "vun") && 
                   strcmp(argv[i], "vmu") && strcmp(argv[i], "vmn")) {
                    std::cout << "Invalid vertices mode specified" << std::endl;
                    std::cout << "Vertices mode must be any of the following: vuu (vertex unmapped unnormalised), vun (vertex unmapped normalised),"\
                                 "vmu (vertex texture mapped unnormalised) or vmn (vertex texture mapped normalised)" << std::endl;
                    exit(0);
                }

                opt.vmode = *((uint32_t*) argv[i]);
                wait_mode = false;
            }

            // Parse command arguments normally
            else {
                if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--input"))
                    wait_input = true;
                else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--vert"))
                    wait_mode = true;
                else if(!strcmp(argv[i], "-nm") || !strcmp(argv[i], "--no-meta")) {
                    opt.meta = NULL;
                    mflag_c++;
                }
                else if(!strcmp(argv[i], "-m") || !strcmp(argv[i], "--meta"))
                    opt.meta = (char*) 1;

                // Check if meta flag has been called more than once
                if(mflag_c > 1) {
                    std::cerr << "Too many meta flags, there must be only one!" << std::endl;
                    exit(0);
                }
            }
        }


        return opt;
    }


    /// Select appropriate group for usage and prompt the user about possibly multiple groups
    uint32_t dam::selectWObjGroup(uint32_t *nz_objs, uint32_t nz_oc, uint32_t *nz_grps, uint32_t nz_gc, WavefrontObjEntityType &t, std::vector<WavefrontObjEntity> &ents) {
        // In case there is only one entity select that and quit
        if(nz_oc + nz_gc == 1) {
            if(nz_oc == 1) {
                t = DAS_WAVEFRONT_ENTITY_TYPE_OBJECT;
                return *nz_objs;
            } else {
                t = DAS_WAVEFRONT_ENTITY_TYPE_GROUP;
                return *nz_grps;
            }
        }

        // Notify the user about multiple objects and groups
        printf("This file contains ");
        if(nz_oc) printf("%u objects", nz_oc);
        if(nz_gc) printf(" and %u groups", nz_gc);
        printf("\nHowever DAS file can contain data for only single object\n"\
               "Please select the object / group you wish to use\n");


        // List all groups
        printf("Groups\n"\
               "======\n");
        for(uint32_t i = 0; i < nz_gc; i++)
            printf("g%u) %s\n", i + 1, ents[nz_grps[i]].data.name);

        // List all objects
        printf("Objects\n"\
               "=======\n");
        for(uint32_t i = 0; i < nz_oc; i++)
            printf("o%u) %s\n", i + 1, ents[nz_objs[i]].data.name);


        char ibuf[BUF_LEN] = { 0 };

        // Wait for correct user input
        do {
            std::cout << "> ";
            fflush(stdout);
            std::cin.read(ibuf, BUF_LEN);

            uint32_t out = UINT32_MAX;
            if((ibuf[0] == 'o' || ibuf[0] == 'g') && (out = (uint32_t) atoi(ibuf + 1))) {
                if(ibuf[0] == 'o')
                    t = DAS_WAVEFRONT_ENTITY_TYPE_OBJECT;
                else if(ibuf[0] == 'g')
                    t = DAS_WAVEFRONT_ENTITY_TYPE_GROUP;
                if((ibuf[0] == 'o' && out <= nz_oc) || (ibuf[0] == 'g' && out <= nz_gc))
                    return out;
            }

            printf("Invalid options, try again\n");
        } while(true);
    }


    /*
     * Read information about the asset
     * Information is following:
     *  - UUID of the asset
     *  - Date and time of the creation
     *  - Type of vertices
     *  - Total count of unique vertices
     *  - Total count of drawn vertices
     */
    void dam::listAssetData(const std::string &file_name) {
        FILE_HDR fhdr = { 0 };
        INFO_HDR inf_hdr = { 0 };
        META_HDR meta_hdr = { 0 };
        VERT_HDR vert_hdr = { 0 };
        INDX_HDR indx_hdr = { 0 };

        /// Specific vertex header reading
        VertAttribute vpos_hdr = { 0 };
        VertAttribute vtex_hdr = { 0 };
        VertAttribute vnor_hdr = { 0 };

        // Open file for reading
        m_file.open(file_name, std::ios_base::in | std::ios_base::binary);
        msgassert(m_file.is_open(), "Could not open file " + file_name + " for reading");

        readFILE_HDR(&fhdr, file_name);
        readINFO_HDR(&inf_hdr, file_name);

        char date[MAX_STR] = { 0 };
        time_t t = (time_t) inf_hdr.time_st;
        struct tm *tmpt = localtime(&t);
        strftime(date, MAX_STR, "%F %H:%S", tmpt);

        // Read metadata
        bool is_meta = tryToReadMeta(&meta_hdr, file_name);

        // Read the initial part of the VERT_HDR
        readVERT_HDR(&vert_hdr, file_name);

        // Read position vertex header and skip its vertices
        readVertAttr(&vpos_hdr, DAS_VPOS_HEADER_SIG, file_name, sizeof(PosData));
        m_offset += vpos_hdr.vert_c * sizeof(PosData);
        m_file.seekg(static_cast<std::fstream::pos_type>(m_offset));

        // Increment reading offset for file buffer
        switch(inf_hdr.asset_type) {
        case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
            readVertAttr(&vtex_hdr, DAS_VTEX_HEADER_SIG, file_name, sizeof(TextureData));
            m_offset += vtex_hdr.vert_c * sizeof(TextureData);
            m_file.seekg(static_cast<std::fstream::pos_type>(m_offset));
            break;

        case DAS_ASSET_MODE_3D_UNMAPPED:
            readVertAttr(&vnor_hdr, DAS_VNOR_HEADER_SIG, file_name, sizeof(NormalData));
            m_offset += vnor_hdr.vert_c * sizeof(NormalData);
            m_file.seekg(static_cast<std::fstream::pos_type>(m_offset));
            break;

        case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
            readVertAttr(&vtex_hdr, DAS_VTEX_HEADER_SIG, file_name, sizeof(TextureData));
            m_offset += vtex_hdr.vert_c * sizeof(TextureData);
            m_file.seekg(static_cast<std::fstream::pos_type>(m_offset));
            
            readVertAttr(&vnor_hdr, DAS_VNOR_HEADER_SIG, file_name, sizeof(NormalData));
            m_offset += vnor_hdr.vert_c * sizeof(NormalData);
            m_file.seekg(static_cast<std::fstream::pos_type>(m_offset));
            break;

        case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
            readVertAttr(&vtex_hdr, DAS_VTEX_HEADER_SIG, file_name, sizeof(TextureData));
            m_offset += vtex_hdr.vert_c * sizeof(TextureData);
            m_file.seekg(static_cast<std::fstream::pos_type>(m_offset));
            break;

        default:
            break;
        }

        readINDX_HDR(static_cast<AssetMode>(inf_hdr.asset_type), &indx_hdr, file_name, false);

        std::cout << "File: " << file_name << std::endl;
        std::cout << "UUID: " << inf_hdr.uuid << std::endl;
        std::cout << "Date and time of creation: " << date << "(UTC)" << std::endl;

        switch(inf_hdr.asset_type) {
        case DAS_ASSET_MODE_3D_UNMAPPED:
            std::cout << "Asset type: 3D unmapped vertices" << std::endl;
            std::cout << "Unique position vertices count: " << vpos_hdr.vert_c << std::endl;
            std::cout << "Unique vertex normals count: " << vnor_hdr.vert_c << std::endl;
            break;

        case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
            std::cout << "Asset type: 3D unnormalised unmapped vertices" << std::endl;
            std::cout << "Unique position vertices count: " << vpos_hdr.vert_c << std::endl;
            break;

        case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
            std::cout << "Asset type: 3D textured vertices" << std::endl;
            std::cout << "Unique position vertices count: " << vpos_hdr.vert_c << std::endl;
            std::cout << "Unique texture vertices count: " << vtex_hdr.vert_c << std::endl;
            std::cout << "Unique vertex normals count: " << vnor_hdr.vert_c << std::endl;
            break;
        
        case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
            std::cout << "Asset type: 3D unnormalised textured vertices" << std::endl;
            std::cout << "Unique position vertices count: " << vpos_hdr.vert_c << std::endl;
            std::cout << "Unique texture vertices count: " << vtex_hdr.vert_c << std::endl;
            break;

        default:
            break;
        }

        std::cout << "Total indices count: " << indx_hdr.ind_c << std::endl;

        // Check if metadata is available and display it
        if(is_meta) {
            std::cout << "Metadata: " << meta_hdr.data << std::endl;
            free(meta_hdr.data);
        }

        m_file.close();
    }


    void dam::mkAssetFromWObj(Asset &asset, std::vector<WavefrontObjEntity> &ents) {
        if(ents.empty()) return;
        uint32_t *nz_objs = (uint32_t*) malloc(ents.size() * sizeof(uint32_t));
        uint32_t *nz_grps = (uint32_t*) malloc(ents.size() * sizeof(uint32_t));
        uint32_t nz_oc = 0;
        uint32_t nz_gc = 0;

        // Find all non-zero objects and grous
        for(uint32_t i = 0; i < static_cast<uint32_t>(ents.size()); i++) {
            // Check if no data is present in obj entity and thus ignore this entity
            // NOTE: no data is present if no position vertices of any kind are available. 
            if(!ents[i].data.vert_data.v3d.mul.pn && !ents[i].data.vert_data.v2d.mul.pn)
                continue;

            switch(ents[i].type) {
                case DAS_WAVEFRONT_ENTITY_TYPE_OBJECT:
                    nz_objs[nz_oc] = i;
                    nz_oc++;
                    break;

                case DAS_WAVEFRONT_ENTITY_TYPE_GROUP:
                    nz_grps[nz_gc] = i;
                    nz_gc++;
                    break;

                default:
                    break;
            }
        }

        /// LOGGING
        for(uint32_t i = 0; i < nz_oc; i++)
            printf("Object with vertices: %s\n", ents[nz_objs[i]].data.name);
        for(uint32_t i = 0; i < nz_gc; i++)
            printf("Object with vertices: %s\n", ents[nz_grps[i]].data.name);

        // In case there are no objects or groups with appropriate vertices, quit
        if(!(nz_oc + nz_gc)) {
            free(nz_objs);
            free(nz_grps);
            fprintf(stderr, "Invalid wavefront OBJ file, no vertices available");
            exit(-1);
        }

        // Check how many groups and objects were found and thus decide whose data to use
        WavefrontObjEntityType etype = DAS_WAVEFRONT_ENTITY_TYPE_UNKNOWN;
        uint32_t eind = selectWObjGroup(nz_objs, nz_oc, nz_grps, nz_gc, etype, ents);
        asset.vertices = ents[eind].data.vert_data;
        asset.indices = ents[eind].data.ind_data;

        // For test purposes assume that 3D vmn mode is used
        asset.asset_mode = DAS_ASSET_MODE_3D_TEXTURE_MAPPED;

        id_generator idg;
        asset.uuid = idg.getUUID();

        if(ents[eind].data.fv == 4) quadTriangulate(asset);

        // Clean all other entities data
        for(uint32_t i = 0; i < static_cast<uint32_t>(ents.size()); i++) {
            if(i != eind) {
                free(ents[i].data.vert_data.v3d.mul.pos);
                free(ents[i].data.vert_data.v3d.mul.tex);
                free(ents[i].data.vert_data.v3d.mul.norm);

                free(ents[i].data.ind_data.pos);
                free(ents[i].data.ind_data.tex);
                free(ents[i].data.ind_data.norm);
            }
        }

        free(nz_objs);
        free(nz_grps);
    }


    void dam::quadTriangulate(Asset &asset) {
        std::cout << "Triangulating quad faces" << std::endl;
        IndicesDynamic inds = { 0 };
        inds.n = asset.indices.n / 2 * 3;
        inds.pos = (uint32_t*) malloc(inds.n * sizeof(uint32_t));
        inds.tex = (uint32_t*) malloc(inds.n * sizeof(uint32_t));
        inds.norm = (uint32_t*) malloc(inds.n * sizeof(uint32_t));

        // For each quad in array, create triangle out of it
        for(uint64_t i = 0; i < asset.indices.n / 4; i++) {
            uint32_t *q_pos = asset.indices.pos + i * 4;
            uint32_t *q_tex = asset.indices.tex + i * 4;
            uint32_t *q_norm = asset.indices.norm + i * 4;

            uint32_t *t_pos = inds.pos + i * 6;
            uint32_t *t_tex = inds.tex + i * 6;
            uint32_t *t_norm = inds.norm + i * 6;

            t_pos[0] = q_pos[0], t_pos[1] = q_pos[1], t_pos[2] = q_pos[2], t_pos[3] = q_pos[2], t_pos[4] = q_pos[3], t_pos[5] = q_pos[0];
            t_tex[0] = q_tex[0], t_tex[1] = q_tex[1], t_tex[2] = q_tex[2], t_tex[3] = q_tex[2], t_tex[4] = q_tex[3], t_tex[5] = q_tex[0];
            t_norm[0] = q_norm[0], t_norm[1] = q_norm[1], t_norm[2] = q_norm[2], t_norm[3] = q_norm[2], t_norm[4] = q_norm[3], t_norm[5] = q_norm[0];
        }

        // Free initial quad memory areas
        free(asset.indices.pos);
        free(asset.indices.tex);
        free(asset.indices.norm);

        asset.indices = inds;
    }
}



int main(int argc, char *argv[]) {
    // Parse user input into input actions
    libdas::dam dam;
    libdas::dam_Options opt = dam.parseInput(argc, argv);

    // Check if info should be listed or assembly performed
    if(opt.show_finfo) dam.listAssetData(opt.out_file);
    else {
        char meta[META_LEN] = { 0 };
        if(opt.meta) {
            fgets(meta, META_LEN, stdin);
            opt.meta = meta;
        }

        // Check the asset file format
        libdas::AssetSrcFileFormat srcf = dam.findSrcFormat(opt.input);
        Asset asset = {};
        switch(srcf) {
            case libdas::WAVEFRONT_OBJ:
                {
                    libdas::WavefrontObjParser parser(opt.input, NO_CLEANUP);
                    std::vector<libdas::WavefrontObjEntity> &ents = parser.getEntities();
                    dam.mkAssetFromWObj(asset, ents);
                }
                break;

            case libdas::STANDARD_TRIANGLE_LANG:
                perror("STL files are not yet supported in DAM :(\n");
                exit(1);
                break;

            case libdas::GLTF:
                perror("GLTF files are not yet supported in DAM :(\n");
                exit(1);
                break;

            case libdas::FLUXBOX:
                perror("FLX file are not yet supported in DAM :(\n");
                exit(1);
                break;

            default:
                break;
        }

        libdas::AssetWriter writer;

        // Check if the file extension exists
        std::string out_file = opt.out_file;
        size_t pos = out_file.rfind(".");
        if(pos == static_cast<size_t>(-1) || out_file.substr(pos) != ".das")
            out_file += ".das";
        writer.createAssetFile(asset, out_file, opt.meta);
        libdas::das_loader::cleanupAsset(asset);
    }
    return 0;
}
