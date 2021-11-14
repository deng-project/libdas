--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: BinarySTLParser.lua - BinarySTLParser class test build configuration
--- author: Karl-Mihkel Ott


local BinarySTLParser = {}

function BinarySTLParser.build() 
    project "BinarySTLParserTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++14"

        files {
            "include/*",
            "src/STLParser.cpp",
            "src/STLStructures.cpp",
            "src/AsciiStreamReader.cpp",
            "src/AsciiLineReader.cpp",
            "src/ParserErrorHandler.cpp",
            "tests/BinarySTLParseTest.cpp"
        }
end


return BinarySTLParser
