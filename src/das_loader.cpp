/// libdas: DENG m_asset handling management library
/// licence: Apache, see LICENCE file
/// file: das_loader.cpp - das file format reader implementation file
/// author: Karl-Mihkel Ott

#define DAS_FILE
#define __DAS_LOADER_CPP
#include <das_loader.h>


namespace libdas {

    das_loader::das_loader() : m_asset{}, m_file(0), m_file_size(0), m_offset(0) {}


    das_loader::das_loader(AssetMode dst_mode, const ColorData &color, char **meta, uuid_t *tex_uuid, const std::string &file_name) {
        loadAsset(dst_mode, color, meta, tex_uuid, file_name);
    }


    /// Read file header containing the file signature and padding
    void das_loader::readFILE_HDR(FILE_HDR *fhdr, const std::string &file_name) {
        m_file.read(reinterpret_cast<char*>(fhdr), sizeof(FILE_HDR));
        m_offset += sizeof(FILE_HDR);
        msgassert(fhdr->hdr_sig == DAS_FILE_HEADER_SIG, "Could not verify file signature in file " + file_name);
    }


    /// Read m_asset information from INFO_HDR
    void das_loader::readINFO_HDR(INFO_HDR *ifhdr, const std::string &file_name) {
        m_file.read(reinterpret_cast<char*>(ifhdr), sizeof(INFO_HDR));
        m_offset += sizeof(INFO_HDR);
        msgassert(ifhdr->hdr_sig == DAS_INFO_HEADER_SIG, "Could not verify INFO_HDR signature in file " + file_name);
    }


    /// Attempt to read metadata and return true if possible, if not return false
    bool das_loader::tryToReadMeta(META_HDR *meta, const std::string &file_name) {
        const size_t rsize = 16;
        m_file.read(reinterpret_cast<char*>(meta), rsize);
        m_offset += rsize;
        
        if(meta->hdr_sig != DAS_META_HEADER_SIG) {
            m_offset -= rsize;
            m_file.seekg(m_offset, std::ios_base::beg);
            return false;
        }

        msgassert(meta->hdr_size != meta->data_size + rsize, "Could not get correct metadata header size in file " + file_name);
        msgassert(meta->data_size + m_offset <= m_file_size, "Metadata size exceeds file size in file " + file_name);

        // Allocate memory for metadata
        meta->data = (char*) malloc((meta->data_size + 1) * sizeof(char));
        
        // Copy metadata to its appropriate memory area
        // BUG: Whenever an error condition is triggered in dataRead() subroutine, 
        //      you can expect segmentation fault from trying to print NULL
        m_file.read(meta->data, meta->data_size);
        m_offset += meta->data_size;
        meta->data[meta->data_size - 1] = 0;

        return true;
    }


    /// Verify that all custom headers are skipped from reading
    void das_loader::skipMetaHeaders(const std::string &file_name) {
        META_HDR mhdr = {};

        // Because sizeof(das_META_HDR) - sizeof(char*) == 16
        const size_t rsize = 16;
        m_file.read(reinterpret_cast<char*>(&mhdr), rsize);
        m_offset += rsize;

        // Could not verify the signature, exit the subroutine
        if(mhdr.hdr_sig != DAS_META_HEADER_SIG) {
            m_offset -= rsize;
            m_file.seekg(m_offset, std::ios_base::beg);
            return;
        }

        msgassert(mhdr.hdr_size == mhdr.data_size + rsize, "Could not get correct metadata header size in file " + file_name);
        msgassert(mhdr.data_size + m_offset <= m_file_size , "Metadata size exceeds file size in file " + file_name);

        m_offset += mhdr.data_size;
        m_file.seekg(m_offset, std::ios_base::beg);
    }


    /// Read m_asset information from VERT_HDR
    void das_loader::readVERT_HDR(VERT_HDR *vhdr, const std::string &file_name) {
        const size_t hsize = 12;
        m_file.read(reinterpret_cast<char*>(vhdr), hsize);
        m_offset += hsize;
        msgassert(vhdr->hdr_sig == DAS_VERT_HEADER_SIG, "Could not verify signature for VERT_HDR in file " + file_name);
    }


