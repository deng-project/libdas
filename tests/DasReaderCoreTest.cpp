// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasReaderCoreTest.cpp - DasReaderCore class test application
// author: Karl-Mihkel Ott

// stl
#include <any>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>
#include <string>

#include <Api.h>
#include <Vector.h>
#include <Matrix.h>
#include <Points.h>
#include <Quaternion.h>
#include <AsciiStreamReader.h>
#include <AsciiLineReader.h>
#include <ErrorHandlers.h>
#include <DasStructures.h>
#include <DasReaderCore.h>

int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cerr << "Please provide a file name as an argument" << std::endl;
        std::exit(-1);
    }

    Libdas::DasReaderCore reader(argv[1]);
    reader.ReadSignature();
    Libdas::DasScopeType type = reader.ParseScopeDeclaration();

    if(type == Libdas::LIBDAS_DAS_SCOPE_PROPERTIES) {
        Libdas::DasProperties props = std::any_cast<Libdas::DasProperties>(reader.ReadScopeData(Libdas::LIBDAS_DAS_SCOPE_PROPERTIES));

        std::cout << "MODEL: " << props.model << std::endl;
        std::cout << "AUTHOR: " << props.author << std::endl;
        std::cout << "COPYRIGHT: " << props.copyright << std::endl;
        std::cout << "MODDATE: " << props.moddate << std::endl;
    }
    else std::cout << "Invalid or corrupt file " << argv[1] << std::endl;
    return 0;
}
