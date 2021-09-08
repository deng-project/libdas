/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: asset_writer.cpp - das binary writer source code
/// author: Karl-Mihkel Ott


#define DAS_FILE
#define __DAS_ASSET_WRITER_CPP
#include <das_asset_writer.h>


namespace libdas {

    AssetWriter::AssetWriter() {}


    AssetWriter::AssetWriter(const Asset &asset, const std::string &file_name, const char *meta) {
        createAssetFile(asset, file_name, meta);
    }


    /// Write the initial FILE_HDR data into file
    void AssetWriter::writeFILE_HDR() {
        FILE_HDR fhdr = {};
        fhdr.hdr_sig = DAS_FILE_HEADER_SIG;
        m_file.write(reinterpret_cast<char*>(&fhdr), sizeof(FILE_HDR));
    }


    /// Write INFO_HDR data into file
    void AssetWriter::writeINFO_HDR(const Asset &asset) {
        INFO_HDR ihdr = {};

        ihdr.hdr_sig = DAS_INFO_HEADER_SIG;
        ihdr.hdr_size = sizeof(INFO_HDR);
        ihdr.asset_type = static_cast<AssetMode>(asset.asset_mode);

        // reserved for future compression algorithms
        ihdr.cmpr = 0;
        ihdr.time_st = time(NULL);
        strcpy(ihdr.uuid, asset.uuid.bytes);

        m_file.write(reinterpret_cast<char*>(&ihdr), sizeof(INFO_HDR));
    }


    /// Write metadata into its corresponding header
    void AssetWriter::writeMETA_HDR(const char *meta) {
        META_HDR mhdr = { 0 };


        const size_t mhdr_size = 16;
        mhdr.hdr_sig = DAS_META_HEADER_SIG;
        mhdr.data_size = strlen(meta) + 1;
        mhdr.hdr_size = mhdr_size + mhdr.data_size;

        m_file.write(reinterpret_cast<char*>(&mhdr), mhdr_size);
        m_file.write(meta, mhdr.data_size);
    }


    /// Write the initial VERT_HDR data to the file
    void AssetWriter::writeVERT_HDR(const Asset &asset) {
        const size_t vhdr_size = 12;
        VERT_HDR vhdr = {};
        vhdr.hdr_sig = DAS_VERT_HEADER_SIG;
        vhdr.hdr_size = vhdr_size;

        // Find out the header size according to assets asset mode
        switch(asset.asset_mode) {
            case DAS_ASSET_MODE_2D_UNMAPPED:
                vhdr.hdr_size += vhdr_size;
                vhdr.hdr_size += asset.vertices.v2d.mul.pn * sizeof(PosData2D);
                break;

            case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
                vhdr.hdr_size += vhdr_size;
                vhdr.hdr_size += asset.vertices.v2d.mul.pn * sizeof(PosData2D);
                vhdr.hdr_size += asset.vertices.v2d.mul.tn * sizeof(TextureData);
                break;

            case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
                vhdr.hdr_size += vhdr_size;
                vhdr.hdr_size += asset.vertices.v3d.mul.pn * sizeof(PosData);
                break;

            case DAS_ASSET_MODE_3D_UNMAPPED:
                vhdr.hdr_size += vhdr_size;
                vhdr.hdr_size += asset.vertices.v3d.mul.pn * sizeof(PosData);
                vhdr.hdr_size += asset.vertices.v3d.mul.nn * sizeof(NormalData);
                break;

            case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
                vhdr.hdr_size += vhdr_size;
                vhdr.hdr_size += asset.vertices.v3d.mul.pn * sizeof(PosData);
                vhdr.hdr_size += asset.vertices.v3d.mul.tn * sizeof(TextureData);
                break;

            case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
                vhdr.hdr_size += vhdr_size;
                vhdr.hdr_size += asset.vertices.v3d.mul.pn * sizeof(PosData);
                vhdr.hdr_size += asset.vertices.v3d.mul.tn * sizeof(TextureData);
                vhdr.hdr_size += asset.vertices.v3d.mul.nn * sizeof(NormalData);
                break;

            default:
                return;
        }

        m_file.write(reinterpret_cast<char*>(&vhdr), vhdr_size);
    }


