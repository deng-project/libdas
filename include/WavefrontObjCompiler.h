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
    #include <cfloat>
    #include <cmath>
    #include <iostream>
    #include <fstream>
    #include <vector>
    #include <queue>
    #include <deque>
    #include <unordered_map>
    
    #include <Api.h>
    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <Quaternion.h>
    #include <Hash.h>

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
            struct Vertex {
                Point3D<float> pos = { FLT_MAX, FLT_MAX, FLT_MAX };
                Point2D<float> uv = { FLT_MAX, FLT_MAX };
                Point3D<float> normal = { FLT_MAX, FLT_MAX, FLT_MAX };

                bool operator==(const Vertex &_v) const {
                    return pos == _v.pos && uv == _v.uv && normal == _v.normal;
                }
            };

            std::vector<Point3D<float>> m_unique_pos;
            std::vector<Point2D<float>> m_unique_uv;
            std::vector<Point3D<float>> m_unique_normals;
            std::vector<uint32_t> m_indices;

            std::vector<uint32_t> m_indices_offsets_per_group;

        private:
            /**
             * Look into WavefrontObjGroups and create buffer structures out of them
             * @param _groups is a reference to std::vector<WavefrontObjGroup> instance that contains all parsed
             * Wavefront OBJ groups
             * @return std::vector<DasBuffer> instance that has all buffer memory area pointers that should be written
             */
            std::vector<DasBuffer> _CreateBuffers(const std::vector<std::string> &_embedded_textures);
            /**
             * Create mesh primitives from existing WavefrontObj groups
             * @param _data specifies a reference to WavefrontObjData object
             * @return std::vector instance containing DasMeshPrimitives
             */
            std::vector<DasMeshPrimitive> _CreateMeshPrimitives(const WavefrontObjData &_data);
            /**
             * Create DasMesh objects from given WavefrontObj groups
             * @param _data specifies a reference to WavefrontObjData object
             * @return std::vector instance containing DasMesh objects
             */
            std::vector<DasMesh> _CreateMeshes(const std::vector<DasMeshPrimitive> &_primitives);
            /**
             * Create a root scene and nodes out of all meshes
             * @param _meshes specifies a reference to std::vector instance containing information about all meshes
             */
            void _CreateRootScene(const std::vector<DasMesh> &_meshes);
            /**
             * Triangulate existing vertex data using simple quad triangulation
             * As of now more complex triangulation algorithms are not supported
             * @param _data specifies a reference to WavefrontObjData object
             */
            void _TriangulateFaces(WavefrontObjData &_data);
            /**
             * Reindex all vertices and fill unique vertex attribute vectors
             * @param _data specifies a reference to WavefrontObjData object
             */
            void _ReindexFaces(WavefrontObjData &_data);

        public:
            WavefrontObjCompiler(const std::string &_out_file = "");
            WavefrontObjCompiler(WavefrontObjData &_data, const DasProperties &_props, const std::string &_out_file, 
                                 const std::vector<std::string> &_embedded_textures = {});
            /**
             * Compile the DAS file from given WavefrontObjGroup objects.
             * @param _groups is a reference to std::queue object that contains all given WavfrontObjGroup objects 
             * @param _props is a reference to DasProperties object that contains all properties to write
             * @param _out_file is an optional file name that can be given as a parameter
             */
            void Compile(WavefrontObjData &_data, const DasProperties &_props, const std::string &_out_file = "", 
                         const std::vector<std::string> &_embedded_textures = {});
    };
}

#endif
