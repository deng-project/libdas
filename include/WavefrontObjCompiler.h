// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: WavefrontObjCompiler.h - Wavefront OBJ to DAS compiler class header
// author: Karl-Mihkel Ott

#ifndef WAVEFRONT_OBJ_COMPILER_H
#define WAVEFRONT_OBJ_COMPILER_H


#ifdef WAVEFRONT_OBJ_COMPILER_CPP
    #include <string>
    #include <climits>
    #include <cstring>
    #include <cstdio>
    #include <cmath>
    #include <iostream>
    #include <fstream>
    #include <vector>
    #include <queue>
    #include <unordered_map>
    
    #include <Api.h>
    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <Quaternion.h>

    #include <LibdasAssert.h>
    #include <ErrorHandlers.h>
    #include <Algorithm.h>
    #include <HuffmanCompression.h>
    #include <WavefrontObjStructures.h>
    #include <DasStructures.h>
    #include <TextureReader.h>
    #include <DasWriterCore.h>
    //#include <DasReaderCore.h>
#endif


namespace Libdas {

    /**
     * Compiler class that uses Wavefront OBJ files as source files and outputs DAS file.
     */
    class LIBDAS_API WavefrontObjCompiler : private DasWriterCore {
        private:
            const uint32_t m_face_attr_count = 3;
            uint32_t m_texture_id = UINT32_MAX;
            uint32_t m_vertex_buffer_id = UINT32_MAX;
            uint32_t m_texture_map_buffer_id = UINT32_MAX;
            uint32_t m_vertex_normal_buffer_id = UINT32_MAX;
            uint32_t m_indices_buffer_id = UINT32_MAX;

        private:
            /**
             * Look into WavefrontObjGroups and create buffer structures out of them
             * @param _groups is a reference to std::vector<WavefrontObjGroup> instance that contains all parsed
             * Wavefront OBJ groups
             * @return std::vector<DasBuffer> instance that has all buffer memory area pointers that should be written
             */
            std::vector<DasBuffer> _CreateBuffers(const std::vector<WavefrontObjGroup> &_groups, const std::vector<std::string> &_embedded_textures);
            /**
             * Look into WavefrontObjGroups and determine all mesh primitives with correct buffer offset values
             * @param _groups is a reference to std::vector<WavefrontObjGroup> instance that contains all parsed
             * Wavefront OBJ groups
             * @return std::vector<DasMeshPrimitive> instance that has contains all information about primitives and their correct
             * buffer offsets
             */
            std::vector<DasMeshPrimitive> _CreateMeshPrimitives(const std::vector<WavefrontObjGroup> &_groups);
            /**
             * Create a single mesh instance from all associated mesh primitives
             * @param _primitive_size specifies the total amount of mesh primitives to consider
             * @return DasMesh instance
             */
            DasMesh _CreateMesh(uint32_t _primitive_size);
            /**
             * Create a root scene and nodes out of all meshes
             * @param _meshes specifies a reference to std::vector instance containing information about all meshes
             */
            void _CreateRootScene();

        public:
            WavefrontObjCompiler(const std::string &_out_file = "");
            WavefrontObjCompiler(const std::vector<WavefrontObjGroup> &_groups, const DasProperties &_props, const std::string &_out_file, 
                                 const std::vector<std::string> &_embedded_textures = {});
            /**
             * Compile the DAS file from given WavefrontObjGroup objects.
             * @param _groups is a reference to std::queue object that contains all given WavfrontObjGroup objects 
             * @param _props is a reference to DasProperties object that contains all properties to write
             * @param _out_file is an optional file name that can be given as a parameter
             */
            void Compile(const std::vector<WavefrontObjGroup> &_groups, const DasProperties &_props, const std::string &_out_file = "", 
                         const std::vector<std::string> &_embedded_textures = {});
    };
}

#endif
