#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <iostream>

#include <Vector.h>
#include <Matrix.h>

#include <LibdasAssert.h>
#include <ParserErrorHandler.h>
#include <DasStructures.h>
#include <DasWriterCore.h>


void ReadFileToBuffer(const std::string &_file_name, Libdas::DasBuffer &_buffer) {
    std::ifstream out(_file_name, std::ios_base::binary);

    if(out.fail()) {
        std::cout << "Could not open file \"" << _file_name << "\"" << std::endl;
        std::exit(LIBDAS_ERROR_INVALID_FILE);
    }

    out.seekg(0, std::ios_base::end);
    _buffer.data_len = static_cast<uint32_t>(out.tellg());
    out.seekg(0, std::ios_base::beg);

    _buffer.data = reinterpret_cast<char*>(std::malloc(_buffer.data_len));
    out.read(_buffer.data, _buffer.data_len);

    out.close();
}


int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cout << "Please provide a file name as the buffer" << std::endl;
        std::exit(LIBDAS_ERROR_INVALID_FILE);
    }
    Libdas::DasWriterCore writer;
    writer.NewFile("test.das");

    // write properties
    Libdas::DasProperties props;
    props.author = "inugami-dev64";
    props.compression = false;
    props.model = "";
    writer.InitialiseFile(props);

    // write buffer section
    Libdas::DasBuffer buffer;
    buffer.type = LIBDAS_BUFFER_TYPE_VERTEX;
    ReadFileToBuffer(argv[1], buffer);
    writer.WriteBuffer(buffer);

    return 0;
}