    /// Write generic vertex attribute header and its data
    void AssetWriter::writeGenericVertAttrHDR(void *vt, uint32_t vc, uint32_t esize, uint64_t sig,
                                              const char *emsg, uint64_t pos_size) {
        VertAttribute vthdr = {};

        const size_t vgen_size = 17;
        uint64_t dsize = 0;
        if(!pos_size) dsize = vc * esize * sizeof(float);
        else dsize = vc * pos_size;
        vthdr.hdr_sig = sig;
        vthdr.esize = esize;
        vthdr.vert_c = vc;
        vthdr.hdr_size = vgen_size + dsize;

        // Write header without its data
        m_file.write(reinterpret_cast<char*>(&vthdr), vgen_size);

        // Write vertex data
        m_file.write(reinterpret_cast<char*>(vt), dsize);
    }


    /// Write INDX_HDR and all indicies data associated with it
    void AssetWriter::writeINDX_HDR(const Asset &asset) {
        INDX_HDR ihdr = {};

        const size_t ihdr_size = 16;
        ihdr.hdr_sig = DAS_INDX_HEADER_SIG;
        ihdr.ind_c = asset.indices.n;

        // Check the asset mode and thus find the header size
        switch(asset.asset_mode) {
            case DAS_ASSET_MODE_2D_UNMAPPED:
            case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
                ihdr.hdr_size = ihdr_size + asset.indices.n * sizeof(uint32_t);
                break;

            case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
            case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
            case DAS_ASSET_MODE_3D_UNMAPPED:
                ihdr.hdr_size = ihdr_size + 2 * asset.indices.n * sizeof(uint32_t);
                break;

            case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
                ihdr.hdr_size = ihdr_size + 3 * asset.indices.n * sizeof(uint32_t);
                break;

            default:
                break;
        }


        // Write header data without actual indices
        m_file.write(reinterpret_cast<char*>(&ihdr), ihdr_size);

        // Check the asset type and write correct indices data to file
        switch(asset.asset_mode) {
            case DAS_ASSET_MODE_2D_UNMAPPED:
                m_file.write(reinterpret_cast<char*>(asset.indices.pos), ihdr.ind_c * sizeof(uint32_t));
                break;

            case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
                m_file.write(reinterpret_cast<char*>(asset.indices.pos), ihdr.ind_c * sizeof(uint32_t));
                m_file.write(reinterpret_cast<char*>(asset.indices.tex), ihdr.ind_c * sizeof(uint32_t));
                break;

            case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
                m_file.write(reinterpret_cast<char*>(asset.indices.pos), ihdr.ind_c * sizeof(uint32_t));
                break;

            case DAS_ASSET_MODE_3D_UNMAPPED:
                m_file.write(reinterpret_cast<char*>(asset.indices.pos), ihdr.ind_c * sizeof(uint32_t));
                m_file.write(reinterpret_cast<char*>(asset.indices.norm), ihdr.ind_c * sizeof(uint32_t));
                break;

            case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
                m_file.write(reinterpret_cast<char*>(asset.indices.pos), ihdr.ind_c * sizeof(uint32_t));
                m_file.write(reinterpret_cast<char*>(asset.indices.tex), ihdr.ind_c * sizeof(uint32_t));
                break;

            case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
                m_file.write(reinterpret_cast<char*>(asset.indices.pos), ihdr.ind_c * sizeof(uint32_t));
                m_file.write(reinterpret_cast<char*>(asset.indices.tex), ihdr.ind_c * sizeof(uint32_t));
                m_file.write(reinterpret_cast<char*>(asset.indices.norm), ihdr.ind_c * sizeof(uint32_t));
                break;

            default:
                // some assertion
                msgassert(NULL, "Invalid asset mode specified");
                break;
        }
    }



