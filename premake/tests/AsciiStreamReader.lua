--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: StreamReader.lua - StreamReader class test build configuration
--- author: Karl-Mihkel Ott

local AsciiStreamReader = {}

function AsciiStreamReader.build() 
    project "AsciiStreamReaderTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++14"

        files { 
            "include/*",
            "src/AsciiStreamReader.cpp",
            "tests/WavefrontObjStreamReader.cpp" 
        }
end


return AsciiStreamReader
