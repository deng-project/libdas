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

    class WavefrontObjCompiler : private DasWriterCore {
        private:
            std::vector<DasBuffer> _ReadImages();
            std::vector<DasBuffer> _CreateBuffers(std::vector<WavefrontObjGroup> &_groups);
        public:
            WavefrontObjCompiler(const std::string &_out_file = "");
            WavefrontObjCompiler(std::vector<WavefrontObjGroup> &_groups, const std::string &_out_file = "");
            ~WavefrontObjCompiler();
            /**
             * Compile the DAS file from given WavefrontObjGroup objects
             * @param _groups is a reference to std::queue object that contains all given WavfrontObjGroup objects 
             * @param _out_file is an optional file name that can be given as a parameter
             */
            void Compile(std::vector<WavefrontObjGroup> &_groups, const std::string &_out_file = "");
    };
}

#endif
