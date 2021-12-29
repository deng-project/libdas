// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: FileNameString.h - File name string handling functions header
// author: Karl-Mihkel Ott

#ifndef FILE_NAME_STRING_H
#define FILE_NAME_STRING_H

#ifdef FILE_NAME_STRING_CPP
    #include <string>
    #include <vector>
    #include <climits>
#endif


namespace Libdas {

    namespace String {

        /**
         * Replace file extension from input file with specified new one
         * @param _input_file is a file name that needs to have its extension replaced
         * @param _ext is a supplement extension to be used
         * @return std::string instance specifying the new file name with new extension
         */
        std::string ReplaceFileExtension(const std::string &_input_file, const std::string &_ext);
        /**
         * Create Knutt-Morris-Pratt LSP array
         * @param _keyword is a string to use for LSP array generation
         * @return std::vector value specifying the LSP array
         */
        std::vector<size_t> CreateLSPArray(const std::string &_keyword);
        /**
         * Find all substring instances from specified string
         * @param _str is the main substring where substring sequence data is searched from
         * @param _keyword is the substring that is searched
         */
        std::vector<size_t> FindSubstringInstances(const std::string &_str, const std::string &_keyword);
        /**
         * Find all substring instances from specified string
         * @param _str is the main substring in char* format where substring sequence data is searched from
         * @param _str_len specifies the length of buffer _str
         * @param _keyword is the substring that is searched
         */
        std::vector<size_t> FindSubstringInstances(const char *_str, size_t _str_len, const std::string &_keyword);
        /**
         * Concatenate name args into single name string
         * @param _names reference to std::vector<std::string> that contains all name strings to concatenate
         * @return std::string object that is essentially the concatenated name string
         */
        std::string ConcatenateNameArgs(const std::vector<std::string> &_names);
        /**
         * Extract pure file name from given file path string
         * @param _path is the given file path
         * @return std::string instance that contains file name only without path
         */
        std::string ExtractFileName(const std::string &_path);
        /**
         * Extract file extension from given file name
         * @param _file_name is a given file name
         * @return Extracted std::string instance that defines the file extension name.
         *         If the extracted string is empty, then it means that no extension could be found 
         *         for the file
         */
        std::string ExtractFileExtension(const std::string &_file_name);
        /**
         * Extract root path from given file name
         * @param _file_name is given file name
         * @return Extracted std::string instance that specifies the root path used for the file
         */
        std::string ExtractRootPath(const std::string &_file_name);
    }
}

#endif
