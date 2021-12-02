--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: WavefrontObjParser.lua - WavefrontObjParser class test build configuration
--- author: Karl-Mihkel Ott

local WavefrontObjParser = {}

function WavefrontObjParser.build()
    project "WavefrontObjParserTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        files {
            "include/*",
            "tests/WavefrontObjParseTest.cpp"
        }

        links { "das" }
end


return WavefrontObjParser
