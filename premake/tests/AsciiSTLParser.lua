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
            "tests/AsciiSTLParseTest.cpp"
        }

        links { "das" }
end


return AsciiSTLParser
