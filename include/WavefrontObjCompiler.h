/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjCompiler.h - Wavefront OBJ to DAS compiler class header
/// author: Karl-Mihkel Ott

#ifndef WAVEFRONT_OBJ_COMPILER_H
#define WAVEFRONT_OBJ_COMPILER_H


#ifdef WAVEFRONT_OBJ_COMPILER_CPP
    #include <string>
    #include <iostream>
    #include <fstream>
    #include <vector>
    #include <queue>
    #include <unordered_map>

    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>

    #include <LibdasAssert.h>
    #include <ErrorHandlers.h>
    #include <WavefrontObjStructures.h>
    #include <DasStructures.h>
    #include <DasWriterCore.h>
#endif


namespace Libdas {

    /**
     * Compiler class that uses Wavefront OBJ files as source files and outputs DAS file.
     * Since there are no scene nor animation structures in Wavefront OBJ format, the outputted DAS
     * files will be considered as mesh data libraries. Unless static assets are needed it would be 
     * wise to avoid Wavefront OBJ format completely, when importing models to DENG.
     */
    class WavefrontObjCompiler : private DasWriterCore {
        private:
            //std::vector<DasBuffer> _ReadImages();
            
            /**
             * Look into WavefrontObjGroups and create buffer structures out of them
             * @param _groups is a reference to std::vector<WavefrontObjGroup> instance that contains all parsed
             * Wavefront OBJ groups
             * @return std::vector<DasBuffer> instance that has all buffer memory area pointers that should be written
             */
            std::vector<DasBuffer> _CreateBuffers(std::vector<WavefrontObjGroup> &_groups);
            /**
             * Look into WavefrontObjGroups and determine all models with correct buffer offset values
             * @param _groups is a reference to std::vector<WavefrontObjGroup> instance that contains all parsed
             * Wavefront OBJ groups
             * @return std::vector<DasModel> instance that has contains all information about models and their correct
             * buffer offsets
             */
            std::vector<DasModel> _CreateModels(std::vector<WavefrontObjGroup> &_groups);
        public:
            WavefrontObjCompiler(const std::string &_out_file = "");
            WavefrontObjCompiler(std::vector<WavefrontObjGroup> &_groups, const std::string &_out_file = "");
            ~WavefrontObjCompiler();
            /**
             * Compile the DAS file from given WavefrontObjGroup objects.
             * @param _groups is a reference to std::queue object that contains all given WavfrontObjGroup objects 
             * @param _out_file is an optional file name that can be given as a parameter
             */
            void Compile(std::vector<WavefrontObjGroup> &_groups, const std::string &_out_file = "");
    };
}

#endif
