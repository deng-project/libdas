/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: FileNameString.h - File name string handling functions implementation
/// author: Karl-Mihkel Ott

#define FILE_NAME_STRING_CPP
#include <FileNameString.h>

namespace Libdas {
    namespace String {

        std::string ReplaceFileExtension(const std::string &_input_file, const std::string &_ext) {
            int32_t i = static_cast<int32_t>(_input_file.size() - 1);
            for(; i >= 0; i--) {
                if(_input_file[i] == '.')
                    break;
            }

            return _input_file.substr(0, i + 1) + _ext;
        }


        std::vector<size_t> CreateLSPArray(const std::string &_keyword) {
            std::vector<size_t> lsp(_keyword.size());

            size_t j = 0;
            for(size_t i = 1; i < _keyword.size(); i++) {
                j = lsp[i - 1];

                // skip all invalid prefixes
                while(j > 0 && _keyword[i] != _keyword[j])
                    j = lsp[j - 1];

                if(_keyword[i] == _keyword[j])
                    j = j + 1;

                lsp[i] = j;
            }

            return lsp;
        }


        std::vector<size_t> FindSubstringInstances(const std::string &_str, const std::string &_keyword) {
            std::vector<size_t> occurences;
            occurences.reserve(_str.size() / _keyword.size() > 0 ? 
                               _str.size() / _keyword.size() : 1); // assuming the worst case

            std::vector<size_t> lsp = CreateLSPArray(_keyword);

            size_t j = 0;
            for(size_t i = 0; i < _str.size(); i++) {
                // fallback on invalid values
                while(j > 0 && _str[i] != _keyword[j])
                    j = lsp[j - 1];

                if(_str[i] == _keyword[j]) {
                    j++;
                    if(j == _keyword.size()) {
                        occurences.push_back(i - j + 1);
                        j = lsp[j - 1];
                    }
                }
            }

            return occurences;
        }


        std::vector<size_t> FindSubstringInstances(const char *_str, size_t _str_len, const std::string &_keyword) {
            std::vector<size_t> occurences;
            occurences.reserve(_str_len / _keyword.size() > 0 ?
                               _str_len / _keyword.size() : 1);

            std::vector<size_t> lsp = CreateLSPArray(_keyword);

            size_t j = 0;
            for(size_t i = 0; i < _str_len; i++) {
                // fallback on invalid values
                while(j > 0 && _str[i] != _keyword[j])
                    j = lsp[j - 1];

                if(_str[i] == _keyword[j]) {
                    j++;
                    if(j == _keyword.size()) {
                        occurences.push_back(i - j + 1);
                        j = lsp[j - 1];
                    }
                }
            }

            return occurences;
        }


        std::string ConcatenateNameArgs(const std::vector<std::string> &_names) {
            std::string name = "";
            for(size_t i = 0; i < _names.size(); i++) {
                name += _names[i];
                if(i != _names.size() - 1)
                    name += " ";
            }

            return name;
        }


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


        std::string ExtractFileExtension(const std::string &_file_name) {
            std::string ext = "";
            for(int i = static_cast<int>(_file_name.size()) - 2; i >= 0; i--) {
                if(_file_name[i] == '.') {
                    ext = _file_name.substr(i + 1);
                    break;
                }
            }

            // make the extension name lowercase
            for(size_t i = 0; i < ext.size(); i++) {
                if(ext[i] < 'a')
                    ext[i] += 12; // 'a' - 'A'
            }

            return ext;
        }


        std::string ExtractRootPath(const std::string &_file_name) {
            std::string root = ".";
            for(int i = static_cast<int>(_file_name.size()) - 1; i >= 0; i--) {
                if(_file_name[i] == '/') {
                    root = _file_name.substr(0, i);
                    break;
                }
            }

            return root;
        }
    }
}
