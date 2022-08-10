// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: STLCompiler.h - STL file to DAS compiler class header
// author: Karl-Mihkel Ott

#ifndef STL_COMPILER_H
#define STL_COMPILER_H

#ifdef STL_COMPILER_CPP
    #include <string>
    #include <fstream>
    #include <cstring>
    #include <cmath>
#ifdef _DEBUG
    #include <iostream>
#endif
    #include <vector>
    #include <unordered_map>
    #include <array>

    #include "trs/Iterators.h"
    #include "trs/Points.h"
    #include "trs/Vector.h"
    #include "trs/Matrix.h"
    #include "trs/Quaternion.h"

    #include "das/Api.h"
    #include "das/Hash.h"
    #include "das/LibdasAssert.h"
    #include "das/ErrorHandlers.h"
    #include "das/DasStructures.h"
    #include "das/TextureReader.h"
    #include "das/DasWriterCore.h"
    #include "das/STLStructures.h"
#endif


namespace Libdas {

    /**
     * Compiler class that uses STL files as source and outputs DAS file
     */
    class LIBDAS_API STLCompiler : private DasWriterCore {
        private:
            struct Vertex {
                TRS::Point3D<float> pos;
                TRS::Point3D<float> norm;

                bool operator==(const Vertex _v1) const {
                    return pos == _v1.pos && norm == _v1.norm;
                }
            };

            std::vector<TRS::Point3D<float>> m_unique_positions;
            std::vector<TRS::Point3D<float>> m_unique_normals;
            std::vector<uint32_t> m_indices;

        private:
            /**
             * Create indexed vertices out of given STL objects
             * @param _objects specifies a reference to const std::vector instance, containing all STL objects
             */
            void _IndexVertices(const std::vector<STLObject> &_objects);
            /**
             * Create a buffer object from parsed or generated vertices, normals and indices 
             * @return DasBuffer instance containing data
             */
            DasBuffer _CreateBuffers();
            /**
             * Create DasMeshPrimitive instances from given STLObjects
             * @param _objects is a reference to std::vector instance that stores all STL objects to be converted into mesh primitives
             * @return std::vector instance containing all mesh primitives
             */
            std::vector<DasMeshPrimitive> _CreateMeshPrimitives(const std::vector<STLObject> &_objects);
            /**
             * Create DasMesh instances from given STLObjects 
             * @param _objects is a reference to std::vector instance that stores all objects to be converted into models
             * @return std::vector instance containing all DasModel instance
             */
            DasMesh _CreateMesh(uint32_t primitive_count);
            /**
             * Create and write main scene with its single node
             * @param _meshes specifies an std::vector instance containing all converted DasMesh objects
             */
            void _CreateDefaultScene();

        public:
            STLCompiler(const std::string &_out_file = "");
            STLCompiler(std::vector<STLObject> &_objects, DasProperties &_props, std::string &_out_file);

            /**
             * Compile the DAS file from given STLObject objects
             * @param _objects is a reference to std::vector object that stores all STL objects that should be written to the output file
             * @param _props is a reference to DasProperties that specifies all properties that should be used for the output file
             * @param _out_file is an optional argument that specifies the output file name, if the file name was given in constructor 
             * it can be ignored
             */
            void Compile(const std::vector<STLObject> &_objects, DasProperties &_props, const std::string &_out_file);
    };
}

#endif
