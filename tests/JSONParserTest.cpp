#include <string>
#include <any>
#include <variant>
#include <map>
#include <fstream>
#include <memory>
#include <vector>
#include <iostream>

#include <Api.h>
#include <ErrorHandlers.h>
#include <AsciiStreamReader.h>
#include <JSONParser.h>

void OutputNodes(const std::string &_name, Libdas::JSONNode *_node, std::string _sep) {
    std::cout << _sep << " " << _name << std::endl;
    _sep += "-";
    for(size_t i = 0; i < _node->values.size(); i++) {
        switch(_node->values[i].index()) {
            case JSON_TYPE_STRING:
                std::cout << _sep << " " << std::get<std::string>(_node->values[i]) << std::endl;
                break;

            case JSON_TYPE_NUMBER:
                std::cout << _sep << " " << std::get<float>(_node->values[i]) << std::endl;
                break;

            case JSON_TYPE_BOOLEAN:
                std::cout << _sep << " " << (std::get<bool>(_node->values[i]) ? "true" : "false") << std::endl;
                break;

            case JSON_TYPE_OBJECT:
                OutputNodes("{}", &std::get<Libdas::JSONNode>(_node->values[i]), _sep);
                break;

            default:
                break;
        }
    }

    // iterate through subnodes
    for(auto it = _node->sub_nodes.begin(); it != _node->sub_nodes.end(); it++)
        OutputNodes(it->first, it->second.get(), _sep);
}


int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cerr << "Please provide a JSON file name to use" << std::endl;
        std::exit(LIBDAS_ERROR_INVALID_FILE);
    }


    Libdas::JSONParser parser(Libdas::MODEL_FORMAT_JSON, argv[1]);
    parser.Parse();

    Libdas::JSONNode &root = parser.GetRootNode();
    OutputNodes("root", &root, "-");

    return 0;
}