    /// Read information about one vertex element header type
    void das_loader::readVertAttr(VertAttribute *ahdr, uint64_t exsig, const std::string &file_name, uint64_t data_size) {
        const size_t hsize = 17;
        std::cout << "File ptr: " << m_file.tellg() << std::endl;
        m_file.read(reinterpret_cast<char*>(ahdr), hsize);
        m_offset += hsize;
        msgassert(ahdr->hdr_sig == exsig, "Could not verify signature for VERT_HDR attribute in file " + file_name);

        if(!data_size) {
            msgassert(ahdr->hdr_size == ahdr->vert_c * ahdr->esize * sizeof(float) + hsize, 
                      "Could not get correct vertex attribute header size in file " + file_name);
        } 
    }


    /// Read m_asset information from INDX_HDR
    void das_loader::readINDX_HDR(AssetMode mode, INDX_HDR *ihdr, const std::string &file_name, bool read_indices) {
        const size_t hsize = 16;
        m_file.read(reinterpret_cast<char*>(ihdr), hsize);
        msgassert(ihdr->hdr_sig == DAS_INDX_HEADER_SIG, "Could not verify signature for INDX_HDR in file " + file_name);

        if(read_indices) {
            uint32_t bfc = 0;
            bool is_tex = false;
            bool is_norm = false;
            switch(mode) {
                case DAS_ASSET_MODE_2D_UNMAPPED:
                    bfc = 1;
                    break;

                case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
                    bfc = 2;
                    is_tex = true;
                    break;

                case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
                    bfc = 1;
                    break;

                case DAS_ASSET_MODE_3D_UNMAPPED:
                    bfc = 2;
                    is_norm = true;
                    break;

                case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
                    bfc = 2;
                    is_tex = true;
                    break;

                case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
                    bfc = 3;
                    is_tex = true;
                    is_norm = true;
                    break;

                case DAS_ASSET_MODE_UNDEFINED:
                    msgassert(NULL, "Invalid m_asset mode specified on reading INDX_HDR in file " + file_name);
                    break;
            }

            msgassert(ihdr->hdr_size == ihdr->ind_c * bfc * sizeof(uint32_t) + hsize, "Could not get correct INDX_HDR size in file " + file_name);

            const size_t dsize = ihdr->ind_c * sizeof(uint32_t);

            // Allocate memory and read position indices
            ihdr->pinds = (uint32_t*) malloc(dsize);
            m_file.read(reinterpret_cast<char*>(ihdr->pinds), dsize);

            // If needed read texture indices
            if(is_tex) {
                ihdr->tinds = (uint32_t*) malloc(dsize);
                m_file.read(reinterpret_cast<char*>(ihdr->tinds), dsize);
            }

            // If needed read vertex normal indices
            if(is_norm) {
                ihdr->ninds = (uint32_t*) malloc(dsize);
                m_file.read(reinterpret_cast<char*>(ihdr->ninds), dsize);
            }
        }
    }