    /// Explicitly create a new asset file from given data
    void AssetWriter::createAssetFile(const Asset &asset, const std::string &file_name, const char *meta) {
        m_file.open(file_name, std::ios::out | std::ios::binary);
        writeFILE_HDR();
        writeINFO_HDR(asset);
        if(meta && *meta != 0) writeMETA_HDR(meta);
        writeVERT_HDR(asset);

        // Check the asset mode and decide which vertex attributes to write
        switch(asset.asset_mode) {
            case DAS_ASSET_MODE_2D_UNMAPPED:
                writeGenericVertAttrHDR(asset.vertices.v2d.mul.pos, asset.vertices.v2d.mul.pn,
                                        2, DAS_VPOS_HEADER_SIG, 
                                        "Could not write 2D asset vertex position attributes", 
                                        sizeof(PosData2D));
                break;

            case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
                writeGenericVertAttrHDR(asset.vertices.v2d.mul.pos, asset.vertices.v2d.mul.pn,
                                        2, DAS_VPOS_HEADER_SIG, 
                                        "Could not write 2D asset vertex position attributes", 
                                        sizeof(PosData2D));

                writeGenericVertAttrHDR(asset.vertices.v2d.mul.tex, asset.vertices.v2d.mul.tn,
                                        2, DAS_VTEX_HEADER_SIG, 
                                        "Could not write 2D asset vertex texture attributes", 
                                        0);
                break;
            
            case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
                writeGenericVertAttrHDR(asset.vertices.v3d.mul.pos, asset.vertices.v3d.mul.pn,
                                        3, DAS_VPOS_HEADER_SIG, 
                                        "Could not write 3D asset vertex position attributes", 
                                        sizeof(PosData));
                break;

            case DAS_ASSET_MODE_3D_UNMAPPED:
                writeGenericVertAttrHDR(asset.vertices.v3d.mul.pos, asset.vertices.v3d.mul.pn,
                                        3, DAS_VPOS_HEADER_SIG, 
                                        "Could not write 3D asset vertex position attributes", 
                                        sizeof(PosData));

                writeGenericVertAttrHDR(asset.vertices.v3d.mul.norm, asset.vertices.v3d.mul.nn,
                                        3, DAS_VNOR_HEADER_SIG, 
                                        "Could not write 3D asset vertex normal attributes", 
                                        0);
                break;

            case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
                writeGenericVertAttrHDR(asset.vertices.v3d.mul.pos, asset.vertices.v3d.mul.pn,
                                        3, DAS_VPOS_HEADER_SIG, 
                                        "Could not write 3D asset vertex position attributes", 
                                        sizeof(PosData));

                writeGenericVertAttrHDR(asset.vertices.v3d.mul.tex, asset.vertices.v3d.mul.tn,
                                        2, DAS_VTEX_HEADER_SIG, 
                                        "Could not write 3D asset vertex texture attributes", 
                                        0);
                break;
            
            case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
                writeGenericVertAttrHDR(asset.vertices.v3d.mul.pos, asset.vertices.v3d.mul.pn,
                                        3, DAS_VPOS_HEADER_SIG, 
                                        "Could not write 3D asset vertex position attributes", 
                                        sizeof(PosData));

                writeGenericVertAttrHDR(asset.vertices.v3d.mul.tex, asset.vertices.v3d.mul.tn,
                                        2, DAS_VTEX_HEADER_SIG, 
                                        "Could not write 3D asset vertex texture attributes", 
                                        0);

                writeGenericVertAttrHDR(asset.vertices.v3d.mul.norm, asset.vertices.v3d.mul.nn,
                                        3, DAS_VNOR_HEADER_SIG, 
                                        "Could not write 3D asset vertex normal attributes", 
                                        0);
                break;

            default: break;
        }

        writeINDX_HDR(asset);
        m_file.close();
    }

}
