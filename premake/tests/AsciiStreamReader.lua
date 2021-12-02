--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: StreamReader.lua - StreamReader class test build configuration
--- author: Karl-Mihkel Ott

local AsciiStreamReader = {}

function AsciiStreamReader.build() 
    project "AsciiStreamReaderTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        files { 
            "include/*",
            "tests/WavefrontObjStreamReader.cpp" 
        }

        links { "das" }
end


return AsciiStreamReader
