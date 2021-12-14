#ifndef STL_COMPILER_H
#define STL_COMPILER_H

#ifdef STL_COMPILER_CPP
    #include <string>
    #include <fstream>
    #include <cstring>
#ifdef _DEBUG
    #include <iostream>
#endif
    #include <vector>
    #include <unordered_map>
    #include <array>

    #include <Iterators.h>
    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>

    #include <LibdasAssert.h>
    #include <ErrorHandlers.h>
    #include <DasStructures.h>
    #include <DasWriterCore.h>

    #include <STLStructures.h>

    #define VERTICES_ID 0
    #define NORMALS_ID  1
    #define INDICES_ID  2
#endif


namespace Libdas {

    /**
     * Compiler class that uses STL files as source and outputs DAS file
     */
    class STLCompiler : private DasWriterCore {
        private:
            std::vector<Point4D<float>> m_unique_verts;
            std::vector<Point3D<float>> m_unique_normals;
            std::vector<DasFace> m_faces;
            std::vector<uint32_t> m_index_offsets;

        private:
            /**
             * Create indexed vertices out of given STL objects
             * @param _objects specifies a reference to const std::vector instance, containing all STL objects
             */
            void _IndexVertices(const std::vector<STLObject> &_objects);
            /**
             * Create buffers from parsed vertices, normals and faces 
             * @return std::array instance containing all created buffers
             */
            std::array<DasBuffer, 3> _CreateBuffers();
            /**
             * Create DasModel instances from given STLObjects 
             * @param _objects is a reference to std::vector instance that stores all objects to be converted into models
             * @return std::vector instance containing all DasModel instance
             */
            std::vector<DasModel> _CreateModels(const std::vector<STLObject> &_objects);

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
