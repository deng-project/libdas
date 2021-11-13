--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: AsciiSTLParser.lua - AsciiSTLParser class test build configuration
--- author: Karl-Mihkel Ott

local AsciiSTLParser = {}

function AsciiSTLParser.build()
    project "AsciiSTLParserTest"
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
            "tests/AsciiSTLParseTest.cpp"
        }
end


return AsciiSTLParser
