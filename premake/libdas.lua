--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: libdas.lua - libdas library build configuration
--- author: Karl-Mihkel Ott

local Libdas = {}

function Libdas.build() 
    project "das"
        kind "SharedLib"
        language "C++"
        cppdialect "C++14"

        files {
            "include/*",
            "src/STLParser.cpp",
            "src/STLStructures.cpp",
            "src/WavefrontObjStructures.cpp",
            "src/WavefrontObjParser.cpp",
            "src/AsciiStreamReader.cpp",
            "src/AsciiLineReader.cpp",
            "src/ParserErrorHandler.cpp",
        }
end

return Libdas
