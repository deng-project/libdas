/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: FileNameString.h - File name string handling functions header
/// author: Karl-Mihkel Ott

#ifndef _FILE_NAME_STRING_H
#define _FILE_NAME_STRING_H

namespace Libdas {

    namespace String {
        /**
         * Concatenate name args into single name string
         * @param _names reference to std::vector<std::string> that contains all name strings to concatenate
         * @return std::string object that is essentially the concatenated name string
         */
        std::string ConcatenateNameArgs(const std::vector<std::string> &_names) {
            std::string name = "";
            for(size_t i = 0; i < _names.size(); i++) {
                name += _names[i];
                if(i != _names.size() - 1)
                    name += " ";
            }

            return name;
        }


        /**
         * Extract pure file name from given file path string
         * @param _path is the given file path
         * @return std::string instance that contains file name only without path
         */
        std::string ExtractFileName(const std::string &_path) {
            int fi = INT_MAX;
            for(int i = static_cast<int>(_path.size()) - 2; i >= 0; i--) {
                if(_path[i] == '/' || _path[i] == '\\') {
                    fi = i;
                    break;
                }
            }

            if(fi != INT_MAX) return _path.substr(fi + 1);
            else return _path;
        }


        /**
         * Extract file extension from given file name
         * @param _file_name is a given file name
         * @return Extracted std::string instance that defines the file extension name.
         *         If the extracted string is empty, then it means that no extension could be found 
         *         for the file
         */
        std::string ExtractFileExtension(const std::string &_file_name) {
            std::string ext = "";
            for(int i = static_cast<int>(_file_name.size()) - 2; i >= 0; i--) {
                if(_file_name[i] == '.') {
                    ext = _file_name.substr(i + 1);
                    break;
                }
            }

            return ext;
        }
    }
}

#endif
