// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasParser.h - DAS format parser header
// author: Karl-Mihkel Ott

#ifndef DAS_PARSER_H
#define DAS_PARSER_H

#ifdef DAS_PARSER_CPP
    #include <any>
    #include <fstream>
    #include <vector>
    #include <string>
    #include <cstring>
    #include <cmath>
    #include <iostream>
    #include <unordered_map>

    #include "trs/Vector.h"
    #include "trs/Matrix.h"
    #include "trs/Points.h"
    #include "trs/Quaternion.h"

    #include "mar/AsciiStreamReader.h"
    #include "mar/AsciiLineReader.h"

    #include "das/Api.h"
    #include "das/LibdasAssert.h"
    #include "das/ErrorHandlers.h"
    #include "das/DasStructures.h"
    #include "das/DasReaderCore.h"
#endif

namespace Libdas {

    class LIBDAS_API DasParser : private DasReaderCore {
        private:
            DasModel m_model;

        private:
            /**
             * Cast scope stored in std::any object into real scope value
             * @param _any_scope is any reference object that will be casted into correct data type
             * @param _type is specified scope type 
             */
            void _DataCast(std::any &_any_scope, DasScopeType _type);

            /**
             * Find root nodes from given scene
             * @param _id is a reference to the DasScene object whose root nodes will be calculated
             */
            void _FindSceneNodeRoots(DasScene &_scene);

        public:
            DasParser(const std::string &_file_name = "");
            DasParser(DasParser &&_parser) noexcept;

            /**
             * Parse contents from provided DAS file into scene array.
             * If the file contains no scenes, a default scene will be created that should be considered as a
             * object library.
             * @param _clean_read is an optional argument when set to true, closes the file stream currently used
             * @param _file_name is an optional argument that specifies new file to use
             */
            void Parse(bool _clean_read = false, const std::string &_file_name = "");
            /**
             * Delete all heap allocated buffer data. Call this when all mesh primitives are copied into video memory.
             */
            void DeleteBuffers();

            ////////////////////////////////
            // ***** Getter methods ***** //
            ////////////////////////////////
            inline DasModel& GetModel() {
                return m_model;
            }
    };
}

#endif
