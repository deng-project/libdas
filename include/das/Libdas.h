// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: Libdas.h - Metaheader for containing all Libdas classes
// author: Karl-Mihkel Ott

/*! \mainpage Libdas documentation
 *
 * \section intro_sec Introduction
 * Libdas is a small library that features custom file format reading and writing features, collision detection
 * and parsers for third party file formats. DAS is an acronym that stands for DENG asset format. The format features
 * data buffers, texture image buffers, Huffman compression, scene hierarchies and animations.
 *
 * \section build_sec Building
 * Libdas can be built using premake5 with following commands:
 * \code{}
 * $ premake5 gmake2
 * \endcode
 * and 
 * \code{}
 * $ premake5 vs2019
 * \endcode
 * when using GNU/Make and Visual Studio respectively.
 *
 * Additionally tests can also be built using --tests=<NAME> flag. See premake5 --help for more information
 */

#ifndef LIBDAS_H
#define LIBDAS_H

#ifndef LIBDAS_NO_STL
    #include <fstream>
    #include <vector>
    #include <queue> // temporary
    #include <unordered_map>
#endif

// misc includes
#include "mar/AsciiStreamReader.h"
#include "mar/AsciiLineReader.h"
#include "ErrorHandlers.h"

// Points, vectors and matrices
#include "trs/Points.h"
#include "trs/Vector.h"
#include "trs/Matrix.h"

// version
#include "das/Version.h"

// DAS format handling related includes
#include "das/DasStructures.h"
#include "das/DasWriterCore.h"

// Wavefront OBJ format handling related includes
#include "das/WavefrontObjStructures.h"
#include "das/WavefrontObjCompiler.h"

// STL format handling related includes
#include "das/STLStructures.h"
#include "das/STLParser.h"

// Texture reading
#include "das/TextureReader.h"

#endif
