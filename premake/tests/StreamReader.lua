--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: StreamReader.lua - StreamReader class test build configuration
--- author: Karl-Mihkel Ott

local StreamReader = {}

function StreamReader.build() 
    project "StreamReaderTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++14"

        files { 
            "include/*"
            "src/StreamReader.cpp",
            "tests/WavefrontObjStreamReader.cpp" 
        }
end


return StreamReader
