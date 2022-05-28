// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: Algorithm.h - Libdas algorithm header file
// author: Karl-Mihkel Ott


#ifndef ALGORITHM_H
#define ALGORITHM_H

#ifdef ALGORITHM_CPP
    #include <string>
    #include <vector>
    #include <climits>
    #include <cerrno>
    #include <iostream>
    #include <algorithm>
#if defined(_WIN32)
    #include <direct.h>
    #include <Windows.h>
    #include <libloaderapi.h>
    #define GetCurrentWorkingDir _getcwd
#elif defined(__linux__)
    #include <unistd.h>
    #define GetCurrentWorkingDir getcwd
#endif

    #include <Api.h>
    #include <ErrorHandlers.h>
#endif


namespace Libdas {

    namespace Algorithm {
        /**
         * Remove duplicate entries from sorted vector
         * @param _vec specifies the vector where to perform removal actions
         * @param _pIsEqual specifies a function pointer to function that compares two instances
         */
        template<typename _T>
        void RemoveDuplicates(std::vector<_T> &_vec, bool(*_pIsEqual)(const _T&, const _T&)) {
            for(size_t i = 0; i < _vec.size(); i++) {
                for(size_t j = i + 1; j < _vec.size(); j++) {
                    if(_pIsEqual(_vec[i], _vec[j])) {
                        _vec.erase(_vec.begin() + j);
                        j--;
                    } else break;
                }
            }
        }
        /**
         * Remove duplicate entries from sorted vector
         * @param _vec specifies the vector where to perform removal actions
         * @complexity O(n^2)
         */
        template<typename _T>
        void RemoveDuplicates(std::vector<_T> &_vec) {
            for(size_t i = 0; i < _vec.size(); i++) {
                for(size_t j = i + 1; j < _vec.size(); j++) {
                    if(_vec[i] == _vec[j]) {
                        _vec.erase(_vec.begin() + j);
                        j--;
                    } else break;
                }
            }
        }

        /**
         * Replace file extension from input file with specified new one
         * @param _input_file is a file name that needs to have its extension replaced
         * @param _ext is a supplement extension to be used
         * @return std::string instance specifying the new file name with new extension
         */
        LIBDAS_API std::string ReplaceFileExtension(const std::string &_input_file, const std::string &_ext);

        /**
         * Convert relative path to absolute path
         * @param _rel specifying the relative path
         * @return std::string instance containing the generated absolute path
         */
        LIBDAS_API std::string RelativePathToAbsolute(const std::string& _rel);

        /**
         * Get the program executable path
         * @return std::string instance containing the executable path
         */
        LIBDAS_API std::string GetProgramPath();

        /**
         * Create Knutt-Morris-Pratt LSP array
         * @param _keyword is a string to use for LSP array generation
         * @return std::vector value specifying the LSP array
         */
        LIBDAS_API std::vector<size_t> CreateLSPArray(const std::string &_keyword);
        
        /**
         * Find all substring instances from specified string
         * @param _str is the main substring where substring sequence data is searched from
         * @param _keyword is the substring that is searched
         */
        LIBDAS_API std::vector<size_t> FindSubstringInstances(const std::string &_str, const std::string &_keyword);

        /**
         * Find all substring instances from specified string
         * @param _str is the main substring in char* format where substring sequence data is searched from
         * @param _str_len specifies the length of buffer _str
         * @param _keyword is the substring that is searched
         */
        LIBDAS_API std::vector<size_t> FindSubstringInstances(const char *_str, size_t _str_len, const std::string &_keyword);

        /**
         * Concatenate name args into single name string
         * @param _names reference to std::vector<std::string> that contains all name strings to concatenate
         * @return std::string object that is essentially the concatenated name string
         */
        LIBDAS_API std::string ConcatenateNameArgs(const std::vector<std::string> &_names);

        /**
         * Extract pure file name from given file path string
         * @param _path is the given file path
         * @return std::string instance that contains file name only without path
         */
        LIBDAS_API std::string ExtractFileName(const std::string &_path);
        
        /**
         * Extract file extension from given file name
         * @param _file_name is a given file name
         * @return Extracted std::string instance that defines the file extension name.
         *         If the extracted string is empty, then it means that no extension could be found 
         *         for the file
         */
        LIBDAS_API std::string ExtractFileExtension(const std::string &_file_name);

        /**
         * Extract root path from given file name
         * @param _file_name is given file name
         * @return Extracted std::string instance that specifies the root path used for the file
         */
        LIBDAS_API std::string ExtractRootPath(const std::string &_file_name);

        /**
         * Remove all numbers from given string
         * @complexity O(n)
         * @param _str is a given string to purge numbers from
         * @return std::string instance without any numbers
         */
        LIBDAS_API std::string RemoveNumbers(const std::string &_str);

        /**
         * Find largest continuos storage blocks from given boundaries
         * @complexity O(n)
         * @param _blocks specifies a vector containing all sorted storage block boundaries
         * @return std::vector instance containing largest continuous storage blocks
         */
        LIBDAS_API std::vector<std::pair<uint32_t, uint32_t>> FindContinuousStorageBlocks(const std::vector<std::pair<uint32_t, uint32_t>> &_blocks);
    }
}


#endif
