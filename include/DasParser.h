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
    #include <iostream>
    #include <unordered_map>

    #include <Vector.h>
    #include <Matrix.h>
    #include <ErrorHandlers.h>
    #include <AsciiStreamReader.h>
    #include <AsciiLineReader.h>
#endif

/// Import all structure definitions
#include <DasStructures.h>

#ifdef DAS_PARSER_CPP
    #include <DasReaderCore.h>
#endif

namespace Libdas {

    class DasParser : private DasReaderCore {
        private:
            DasProperties m_props;
            std::vector<DasBuffer> m_buffers;
            std::vector<DasModel> m_models;
            std::vector<DasAnimation> m_animations;
            std::vector<DasScene> m_scenes;

        private:
            /**
             * Cast scope stored in std::any object into real scope value
             * @param _any_scope is any reference object that will be casted into correct data type
             * @param _type is specified scope type 
             */
            void _DataCast(std::any &_any_scope, DasScopeType _type);

        public:
            DasParser(const std::string &_file_name = "");
            /**
             * Parse contents from provided DAS file into scene array.
             * If the file contains no scenes, a default scene will be created that should be considered as a
             * object library.
             * @param _file_name is an optional argument that specifies new file to use
             */
            void Parse(const std::string &_file_name = "");

            ////////////////////////////////
            // ***** Getter methods ***** //
            ////////////////////////////////
            /**
             * Get all specified file properties
             * @return reference to DasProperties object that specifies all parsed properties
             */
            DasProperties &GetProperties();
            /**
             * Get all parsed scenes
             * @return reference to std::vector<DasScene> object with all parsed scenes
             */
            std::vector<DasScene> &GetScenes();
            /**
             * Access buffer reference by id
             * @param _id specifies the buffer id to use for accessing
             * @return reference to DasBuffer object, accessed by the id
             */
            DasBuffer &AccessBuffer(uint32_t _id);
            /**
             * Access model reference by id
             * @param _id specifies the model id to use for accessing
             * @return reference to DasModel, accessed by the id
             */
            DasModel &AccessModel(uint32_t _id);
            /**
             * Access animation reference by id
             * @param _id specifies the animation id to use for accessing
             * @return reference to DasAnimation, accessed by the id
             */
            DasAnimation &AccessAnimation(uint32_t _id);

    };
}

#endif
