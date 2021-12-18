#include <string>
#include <any>
#include <map>
#include <fstream>
#include <memory>
#include <vector>
#include <iostream>

#include <ErrorHandlers.h>
#include <AsciiStreamReader.h>
#include <JSONParser.h>

void OutputNodes(const std::string &_name, Libdas::JSONNode *_node, std::string _sep) {
    std::cout << _sep << " " << _name << std::endl;
    _sep += "-";
    for(size_t i = 0; i < _node->values.size(); i++) {
        switch(_node->values[i].first) {
            case Libdas::JSON_TYPE_STRING:
                std::cout << _sep << " " << std::any_cast<std::string>(_node->values[i].second) << std::endl;
                break;

            case Libdas::JSON_TYPE_INTEGER:
                std::cout << _sep << " " << std::any_cast<int>(_node->values[i].second) << std::endl;
                break;

            case Libdas::JSON_TYPE_FLOAT:
                std::cout << _sep << " " << std::any_cast<float>(_node->values[i].second) << std::endl;
                break;

            case Libdas::JSON_TYPE_BOOLEAN:
                std::cout << _sep << " " << (std::any_cast<bool>(_node->values[i].second) ? "true" : "false") << std::endl;
                break;

            case Libdas::JSON_TYPE_OBJECT:
                OutputNodes("{}", &std::any_cast<Libdas::JSONNode&>(_node->values[i].second), _sep);
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


    Libdas::JSONParser parser(argv[1]);
    parser.Parse();

    Libdas::JSONNode &root = parser.GetRootNode();
    OutputNodes("root", &root, "-");

    return 0;
}
