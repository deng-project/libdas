--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: BinarySTLParser.lua - BinarySTLParser class test build configuration
--- author: Karl-Mihkel Ott


local BinarySTLParser = {}

function BinarySTLParser.build() 
    project "BinarySTLParserTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        files {
            "include/*",
            "tests/BinarySTLParseTest.cpp"
        }

        links { "das" }
end


return BinarySTLParser