    /// Load m_asset into given m_asset file
    void das_loader::loadAsset(AssetMode dst_mode, const ColorData &color, char **meta, uuid_t *tex_uuid, const std::string &file_name) {
        memset(&m_asset, 0, sizeof(Asset));
        m_asset.tex_uuid = tex_uuid;
        strcpy(m_asset.src, file_name.c_str());
        m_asset.diffuse = DAS_DEFAULT_DIFFUSE_COLOR;
        m_asset.ambient = DAS_DEFAULT_AMBIENT_COLOR;
        m_asset.specular = DAS_DEFAULT_SPECULAR_COLOR;
        m_asset.is_shown = true;
        m_asset.ignore_transform = false;
        m_asset.force_unmap = false;

        // Open file stream for reading
        m_file.open(file_name, std::ios_base::in | std::ios_base::binary);
        msgassert(m_file.is_open(), "Could not open file " + file_name);

        FILE_HDR file_hdr = {};
        INFO_HDR info_hdr = {};
        META_HDR meta_hdr = {};
        VERT_HDR vert_hdr = {};
        INDX_HDR indx_hdr = {};

        readFILE_HDR(&file_hdr, file_name);
        readINFO_HDR(&info_hdr, file_name);

        m_asset.asset_mode = static_cast<AssetMode>(info_hdr.asset_type);
        strcpy(m_asset.uuid.bytes, info_hdr.uuid);

        // Check if metadata should be read or ignored
        if(meta) {
            tryToReadMeta(&meta_hdr, file_name);
            *meta = meta_hdr.data;
        }
        else skipMetaHeaders(file_name);

        // Read vertices header
        readVERT_HDR(&vert_hdr, file_name);

        VertAttribute attr = {};

        // Read vertex attributes starting from position elements, texture elements and finishing on normal elements
        readVertAttr(&attr, DAS_VPOS_HEADER_SIG, file_name, sizeof(PosData));

        // Check vertex position data element count
        switch(attr.esize) {
            case 2:
                m_asset.vertices.v2d.mul.pn = attr.vert_c;
                m_asset.vertices.v2d.mul.pos = (PosData2D*) malloc(attr.vert_c * sizeof(PosData2D));
                m_file.read(reinterpret_cast<char*>(m_asset.vertices.v2d.mul.pos), m_asset.vertices.v2d.mul.pn * sizeof(PosData2D));
                break;

            case 3:
                m_asset.vertices.v3d.mul.pn = attr.vert_c;
                m_asset.vertices.v3d.mul.pos = (PosData*) malloc(attr.vert_c * sizeof(PosData));
                m_file.read(reinterpret_cast<char*>(m_asset.vertices.v3d.mul.pos), m_asset.vertices.v3d.mul.pn * sizeof(PosData));
                break;

            default:
                break;
        }

        // Read texture data if needed
        if(m_asset.asset_mode == DAS_ASSET_MODE_3D_TEXTURE_MAPPED ||
           m_asset.asset_mode == __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR ||
           m_asset.asset_mode == DAS_ASSET_MODE_2D_TEXTURE_MAPPED) {
            readVertAttr(&attr, DAS_VTEX_HEADER_SIG, file_name, sizeof(TextureData));

            m_asset.vertices.v3d.mul.tn = attr.vert_c;
            m_asset.vertices.v3d.mul.tex = (TextureData*) malloc(m_asset.vertices.v3d.mul.tn * sizeof(TextureData));
            m_file.read(reinterpret_cast<char*>(m_asset.vertices.v3d.mul.tex), m_asset.vertices.v3d.mul.tn * sizeof(TextureData));
        }

        // Read vertex normals if needed
        if(m_asset.asset_mode == DAS_ASSET_MODE_3D_TEXTURE_MAPPED ||
           m_asset.asset_mode == DAS_ASSET_MODE_3D_UNMAPPED) {
            readVertAttr(&attr, DAS_VNOR_HEADER_SIG, file_name, sizeof(NormalData));

            m_asset.vertices.v3d.mul.nn = attr.vert_c;
            m_asset.vertices.v3d.mul.norm = (NormalData*) malloc(attr.vert_c * sizeof(NormalData));
            m_file.read(reinterpret_cast<char*>(m_asset.vertices.v3d.mul.norm), m_asset.vertices.v3d.mul.nn * sizeof(NormalData));
        }


        // Read and copy index data
        readINDX_HDR(m_asset.asset_mode, &indx_hdr, file_name, true);
        m_asset.indices.n = indx_hdr.ind_c;
        m_asset.indices.pos = indx_hdr.pinds;
        m_asset.indices.tex = indx_hdr.tinds;
        m_asset.indices.norm = indx_hdr.ninds;

        m_file.close();
    }


    void das_loader::cleanupAsset(const Asset &asset) {
        switch(asset.asset_mode) {
            case DAS_ASSET_MODE_2D_UNMAPPED:
                free(asset.vertices.v2d.mul.pos);
                free(asset.indices.pos);
                break;

            case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
                free(asset.vertices.v2d.mul.pos);
                free(asset.vertices.v2d.mul.tex);
                free(asset.indices.pos);
                free(asset.indices.tex);
                break;

            case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
                free(asset.vertices.v3d.mul.pos);
                free(asset.indices.pos);
                break;

            case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
                free(asset.vertices.v3d.mul.pos);
                free(asset.indices.pos);
                free(asset.vertices.v3d.mul.tex);
                free(asset.indices.tex);
                break;

            case DAS_ASSET_MODE_3D_UNMAPPED:
                free(asset.vertices.v3d.mul.pos);
                free(asset.indices.pos);
                free(asset.vertices.v3d.mul.norm);
                free(asset.indices.norm);
                break;

            case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
                free(asset.vertices.v3d.mul.pos);
                free(asset.indices.pos);
                free(asset.vertices.v3d.mul.tex);
                free(asset.indices.tex);
                free(asset.vertices.v3d.mul.norm);
                free(asset.indices.norm);
                break;

            default:
                break;
        }
    }


    Asset &das_loader::getAsset() {
        return m_asset;
    }
}
