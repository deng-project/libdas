--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: DasParser.lua - DasParser class test build configuration
--- author: Karl-Mihkel Ott

local DasParser = {}

function DasParser.build()
    project "DasParser"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        files {
            "include/*",
            "tests/DasParserTest.cpp"
        }

        links { "das" }
end

return DasParser
