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
    #include <queue>
    #include <unordered_map>

    #include <Points.h>
    #include <LibdasAssert.h>
    #include <ParserErrorHandler.h>
    #include <WavefrontObjStructures.h>
#endif


namespace Libdas {

    class WavefrontObjCompiler {
        private:

        public:
            WavefrontObjCompiler(const std::string &_out_file = "");
            WavefrontObjCompiler(std::queue<WavefrontObjGroup> &_groups, const std::string &_out_file = "");
            ~WavefrontObjCompiler();

            /**
             * Compile the DAS file from given WavefrontObjGroup objects
             * @param _groups is a reference to std::queue object that contains all given WavfrontObjGroup objects 
             * @param _out_file is an optional file name that can be given as a parameter
             */
            void Compile(std::queue<WavefrontObjGroup> &_groups, const std::string &_out_file = "");
    };
}

#endif
