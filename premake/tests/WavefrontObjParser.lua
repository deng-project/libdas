--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: WavefrontObjParser.lua - WavefrontObjParser class test build configuration
--- author: Karl-Mihkel Ott

local WavefrontObjParser = {}

function WavefrontObjParser.build()
    project "WavefrontObjParserTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++14"

        files {
            "include/*",
            "src/WavefrontObjParser.cpp",
            "src/WavefrontObjStructures.cpp",
            "src/AsciiStreamReader.cpp",
            "src/AsciiLineReader.cpp",
            "src/ParserErrorHandler.cpp",
            "tests/WavefrontObjParseTest.cpp"
        }
end


return WavefrontObjParser
